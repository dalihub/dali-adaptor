
QUnit.module("Shaders", {
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

QUnit.test( "shader meta data", function( assert ) {
  var info = new dali.ShaderInfo();

  var vertex;
  var fragment;

  vertex = "\n" +
    "attribute mediump vec3 aPosition;\n" +
    "attribute mediump vec2 aTexCoord;\n" +
    "varying mediump vec2 vTexCoord;\n" +
    "uniform mediump vec3 uSize;\n" +
    "// uniform mediump vec3 unusedUniform;\n" +
    "uniform mediump mat4 uModelView;\n" +
    "uniform mediump mat4 uProjection;\n" +
    "\n" +
    "void main(void)\n" +
    "{\n" +
    "  gl_Position = uProjection * uModelView * vec4(aPosition, 1.0);\n" +
    "  gl_Position.xyz *= uSize;\n" +
    "  vTexCoord = aTexCoord;\n" +
    "}\n";

  fragment = "precision mediump float;\n" +
    "\n" +
    "uniform sampler2D sTexture;\n" +
    "uniform mediump vec4 uMyColor; // {min:[0,0,0,0], max:[1,1,1,1]}\n" +
    "uniform mediump vec4 uColor;\n" +
    "varying mediump vec2 vTexCoord;\n" +
    "\n" +
    "void main()\n" +
    "{\n" +
    "  gl_FragColor = texture2D( sTexture, vTexCoord ) * uColor * uMyColor;\n" +
    "}\n";

  var canvas = document.createElement("canvas");
  var meta = info.fromCompilation(canvas.getContext("webgl"), vertex, fragment);

  var uniforms = { uSize: 1,
                   uModelView: 1,
                   uProjection: 1,
                   uMyColor: 1,
                   uColor: 1
                 };

  assert.ok(meta.hasError === false);
  var name;
  var metaUniformName;
  var found;

  for(name in uniforms) {
    found = false;
    for(metaUniformName in meta.uniforms) {
      if(metaUniformName === name) {
        found = true;
        break;
      }
    }
    assert.ok(found, "missing:" + name);
  }

  assert.ok(compareArrays(meta.uniformUISpec.uMyColor.min, [0, 0, 0, 0]));
  assert.ok(compareArrays(meta.uniformUISpec.uMyColor.max, [1, 1, 1, 1]));


  meta = info.fromRegEx(vertex, fragment);

  assert.ok(meta.hasError === false);

  for(name in uniforms) {
    found = false;
    for(metaUniformName in meta.uniforms) {
      if(metaUniformName === name) {
        found = true;
        break;
      }
    }
    assert.ok(found, "missing:" + name);
  }

  assert.ok(compareArrays(meta.uniformUISpec.uMyColor.min, [0, 0, 0, 0]));
  assert.ok(compareArrays(meta.uniformUISpec.uMyColor.max, [1, 1, 1, 1]));

});
