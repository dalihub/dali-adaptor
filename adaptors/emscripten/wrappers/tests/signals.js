
QUnit.module("Signals", {
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

QUnit.test( "hello test", function( assert ) {
  var done = false;

  function onStage() {
    done = true;
  }

  var actor = new dali.Actor();
  actor.parentOrigin = [0.5, 0.5, 0.5];
  actor.anchorPoint = [0.5, 0.5, 0.5];
  actor.text = "actor";
  actor.name = actor.text;
  actor.size = [100, 100, 1];
  actor.position = [0, 0, 10];

  actor.connect("onStage", onStage);

  dali.stage.add(actor);

  dali.updateFrame();

  assert.ok(done === true);

});
