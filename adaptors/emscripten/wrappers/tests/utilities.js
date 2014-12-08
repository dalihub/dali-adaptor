var EPSILON = 0.005;

function compareArrays(a, b) {
  "use strict";
  if (Array.isArray(a) && Array.isArray(b)) {
    if (a.length === b.length) {
      for (var i = 0, len = a.length; i < len; i++) {
        if (Array.isArray(a[i])) {
          if (Array.isArray(b[i])) {
            if (!compareArrays(a[i], b[i])) {
              return false;
            }
          } else {
            return false;
          }
        } else {
          if (typeof (a[i]) === "number") {
            if (typeof (b[i]) !== "number") {
              return false;
            } else {
              if (Math.abs(a[i]) > Math.abs(b[i]) + EPSILON ||
                  Math.abs(a[i]) < Math.abs(b[i]) - EPSILON) {
                return false;
              }
            }
          } else {
            if (a[i] !== b[i]) {
              return false;
            }
          }
        }
      }
      return true;
    }
  }
  return false;
}

function collectByName(collection) {
  var root = dali.stage.getRootLayer();
  if (collection === undefined) {
    collection = {};
  }
  var op = function(actor) {
    if (actor.name) {
      collection[actor.name] = actor;
    }
    return true;
  };

  dali.debug.depthVisit(root, op, true);

  return collection;
}

function square(color, size) {
  var a = dali.createSolidColorActor(color, 0, [0, 0, 0, 1], 0);
  a.size = size;
  return a;
}

function threeSquares() {
  var root = dali.stage.getRootLayer();

  var a = square([1, 0, 0, 1], [200, 200, 0]);
  a.name = "red";
  a.position = [-100, 0, -20];
  root.add(a);
  a.delete();

  a = square([0, 1, 0, 1], [200, 200, 0]);
  a.name = "green";
  a.position = [0, -100, -10];
  root.add(a);
  a.delete();

  a = square([0, 0, 1, 1], [200, 200, 0]);
  a.name = "blue";
  a.position = [0, -100, 0];
  root.add(a);
  a.delete();

  //  root.delete();
}

function clear() {
  var root = dali.stage.getRootLayer();
  var children = root.getChildren();

  for (var i = 0, len = children.length; i < len; i++) {
    root.remove(children[i]);
    children[i].delete(); // delete the wrapper
  }
  //  root.delete(); // wrapper
}

var loadDocument = function(url, f) {
    var v = document.getElementById("daliframe");
    QUnit.ok(v, "Found frame.");
    v.addEventListener("load", f, true);
    v.src = url;
};
