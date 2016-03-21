
QUnit.module("Properties", {
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

QUnit.test( "dotted property access", function( assert ) {

  var actor = new dali.Actor();

  assert.ok( "" === actor.name );
  assert.ok( compareArrays(actor.position, [0, 0, 0]) );
  assert.ok( compareArrays(actor.parentOrigin, [0, 0, 0.5]) );

});

QUnit.test( "hierarchy", function( assert ) {

  var actor = new dali.Actor();
  actor.parentOrigin = [0.5, 0.5, 0.5];
  actor.anchorPoint = [0.5, 0.5, 0.5];
  actor.text = "actor";
  actor.name = actor.text;
  actor.size = [100, 100, 1];
  actor.position = [0, 0, 10];
  dali.stage.add(actor);

  var hello = new dali.Actor();
  hello.text = "hello";
  hello.name = hello.text;
  actor.add(hello);

  var hellochild = new dali.Actor();
  hellochild.text = "hello-child";
  hellochild.name = hellochild.text;
  hello.add(hellochild);

  var hellochild2 = new dali.Actor();
  hellochild2.text = "hello-child2";
  hellochild2.name = hellochild2.text;
  hello.add(hellochild2);

  var hellochildchild = new dali.Actor();
  hellochildchild.text = "hello-child-child1";
  hellochildchild.name = "hello-child-child1";
  hellochildchild.name = hellochildchild.text;
  hellochild.add(hellochildchild);


  var depthfirst = actor.findAllChildren();

  assert.ok(actor.getChildCount() === 1);

  var directChildren = actor.directChildren();

  assert.ok(directChildren.length === 1);
  assert.ok(directChildren[0].getId() === hello.getId());

  actor.position = [100, 100, 0];

  var root = dali.stage.getRootLayer(); //rootRotationActor;

  actor.remove(hello);
  assert.ok(actor.getChildCount() === 0);

  actor.add(hello);
  assert.ok(actor.getChildCount() === 1);

  var rootLayerCount = root.getChildCount();
  dali.stage.remove(actor); // check these don't assert
  assert.ok(root.getChildCount() === rootLayerCount - 1);

  dali.stage.add(actor);
  assert.ok(root.getChildCount() === rootLayerCount);

  assert.ok(root.findChildByName("none") === null);

});

QUnit.test( "register property", function( assert ) {
  var s = dali.stage;
  var root = s.getRootLayer(); //rootRotationActor;

  var another = new dali.Actor();
  another.parentOrigin = [0.5, 0.5, 0.5];
  another.anchorPoint = [0.5, 0.5, 0.5];
  another.text = "peppa";
  another.name = another.text;
  another.size = [100, 100, 1];
  another.position = [-50, 100, 0];
  root.add(another);

  var c = root.getChildAt(root.getChildCount() - 1);
  //var n = c.getChildCount();
  var p = c.getParent();
  assert.ok(p.getId() == root.getId());

  var matrix = c.worldMatrix;

  assert.ok(matrix.length === 16);

});

QUnit.test( "get/set", function( assert ) {
  var s = dali.stage;
  threeSquares();
  var col = {};
  collectByName(col);
  var actor = col.red;
  var p = actor.position;
  actor.position = [1, 1, 1];
  assert.ok(compareArrays(actor.position, [1, 1, 1]));
  actor.position = [3, 3, 3];
  assert.ok(compareArrays(actor.position, [3, 3, 3]));
  actor.position = p;
});
