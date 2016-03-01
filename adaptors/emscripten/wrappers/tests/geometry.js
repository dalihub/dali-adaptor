
QUnit.module("Geometry", {
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


QUnit.test( "colour quad", function( assert ) {
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

});


QUnit.test( "textured quad", function( assert ) {

  var halfQuadSize = 0.5;

  // using helper function to create property buffer
  var verts = dali.createPropertyBuffer( {format: [ ["aPosition", dali.PropertyType.VECTOR3],
                                                    ["aTexCoord", dali.PropertyType.VECTOR2] ],
                                          data: { "aPosition": [ [-halfQuadSize, -halfQuadSize, 0.0],
                                                                 [+halfQuadSize, -halfQuadSize, 0.0],
                                                                 [-halfQuadSize, +halfQuadSize, 0.0],
                                                                 [+halfQuadSize, +halfQuadSize, 0.0]
                                                               ],
                                                  "aTexCoord": [ [0, 0],
                                                                 [1, 0],
                                                                 [0, 1],
                                                                 [1, 1]
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
        "// atributes\n" +
        "attribute mediump vec3 aPosition;" +
        "attribute mediump vec2 aTexCoord;\n" +
        "// inbuilt\n" +
        "uniform mediump mat4 uMvpMatrix;" +
        "uniform mediump vec3 uSize;" +
        "uniform lowp vec4 uColor;" +
        "// varying\n" +
        "varying mediump vec2 vTexCoord;\n" +
        "" +
        "void main()" +
        "{" +
        "  mediump vec4 vertexPosition = vec4(aPosition, 1.0);" +
        "  vertexPosition.xyz *= uSize;" +
        "  gl_Position = uMvpMatrix * vertexPosition;" +
        "  vTexCoord = aTexCoord;\n" +
        "}";

  var fragment = "" +
        "uniform lowp vec4 uColor;" +
        "uniform sampler2D sTexture;\n" +
        "varying mediump vec2 vTexCoord;\n" +
        "\n" +
        "void main()" +
        "{" +
        "  gl_FragColor = texture2D(sTexture, vTexCoord) * uColor;\n" +
        "}";


  var shader = new dali.Shader(vertex, fragment, dali.ShaderHints.HINT_NONE);
  assert.ok(shader);

  var material = new dali.Material(shader);
  assert.ok(material);

  var image = dali.unitTestEmbeddedImage() ;
  assert.ok(image);
  var sampler = new dali.Sampler();
  material.addTexture(image, "sTexture", sampler);

  var renderer = new dali.Renderer(geometry, material);
  assert.ok(renderer);

  var actor = new dali.Actor();

  actor.addRenderer(renderer);

  dali.stage.add(actor);

  actor.parentOrigin = [0.5, 0.5, 0.0];
  actor.size = [100,100,1];

});
