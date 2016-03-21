function assertArray(assert, a, b, epsilon) {
  assert.ok(a.length === b.length);
  for (var i = 0, len = a.length; i < len; ++i) {
    assert.ok(a[i] > b[i] - epsilon && a[i] < b[i] + epsilon);
  }
}

QUnit.module("Math", {
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

QUnit.test( "vector", function( assert ) {
  assert.ok(dali.vectorLength([1, 2, 3, 4]) === Math.sqrt(1 * 1 + 2 * 2 + 3 * 3));
  assert.ok(dali.vectorLengthSquared(dali.normalize([0, 0, 0, 1])) === 0);

  var f = 2;
  assert.ok(1 === dali.vectorLengthSquared(dali.normalize([Math.cos(f) * 10.0,
                                                        Math.cos(f + 1.0) * 10.0,
                                                        Math.cos(f + 2.0) * 10.0,
                                                        1.0
                                                       ])));
  assertArray(assert, dali.vectorCross([0, 1, 0], [0, 0, 1]), [1, 0, 0], 0.001);

  assertArray(assert, dali.vectorAdd([1, 2, 3], [2, 3, 4], [1, 1, 1]), [4, 6, 8], 0.001);

});


QUnit.test( "quaternion", function( assert ) {

  assertArray(assert, dali.axisAngleToQuaternion([1.0, 2.0, 3.0, Math.PI / 3.0, Math.PI / 2.0]), [0.189, 0.378, 0.567, 0.707], 0.001);

  assertArray(assert, dali.quaternionToAxisAngle([1.1, 3.4, 2.7, 0.932]), [3.03, 9.38, 7.45, 0.74],
              0.01);

  var mq = dali.vectorAdd(dali.vectorCross([0.045, 0.443, 0.432], [0.612, 0.344, -0.144]),
                          dali.vectorByScalar([0.612, 0.344, -0.144], 0.784),
                          dali.vectorByScalar([0.045, 0.443, 0.432], 0.697));

  assertArray(assert, dali.quatByQuat([0.045, 0.443, 0.432, 0.784], [0.612, 0.344, -0.144, 0.697]), [mq[0], mq[1], mq[2], (0.784 * 0.697) - dali.vectorDot([0.045, 0.443, 0.432], [0.612, 0.344, -0.144])],
              0.001);

});
