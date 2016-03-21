
QUnit.module("Views", {
    setup : function() {
        QUnit.stop();
        var that = this;
        this.cb = function(e) {
          QUnit.ok(true, "Scene loaded");
          var iframe = document.getElementById('daliframe');
          that.doc =  iframe.contentDocument || iframe.contentWindow.document;
          that.doc.Module.postDaliWrapperRun = function() {
            dali = that.doc.Module;
            QUnit.start();
          };
        };
        loadDocument("dali-page.html"+window.location.search, this.cb);
    },
    teardown : function() {
        var v = document.getElementById("daliframe");
        v.removeEventListener("load", this.cb, true);
    }
});

function addOnScreenQuad(assert)
{
  var halfQuadSize = 0.5;

  // using helper function to create property buffer
  var verts = dali.createPropertyBuffer( {format: [ ["aPosition", dali.PropertyType.VECTOR3],
                                                    ["aCol", dali.PropertyType.VECTOR4] ],
                                          data: { "aPosition": [ [-halfQuadSize, -halfQuadSize, 0.0],
                                                                 [+halfQuadSize, -halfQuadSize, 0.0],
                                                                 [-halfQuadSize, +halfQuadSize, 0.0],
                                                                 [+halfQuadSize, +halfQuadSize, 0.0]
                                                               ],
                                                  "aCol": [ [0, 0, 0, 1],
                                                            [1, 0, 1, 1],
                                                            [0, 1, 0, 1],
                                                            [1, 1, 1, 1]
                                                          ]
                                                }
                                         });

  var indices = dali.createPropertyBuffer( { format: [ ["indices", dali.PropertyType.INTEGER]],
                                             data: { "indices": [0, 3, 1, 0, 2, 3] } } ) ;

  var geometry = new dali.Geometry();

  assert.ok(verts);
  assert.ok(indices);
  assert.ok(geometry);

  geometry.addVertexBuffer(verts);
  geometry.setIndexBuffer(indices);

  var vertex = "" +
        "attribute mediump vec3 aPosition;" +
        "attribute mediump vec4 aCol;" +
        "uniform mediump mat4 uMvpMatrix;" +
        "uniform mediump vec3 uSize;" +
        "uniform lowp vec4 uColor;" +
        "varying lowp vec4 vColor;" +
        "" +
        "void main()" +
        "{" +
        "  vColor = aCol * uColor;" +
        "  mediump vec4 vertexPosition = vec4(aPosition,1.0);" +
        "  vertexPosition.xyz *= uSize;" +
        "  gl_Position = uMvpMatrix * vertexPosition;" +
        "}";

  var fragment = "" +
        "varying lowp vec4 vColor;" +
        "uniform lowp vec4 uColor;" +
        "" +
        "void main()" +
        "{" +
        "  gl_FragColor = vColor * uColor;" +
        "}";

  var shader = new dali.Shader(vertex, fragment, dali.ShaderHints.HINT_NONE);
  assert.ok(shader);

  var material = new dali.Material(shader);
  assert.ok(material);

  var renderer = new dali.Renderer(geometry, material);
  assert.ok(renderer);

  var actor = new dali.Actor();
  assert.ok(actor);

  actor.addRenderer(renderer);

  dali.stage.add(actor);

  actor.parentOrigin = [0.5, 0.5, 0.0];
  actor.size = [100,100,1];
}


QUnit.test( "Exercise view helpers", function( assert ) {

  // just setting the back colour doesnt trigger a redraw in Dali so we
  // add a quad just to see the test running
  addOnScreenQuad(assert);

  var done1 = assert.async();
  var done2 = assert.async();
  var done3 = assert.async();

  var w = dali.canvas.width;
  var h = dali.canvas.height;
  var col = dali.getClearColor(0);
  console.log(col);

  function one() {
    dali.onePane();
    dali.setFrontView(0, 0, 0, w, h);
    dali.setClearColor(0, col);
    done3();
  }

  function three() {
    dali.threePane();
    dali.setClearColor(0, [0.4, 0, 0, 1]);
    dali.setClearColor(1, [0, 0.4, 0, 1]);
    dali.setClearColor(2, [0, 0, 0.4, 1]);
    dali.setFrontView(0, 0, 0, w / 2 - 5, h);
    dali.setTopView(1, w / 2, 0, w / 2, h / 2 - 5);
    dali.setRightView(2, w / 2, h / 2 + 5, w / 2, h / 2 - 5);

    window.setTimeout(one, 100);
    done2();
  }

  function two() {
    dali.twoPane();
    dali.setFrontView(0, 0, 0, w / 2 - 10, h);
    dali.setTopView(1, 210, 0, w / 2 - 10, h);
    dali.setClearColor(0, [0.4, 0, 0, 1]);
    dali.setClearColor(1, [0, 0.4, 0, 1]);

    window.setTimeout(three, 100);
    done1();
  }

  //one();

  window.setTimeout(two, 100);

});
