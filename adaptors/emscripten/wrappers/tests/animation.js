
QUnit.module("Animation", {
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

QUnit.test( "spline path", function( assert ) {

  var done1 = assert.async();

  threeSquares();
  var col = {};
  collectByName(col);
  var actor = col.red;

  var a = new dali.Animation(0);
  var path = new dali.Path();

  path.points = [
    [-150, -50, 0],
    [0.0, 70.0, 0.0],
    [190.0, -150.0, 0.0]
  ];

  assert.ok(compareArrays(path.points, [
    [-150, -50, 0],
    [0.0, 70.0, 0.0],
    [190.0, -150.0, 0.0]
  ]));

  dali.generateControlPoints(path, 0.35);

  assert.ok(compareArrays(path.controlPoints, [
    [-97.5, -8, 0],
    [-66.94940948486328, 76.16658020019531, 0],
    [101.31224060058594, 60.66832733154297, 0],
    [123.5, -73, 0]
  ]));

  a.setDuration(0.1);
  a.animatePath(actor, path, [1, 0, 0], dali.AlphaFunction.LINEAR, 0, 0.1);
  a.play();


  function checkPos() {
    assert.ok(actor.position = path.points[2]);
    clear();
    actor.delete();
    path.delete();
    a.delete();
    done1();
  }

  window.setTimeout(checkPos, 200);

});

QUnit.test( "linear", function( assert ) {

  var done1 = assert.async();
  var done2 = assert.async();
  var done3 = assert.async();

  threeSquares();
  var col = {};
  collectByName(col);
  var actor = col.red;

  var a = new dali.Animation(0);
  a.setDuration(0.1);
  a.animateTo(actor, "position", [20, 0, 0], dali.AlphaFunction.LINEAR, 0, 0.1);
  a.play();

  function checkAnimateBetween() {
    assert.ok(actor.position = [0, 0, -30]);
    clear();
    a.delete();
    actor.delete();
    done3();
  }

  function checkAnimateBy() {
    assert.ok(actor.position = [120, 100, 0]);
    a.clear();
    a.animateBetween(actor,
                     "position", [ [ 0,  [10,20,30] ],
                                   [ 1.0,[0, 0, -30] ] ],
                     "linear",
                     0,
                     0.1,
                     "linear");
    a.play();
    window.setTimeout(checkAnimateBetween, 200);
    done2();
  }

  function checkAnimateTo() {
    assert.ok(actor.position = [20, 0, 0]);
    actor.position = [100, 100, 0];

    a.clear(); // var a = new dali.Animation(0);
    a.setDuration(0.1);
    a.animateBy(actor, "position", [20, 0, 0], dali.AlphaFunction.LINEAR, 0, 0.1);
    a.play();
    window.setTimeout(checkAnimateBy, 200);
    done1();
  }

  window.setTimeout(checkAnimateTo, 200);

});

