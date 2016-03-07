/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* eslint-env browser */
/* eslint "brace-style": [2, "1tbs"] */
/* eslint "no-console": 0 */
/* eslint "no-underscore-dangle": 0 */

/*******************************************************************************
 *
 * The javascript counterpart to the C++ DALi wrapper.
 *
 * Provides helper functionality to make the use of the dali module.
 *
 * Where possible it creates a natural Javascript API. One problem
 * is that wrapped C++ objects must be explicitly deleted '<obj>.delete()'.
 * This is because javascript in the browser has no available hook to watch
 * for an objects destruction.
 *
 * This file combines several 'Modules' and could be split when using a
 * a recombining web build tool.
 *
 *
 ******************************************************************************/

// forward refs for lint
var Module;
// On upgrading to emscripten 1.34.2 and -s EXPORT_NAME="dali" on emcc command
// line no longer seems to work so set it here.
var dali = Module;
var document;
var console;

//------------------------------------------------------------------------------
//
// Math Module
//
//------------------------------------------------------------------------------

/**
 * Matrix Multiplication
 * @method matrixByMatrix
 * @param {Array} A Matrix4 array
 * @param {Array} B Matrix4 array
 * @return {Array} Matrix4
 */
dali.matrixByMatrix = function(A, B) {
  "use strict";

  var ret = [1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1
            ];

  ret[0] = A[0] * B[0] + A[1] * B[4] + A[2] * B[8] + A[3] * B[12];
  ret[4] = A[4] * B[0] + A[5] * B[4] + A[6] * B[8] + A[7] * B[12];
  ret[8] = A[8] * B[0] + A[9] * B[4] + A[10] * B[8] + A[11] * B[12];
  ret[12] = A[12] * B[0] + A[13] * B[4] + A[14] * B[8] + A[15] * B[12];

  ret[1] = A[0] * B[1] + A[1] * B[5] + A[2] * B[9] + A[3] * B[13];
  ret[5] = A[4] * B[1] + A[5] * B[5] + A[6] * B[9] + A[7] * B[13];
  ret[9] = A[8] * B[1] + A[9] * B[5] + A[10] * B[9] + A[11] * B[13];
  ret[13] = A[12] * B[1] + A[13] * B[5] + A[14] * B[9] + A[15] * B[13];

  ret[2] = A[0] * B[2] + A[1] * B[6] + A[2] * B[10] + A[3] * B[14];
  ret[6] = A[4] * B[2] + A[5] * B[6] + A[6] * B[10] + A[7] * B[14];
  ret[10] = A[8] * B[2] + A[9] * B[6] + A[10] * B[10] + A[11] * B[14];
  ret[14] = A[12] * B[2] + A[13] * B[6] + A[14] * B[10] + A[15] * B[14];

  ret[3] = A[0] * B[3] + A[1] * B[7] + A[2] * B[11] + A[3] * B[15];
  ret[7] = A[4] * B[3] + A[5] * B[7] + A[6] * B[11] + A[7] * B[15];
  ret[11] = A[8] * B[3] + A[9] * B[7] + A[10] * B[11] + A[11] * B[15];
  ret[15] = A[12] * B[3] + A[13] * B[7] + A[14] * B[11] + A[15] * B[15];

  return ret;
};

/**
 * Matrix Vector4 Multiplication
 * @method matrixByVector
 * @param {Array} A Matrix4 array
 * @param {Array} v Vector4
 * @return {Array} Vector4
 */
dali.matrixByVector = function(A, v) {
  "use strict";

  var x = v[0];
  var y = v[1];
  var z = v[2];
  var w = 1;

  if (v.length === 4) {
    w = v[3];
  }

  return [
    A[0] * x + A[4] * y + A[8] * z + A[12] * w,
    A[1] * x + A[5] * y + A[9] * z + A[13] * w,
    A[2] * x + A[6] * y + A[10] * z + A[14] * w,
    A[3] * x + A[7] * y + A[11] * z + A[15] * w
  ];
};

/**
 * Get Matrix Determinant
 * @method matrixDeterminant
 * @param {Array} A Matrix4 array
 * @return {float} Determinant
 */
dali.matrixDeterminant = function(A) {
  "use strict";

  var n11 = A[0],
      n12 = A[4],
      n13 = A[8],
      n14 = A[12];
  var n21 = A[1],
      n22 = A[5],
      n23 = A[9],
      n24 = A[13];
  var n31 = A[2],
      n32 = A[6],
      n33 = A[10],
      n34 = A[14];
  var n41 = A[3],
      n42 = A[7],
      n43 = A[11],
      n44 = A[15];

  var m0 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
  var m4 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
  var m8 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
  var m12 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

  return n11 * m0 + n21 * m4 + n31 * m8 + n41 * m12;
};

/**
 * Matrix Multiplication by scalar
 * @method matrixByScalar
 * @param {Array} A Matrix4 array
 * @param {float} s float
 * @return {Array} Matrix4
 */
dali.matrixByScalar = function(A, s) {
  "use strict";
  return [A[0] * s, A[1] * s, A[2] * s, A[3] * s,
          A[4] * s, A[5] * s, A[6] * s, A[7] * s,
          A[8] * s, A[9] * s, A[10] * s, A[11] * s,
          A[12] * s, A[13] * s, A[14] * s, A[15] * s
         ];
};

/**
 * Matrix Inverse. Raises if there is none.
 * @method matrixInverse
 * @param {Array} A Matrix4 array
 * @return {Array} Inverse Matrix4
 */
dali.matrixInverse = function(A) {
  "use strict";
  var ret = [1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1
            ];

  ret[0] = A[5] * A[10] * A[15] - A[5] * A[11] * A[14] - A[9] * A[6] * A[15] + A[9] * A[7] * A[14] + A[13] * A[6] * A[11] - A[13] * A[7] * A[10];
  ret[4] = -A[4] * A[10] * A[15] + A[4] * A[11] * A[14] + A[8] * A[6] * A[15] - A[8] * A[7] * A[14] - A[12] * A[6] * A[11] + A[12] * A[7] * A[10];
  ret[8] = A[4] * A[9] * A[15] - A[4] * A[11] * A[13] - A[8] * A[5] * A[15] + A[8] * A[7] * A[13] + A[12] * A[5] * A[11] - A[12] * A[7] * A[9];
  ret[12] = -A[4] * A[9] * A[14] + A[4] * A[10] * A[13] + A[8] * A[5] * A[14] - A[8] * A[6] * A[13] - A[12] * A[5] * A[10] + A[12] * A[6] * A[9];

  ret[1] = -A[1] * A[10] * A[15] + A[1] * A[11] * A[14] + A[9] * A[2] * A[15] - A[9] * A[3] * A[14] - A[13] * A[2] * A[11] + A[13] * A[3] * A[10];
  ret[5] = A[0] * A[10] * A[15] - A[0] * A[11] * A[14] - A[8] * A[2] * A[15] + A[8] * A[3] * A[14] + A[12] * A[2] * A[11] - A[12] * A[3] * A[10];
  ret[9] = -A[0] * A[9] * A[15] + A[0] * A[11] * A[13] + A[8] * A[1] * A[15] - A[8] * A[3] * A[13] - A[12] * A[1] * A[11] + A[12] * A[3] * A[9];
  ret[13] = A[0] * A[9] * A[14] - A[0] * A[10] * A[13] - A[8] * A[1] * A[14] + A[8] * A[2] * A[13] + A[12] * A[1] * A[10] - A[12] * A[2] * A[9];

  ret[2] = A[1] * A[6] * A[15] - A[1] * A[7] * A[14] - A[5] * A[2] * A[15] + A[5] * A[3] * A[14] + A[13] * A[2] * A[7] - A[13] * A[3] * A[6];
  ret[6] = -A[0] * A[6] * A[15] + A[0] * A[7] * A[14] + A[4] * A[2] * A[15] - A[4] * A[3] * A[14] - A[12] * A[2] * A[7] + A[12] * A[3] * A[6];
  ret[10] = A[0] * A[5] * A[15] - A[0] * A[7] * A[13] - A[4] * A[1] * A[15] + A[4] * A[3] * A[13] + A[12] * A[1] * A[7] - A[12] * A[3] * A[5];
  ret[14] = -A[0] * A[5] * A[14] + A[0] * A[6] * A[13] + A[4] * A[1] * A[14] - A[4] * A[2] * A[13] - A[12] * A[1] * A[6] + A[12] * A[2] * A[5];

  ret[3] = -A[1] * A[6] * A[11] + A[1] * A[7] * A[10] + A[5] * A[2] * A[11] - A[5] * A[3] * A[10] - A[9] * A[2] * A[7] + A[9] * A[3] * A[6];
  ret[7] = A[0] * A[6] * A[11] - A[0] * A[7] * A[10] - A[4] * A[2] * A[11] + A[4] * A[3] * A[10] + A[8] * A[2] * A[7] - A[8] * A[3] * A[6];
  ret[11] = -A[0] * A[5] * A[11] + A[0] * A[7] * A[9] + A[4] * A[1] * A[11] - A[4] * A[3] * A[9] - A[8] * A[1] * A[7] + A[8] * A[3] * A[5];
  ret[15] = A[0] * A[5] * A[10] - A[0] * A[6] * A[9] - A[4] * A[1] * A[10] + A[4] * A[2] * A[9] + A[8] * A[1] * A[6] - A[8] * A[2] * A[5];

  var det = A[0] * ret[0] + A[1] * ret[4] + A[2] * ret[8] + A[3] * ret[12];

  if (det === 0) {
    throw "no inverse";
  }

  return dali.matrixByScalar(ret, 1 / det);
};

/**
 * Radian to degree
 * @method degree
 * @param {float} radians
 * @return {float} degrees
 */
dali.degree = function(radians) {
  "use strict";
  return (radians * 180.0) / Math.PI;
};

/**
 * Degree to Radians
 * @method radian
 * @param {float} degree
 * @return {float} radian
 */
dali.radian = function(degrees) {
  "use strict";
  return (degrees / 180.0) * Math.PI;
};

/**
 * Length of a vector
 * @method vectorLength
 * @param {array} Vector4
 * @return {float} The length of a vector
 */
dali.vectorLength = function(array) {
  "use strict";
  var N = 3; // array.length;

  var length = 0;
  for (var i = 0; i < N; ++i) {
    length += array[i] * array[i];
  }
  return Math.sqrt(length);
};

/**
 * Length of a vector squared
 * @method vectorLengthSquared
 * @param {array} Vector4
 * @return {float} The length of a vector squared
 */
dali.vectorLengthSquared = function(array) {
  "use strict";
  var N = 3; // array.length;

  var length = 0;
  for (var i = 0; i < N; ++i) {
    length += array[i] * array[i];
  }
  return length;
};

/**
 * Normalized vector
 * @method normalize
 * @param {array} Vector4
 * @return {float} The normalized vector
 */
dali.normalize = function(array) {
  "use strict";
  var N = 3; // array.length;

  var length = 0;
  for (var i = 0; i < 3; ++i) {
    length += array[i] * array[i];
  }
  length = Math.sqrt(length);

  if (length !== 0) {
    var ret = [];
    for (i = 0; i < N; ++i) {
      ret.push(array[i] / length);
    }
    for (i = N; i < array.length; ++i) {
      ret.push(array[i]);
    }
    return ret;
  } else {
    return array;
  }

};

/**
 * AxisAngle conversion to Quaternion
 * @method axisAngleToQuaternion
 * @param {array} axisAngle Vector4 [Axis.x, Axis.y, Axis.z, Angle]
 * @return {array} Quaternion
 */
dali.axisAngleToQuaternion = function(axisAngle) {
  "use strict";
  var an = dali.normalize(axisAngle);
  var angle = axisAngle[axisAngle.length - 1];
  var halfTheta = angle * 0.5;
  var sinThetaByTwo = Math.sin(halfTheta);
  var cosThetaByTwo = Math.cos(halfTheta);
  return [an[0] * sinThetaByTwo,
          an[1] * sinThetaByTwo,
          an[2] * sinThetaByTwo,
          cosThetaByTwo
         ];
};

/**
 * Vector3 dot product
 * @method vectorDot
 * @param {array} v1 Vector3
 * @param {array} v2 Vector3
 * @return {array} Quaternion
 */
dali.vectorDot = function(v1, v2) {
  "use strict";
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
};

/**
 * Vector4 dot product
 * @method vectorDot
 * @param {array} v1 Vector4
 * @param {array} v2 Vector4
 * @return {float} Dot product
 */
dali.vectorDot4 = function(v1, v2) {
  "use strict";
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3];
};

/**
 * Vector3 cross product
 * @method vectorCross
 * @param {array} v1 Vector3
 * @param {array} v2 Vector3
 * @return {array} Vector3 cross product
 */
dali.vectorCross = function(v1, v2) {
  "use strict";
  var v1X = v1[0];
  var v1Y = v1[1];
  var v1Z = v1[2];

  var v2X = v2[0];
  var v2Y = v2[1];
  var v2Z = v2[2];

  return [v1Y * v2Z - v1Z * v2Y,
          v1Z * v2X - v1X * v2Z,
          v1X * v2Y - v1Y * v2X
         ];
};

/**
 * VectorN dot product
 * @method vectorByScalar
 * @param {array} v1 VectorN
 * @param {array} v2 VectorN
 * @return {array} VectorN * s
 */
dali.vectorByScalar = function(v1, s) {
  "use strict";
  var ret = [];
  for (var i = 0, len = v1.length; i < len; i++) {
    ret.push(v1[i] * s);
  }
  return ret;
};

/**
 * VectorN dot product
 * @method vectorAdd
 * @param {array} v1 VectorN
 * @param {array} v2 VectorN
 * @param {array} ..vN VectorN
 * @return {array} v1 + v2 + ... + vN
 */
dali.vectorAdd = function() {
  "use strict";
  var ret = arguments[0];
  var l = ret.length;
  for (var i = 1, len = arguments.length; i < len; i++) {
    var v = arguments[i];
    for (var j = 0; j < l; j++) {
      ret[j] += v[j];
    }
  }
  return ret;
};

/**
 * Quaternion by quaternion
 * @method quatByQuat
 * @param {array} q1 Quaternion
 * @param {array} q2 Quaternion
 * @return {array} Quaternion
 */
dali.quatByQuat = function(q1, q2) {
  "use strict";
  var q1X = q1[0];
  var q1Y = q1[1];
  var q1Z = q1[2];
  var q1W = q1[3];

  var q2X = q2[0];
  var q2Y = q2[1];
  var q2Z = q2[2];
  var q2W = q2[3];

  return [q1Y * q2Z - q1Z * q2Y + q1W * q2X + q1X * q2W,
          q1Z * q2X - q1X * q2Z + q1W * q2Y + q1Y * q2W,
          q1X * q2Y - q1Y * q2X + q1W * q2Z + q1Z * q2W,
          q1W * q2W - dali.vectorDot(q1, q2)
         ];
};

/**
 * Quaternion to Vector4 Axis angle
 * @method quaternionToAxisAngle
 * @param {array} q Quaternion
 * @return {array} Vector4 [Axis.x, Axis.y, Axis.z, Angle]
 */
dali.quaternionToAxisAngle = function(q) {
  "use strict";
  var angle = Math.acos(q[3]);
  var sine = Math.sin(angle);

  if (sine === 0.0) {
    throw "Cannot convert quaternion";
  }

  var sinfThetaInv = 1.0 / sine;

  return [q[0] * sinfThetaInv,
          q[1] * sinfThetaInv,
          q[2] * sinfThetaInv,
          angle * 2.0
         ];
};

/**
 * Euler angles to Quaternion
 * @method eulerToQuaternion
 * @param {float} rxPitch Euler radians pitch
 * @param {float} ryYaw Euler radians yaw
 * @param {float} rzRoll Euler radians roll
 * @return {array} Quaternion
 */
dali.eulerToQuaternion = function(rXPitch, rYYaw, rZRoll)
{
  var halfX = 0.5 * rXPitch;
  var halfY = 0.5 * rYYaw;
  var halfZ = 0.5 * rZRoll;

  var cosX2 = Math.cos(halfX);
  var cosY2 = Math.cos(halfY);
  var cosZ2 = Math.cos(halfZ);

  var sinX2 = Math.sin(halfX);
  var sinY2 = Math.sin(halfY);
  var sinZ2 = Math.sin(halfZ);

  return [ cosZ2 * cosY2 * sinX2 - sinZ2 * sinY2 * cosX2,
           cosZ2 * sinY2 * cosX2 + sinZ2 * cosY2 * sinX2,
           sinZ2 * cosY2 * cosX2 - cosZ2 * sinY2 * sinX2,
           cosZ2 * cosY2 * cosX2 + sinZ2 * sinY2 * sinX2 ];
};

/**
 * Euler angles to Vector4 Axis angle
 * @method eulerToAxisAngle
 * @param {float} eulerInDegrees
 * @return {array} Vector4 [Axis.x, Axis.y, Axis.z, Angle]
 */
dali.eulerToAxisAngle = function(eulerInDegrees)
{
  var q = dali.eulerToQuaternion(dali.radian(eulerInDegrees[0]),dali.radian(eulerInDegrees[1]), dali.radian(eulerInDegrees[2]));
  var aa = dali.quaternionToAxisAngle(q);
  aa[3] = dali.degree(aa[3]); // @todo - radian?
  return aa;
};

/**
 * Axis angle to Euler
 * @method axisAngleToEuler
 * @param {float} axisAngle [Axis.x, Axis.y, Axis.z, Angle]
 * @return {array} Vector4 [roll, pitch, yaw]
 */
dali.axisAngleToEuler = function(axisAngle)
{
  // presume return from dali.js is degrees
  axisAngle[3] = dali.radian(axisAngle[3]);
  var q = dali.axisAngleToQuaternion(axisAngle);
  return dali.quaternionToEuler(q).map(dali.degree);
};

/**
 * Euler angles to Vector4 Axis angle
 * @method quaternionToMatrix
 * @param {float} axisAngle [Axis.x, Axis.y, Axis.z, Angle]
 * @return {array} Vector4
 */
dali.quaternionToMatrix = function(q) {
  "use strict";
  var x = q[0];
  var y = q[1];
  var z = q[2];
  var w = q[3];
  var xx = x * x;
  var yy = y * y;
  var zz = z * z;
  var xy = x * y;
  var xz = x * z;
  var wx = w * x;
  var wy = w * y;
  var wz = w * z;
  var yz = y * z;

  var m = [1, 0, 0, 0,
           0, 1, 0, 0,
           0, 0, 1, 0,
           0, 0, 0, 1];

  m[0] = 1.0 - 2.0 * (yy + zz);
  m[1] = 2.0 * (xy + wz);
  m[2] = 2.0 * (xz - wy);
  m[3] = 0.0;

  m[4] = 2.0 * (xy - wz);
  m[5] = 1.0 - 2.0 * (xx + zz);
  m[6] = 2.0 * (yz + wx);
  m[7] = 0.0;

  m[8] = 2.0 * (xz + wy);
  m[9] = 2.0 * (yz - wx);
  m[10] = 1.0 - 2.0 * (xx + yy);
  m[11] = 0.0;

  m[12] = 0.0;
  m[13] = 0.0;
  m[14] = 0.0;
  m[15] = 1.0;

  return m;
};

/**
 * Quaternion to Euler
 * @method quaternionToEuler
 * @param {array} q Quaternion
 * @return {array} Vector3 [roll, pitch, yaw]
 */
dali.quaternionToEuler = function(q) {
  "use strict";
  var x = q[0];
  var y = q[1];
  var z = q[2];
  var w = q[3];

  var sqw = w * w;
  var sqx = x * x;
  var sqy = y * y;
  var sqz = z * z;

  return [ Math.atan2(2.0 * (y * z + x * w), -sqx - sqy + sqz + sqw),
           Math.asin(-2.0 * (x * z - y * w)),
           Math.atan2(2.0 * (x * y + z * w), sqx - sqy - sqz + sqw)];
};


/**
 * Gets screen coordinates of world position
 * @method worldToScreen
 * @param {array} position array
 * @param {Dali.RenderTask} renderTask Dali RenderTask object
 * @return {array} Vector3 Screen position
 */
dali.worldToScreen = function(position, renderTask) {
  "use strict";
  var useFirstRenderTask = false;

  if (typeof renderTask === "undefined") {
    useFirstRenderTask = true;
  } else if (renderTask === null) { // null is an object
    useFirstRenderTask = true;
  }

  if (useFirstRenderTask) {
    var tasks = dali.stage.getRenderTaskList();
    renderTask = tasks.getTask(0);
    tasks.delete(); // wrapper
  }

  var camera = renderTask.getCameraActor();
  var pos = renderTask.getCurrentViewportPosition();
  var size = renderTask.getCurrentViewportSize();

  var mat = dali.matrixByMatrix(camera.viewMatrix, camera.projectionMatrix);
  var p = dali.matrixByVector(mat, position);
  var depthRange = [0, 1];
  var viewport = [pos[0], pos[1], size[0], size[1]];

  var div;
  // homogenous divide
  if(0.0 === p[3]) {
    div = 1.0;
  } else {
    div = 1 / p[3];
  }

  camera.delete(); // wrapper

  return [
    (1 + p[0] * div) * viewport[2] / 2 + viewport[0], (1 - p[1] * div) * viewport[3] / 2 + viewport[1], (p[2] * div) * (depthRange[1] - depthRange[0]) + depthRange[0],
    div
  ];
};

/**
 * Gets matrix identity
 * @method matrixIdentity
 * @return {array} Matrix4 identity
 */
dali.matrixIdentity = function() {
  "use strict";
  return [1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1
         ];
};

/**
 * Gets matrix identity with position transformation
 * @method matrixTransform
 * @param {float} x X position
 * @param {float} y Y position
 * @param {float} z Z position
 * @return {array} Matrix4
 */
dali.matrixTransform = function(x, y, z) {
  "use strict";
  return [1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          x, y, z, 1
         ];
};

/**
 * Gets matrix identity with position transformation
 * @method screenToPlaneLocal
 * @param {float} screenX Screen X position
 * @param {float} screenY Screen Y position
 * @param {Dali.RenderTask} Dali RenderTask
 * @param {array} planeOrientationMatrix
 * @param {float} planeWidth
 * @param {float} planeHeight
 * @return {array} Local coordinates
 */
dali.screenToPlaneLocal = function(screenX, screenY, renderTask, planeOrientationMatrix, planeWidth, planeHeight) {
  "use strict";

  var camera = renderTask.getCameraActor();

  var pos = renderTask.getCurrentViewportPosition();
  var size = renderTask.getCurrentViewportSize();
  var viewportX = pos[0];
  var viewportY = pos[1];
  var viewportW = size[0];
  var viewportH = size[1];
  var modelView = dali.matrixByMatrix(planeOrientationMatrix, camera.viewMatrix);

  var inverseMvp = dali.matrixInverse(
    dali.matrixByMatrix(modelView, camera.projectionMatrix));

  var screenPos = [screenX - viewportX,
                   viewportH - (screenY - viewportY),
                   0.0, 1.0
                  ];

  screenPos[2] = 0.0;

  var oglScreenPos = [(screenPos[0] / viewportW) * 2 - 1, (screenPos[1] / viewportH) * 2 - 1, (screenPos[2]) * 2 - 1,
                      1
                     ];


  var nearPoint = dali.matrixByVector(inverseMvp, oglScreenPos);

  if (nearPoint[3] === 0.0) {
    throw "Unproject near fails";
  }

  nearPoint[3] = 1 / nearPoint[3];
  nearPoint[0] = nearPoint[0] * nearPoint[3];
  nearPoint[1] = nearPoint[1] * nearPoint[3];
  nearPoint[2] = nearPoint[2] * nearPoint[3];


  oglScreenPos[2] = 1.0 * 2 - 1;

  var farPoint = dali.matrixByVector(inverseMvp, oglScreenPos);

  if (farPoint[3] === 0.0) {
    throw "Unproject far fails";
  }

  farPoint[3] = 1 / farPoint[3];
  farPoint[0] = farPoint[0] * farPoint[3];
  farPoint[1] = farPoint[1] * farPoint[3];
  farPoint[2] = farPoint[2] * farPoint[3];

  if (!((farPoint[2] < 0) && (nearPoint[2] > 0))) {
    throw "ray not crossing xy plane";
  }

  var dist = nearPoint[2] / (nearPoint[2] - farPoint[2]);

  var intersect = [nearPoint[0] + (farPoint[0] - nearPoint[0]) * dist,
                   nearPoint[1] + (farPoint[1] - nearPoint[1]) * dist,
                   0.0
                  ];

  intersect[0] = intersect[0] + planeWidth * 0.5;
  intersect[1] = intersect[1] + planeHeight * 0.5;

  return intersect;
};

/**
 * Gets matrix identity with position transformation
 * @method screenToLocal
 * @param {float} screenX Screen X position
 * @param {float} screenY Screen Y position
 * @param {Dali.Actor} actor Dali Actor
 * @param {Dali.RenderTask} renderTask Dali RenderTask
 * @return {array} Local coordinates
 */
dali.screenToLocal = function(screenX, screenY, actor, renderTask) {
  "use strict";
  return dali.screenToPlaneLocal(screenX, screenY, renderTask, actor.worldMatrix, actor.size[0], actor.size[1]);
};

/**
 * Screen to local coordinates in the XY plane
 * @method screenToXY
 * @param {float} screenX Screen X position
 * @param {float} screenY Screen Y position
 * @param {Dali.Actor} actor Dali Actor
 * @param {Dali.RenderTask} renderTask Dali RenderTask
 * @return {array} Local coordinates
 */
dali.screenToXY = function(screenX, screenY, actor, renderTask) {
  "use strict";
  var size = dali.stage.getSize();
  return dali.screenToPlaneLocal(screenX, screenY,
                                 renderTask,
                                 dali.matrixIdentity(),
                                 size[0],
                                 size[1]);
};

/**
 * Screen to local coordinates in the YZ plane
 * @method screenToYZ
 * @param {float} screenX Screen X position
 * @param {float} screenY Screen Y position
 * @param {Dali.Actor} actor Dali Actor
 * @param {Dali.RenderTask} renderTask Dali RenderTask
 * @return {array} Local coordinates
 */
dali.screenToYZ = function(screenX, screenY, actor, renderTask) {
  "use strict";
  var size = dali.stage.getSize();
  var q = dali.axisAngleToQuaternion( [0, 1, 0, dali.radian(90)] );
  return dali.screenToPlaneLocal(screenX, screenY,
                                 renderTask,
                                 dali.quaternionToMatrix(q),
                                 size[0],
                                 size[1]);
};

/**
 * Screen to local coordinates in the XZ plane
 * @method screenToXZ
 * @param {float} screenX Screen X position
 * @param {float} screenY Screen Y position
 * @param {Dali.Actor} actor Dali Actor
 * @param {Dali.RenderTask} renderTask Dali RenderTask
 * @return {array} Local coordinates
 */
dali.screenToXZ = function(screenX, screenY, actor, renderTask) {
  "use strict";
  var size = dali.stage.getSize();
  var q = dali.axisAngleToQuaternion( [1, 0, 0, dali.radian(90)] );
  return dali.screenToPlaneLocal(screenX, screenY,
                                 renderTask,
                                 dali.quaternionToMatrix(q),
                                 size[0],
                                 size[1]);
};

/**
 * Screen coordinates for the given renderTask
 * @method screenCoordinates
 * @param {Dali.Actor} actor Dali Actor
 * @param {Dali.RenderTask} renderTask Dali RenderTask
 * @return {array} Local coordinates
 */
dali.screenCoordinates = function(actor, renderTask) {
  "use strict";
  var size = actor.size;
  var w2 = size[0] / 2;
  var h2 = size[1] / 2;
  var actorWorldMatrix = actor.worldMatrix;
  var actorWorldPosition = actor.worldPosition;

  return {
    topLeft: dali.worldToScreen(dali.matrixByVector(actorWorldMatrix, [-w2, -h2, 0]), renderTask),
    topRight: dali.worldToScreen(dali.matrixByVector(actorWorldMatrix, [+w2, -h2, 0]), renderTask),
    bottomRight: dali.worldToScreen(dali.matrixByVector(actorWorldMatrix, [+w2, +h2, 0]), renderTask),
    bottomLeft: dali.worldToScreen(dali.matrixByVector(actorWorldMatrix, [-w2, +h2, 0]), renderTask),
    centre: dali.worldToScreen(actorWorldPosition, renderTask)
  };
};

/**
 * Screen coordinates for the given renderTask
 * @method screenCoordinates
 * @param {Dali.Actor} actor Dali Actor
 * @param {Dali.RenderTask} renderTask Dali RenderTask
 * @return {array} Local coordinates
 */
dali.screenToActor = function(actor, screenPos, renderTask) {
  "use strict";
  // , function will return coordinates in relation to actor`s anchorPoint (client coordinates).
  var useFirstRenderTask = false;

  if (typeof renderTask === "undefined") {
    useFirstRenderTask = true;
  } else if (renderTask === null) { // null is an object
    useFirstRenderTask = true;
  }

  if (useFirstRenderTask) {
    var tasks = dali.stage.getRenderTaskList();
    renderTask = tasks.getTask(0);
    tasks.delete(); // wrapper
  }

  var camera = renderTask.getCameraActor();
  var vpp = renderTask.getCurrentViewportPosition();
  var vps = renderTask.getCurrentViewportSize();

  var mat = dali.matrixByMatrix(camera.projectionMatrix, camera.viewMatrix);

  var inverseMvp = dali.matrixInverse(mat);

  var x = screenPos[0];
  var y = screenPos[1];
  var z = screenPos[2];

  var objectPos = dali.matrixByVector(inverseMvp,
                                      // normalized +-1
                                      [((x - vpp[0]) / vps[0]) * 2.0 - 1.0, ((y - vpp[1]) / vps[1]) * 2.0 - 1.0, (z * 2.0) - 1.0,
                                       1.0
                                      ]);

  if (objectPos[3] === 0.0) {
    throw "Cannot find screen Position";
  }

  objectPos[3] = 1 / objectPos[3];

  return [objectPos[0] * objectPos[3],
          objectPos[1] * objectPos[3],
          objectPos[2] * objectPos[3],
          objectPos[3]
         ];
};


//------------------------------------------------------------------------------
//
// Utils Module
//
//------------------------------------------------------------------------------

/**
 * Cache to fix the dali get/set thread issue
 *
 * Property sets are cached and cleared at the renderFinished callback
 */
dali.internalUniqueId = function() {
  "use strict";
};

dali.internalUniqueId.prototype._id = 0;
dali.internalUniqueId.prototype.generateId = function() {
  "use strict";
  return ++dali.internalUniqueId.prototype._id;
};

dali.internalPropertyCacheEnable = true;
dali.internalPropertyCache = {};

/**
 * Merge two objects together in a simplistic key,value merge
 * @method mergeObjects
 * @param {object} o1 first object
 * @param {object} o2 second object
 * @return {object} The merged objects
 */
dali.mergeObjects = function(o1, o2) {
  "use strict";
  for (var p in o2) {
    try {
      // Property in destination object set; update its value.
      if ( o2[p].constructor === Object){
        o1[p] = dali.mergeObjects(o1[p], o2[p]);

      } else {
        o1[p] = o2[p];
      }
    } catch(e) {
      // Property in destination object not set; create it and set its value.
      o1[p] = o2[p];
    }
  }
  return o1;
};

//------------------------------------------------------------------------------
//
// Callbacks Module
//
// Data dali can request during update & render loop.
//
//------------------------------------------------------------------------------

/**
 * Gets a glyph by rendering to a hidden browser canvas
 */
/** @private */
dali.requestedGlyphImage = function(sFontFamily, sFontStyle, fFontSize, iChar) {
  "use strict";

  var buffer = document.createElement("canvas");
  buffer.width = fFontSize;
  buffer.height = fFontSize;
  var ctx = buffer.getContext("2d");

  ctx.font = sFontStyle + " " + fFontSize + "px " + sFontFamily;
  ctx.fillText(String.fromCharCode(iChar), 0, 0 + fFontSize);

  var imageData = ctx.getImageData(0, 0, fFontSize, fFontSize);

  // emscripten checks only for this type if array in converting to strings
  // (getImageData() returns Uint8CheckedArray or some such)
  //  var uint8array = new Uint8Array( imageData.data );

  return imageData.data; // return uint8array; //
};

/** @private */
dali.postRenderFunction = undefined;

/*
 * End of renderering tasks
 *  - Reset the property cache
 */
/** @private */
dali.requestedRenderFinished = function() {
  "use strict";
  // reset the temporary cache
  dali.internalPropertyCache = {};
  if(dali.postRenderFunction) {
    dali.postRenderFunction();
  }
};

dali.setCallbackGetGlyphImage(dali.requestedGlyphImage);
dali.setCallbackRenderFinished(dali.requestedRenderFinished);

//------------------------------------------------------------------------------
//
// Property Marshalling Module
//
// Javascript objects are adorned with dali properties as they are created or
// fetched from the C++ api
//
// Data is marshalled to avoid some of the necessary C++ memory management for
// small property classes.
//
//------------------------------------------------------------------------------
dali.__propertyTypeJsLut = { boolean: dali.PropertyType.BOOLEAN,
                             number: dali.PropertyType.FLOAT,
                             string: dali.PropertyType.STRING };

dali.__propertyValueCtor = {};

[ [dali.PropertyType.INTEGER.value, dali.PropertyValueInteger],
  [dali.PropertyType.FLOAT.value, dali.PropertyValueFloat],
  [dali.PropertyType.STRING.value, dali.PropertyValueString],
  [dali.PropertyType.VECTOR2.value, dali.PropertyValueVector2],
  [dali.PropertyType.VECTOR3.value, dali.PropertyValueVector3],
  [dali.PropertyType.VECTOR4.value, dali.PropertyValueVector4],
  [dali.PropertyType.MATRIX.value, dali.PropertyValueMatrix],
  [dali.PropertyType.MATRIX3.value, dali.PropertyValueMatrix3],
  [dali.PropertyType.ARRAY.value, dali.PropertyValueArray],
  [dali.PropertyType.MAP.value, dali.PropertyValueMap],
  [dali.PropertyType.RECTANGLE.value, dali.PropertyValueIntRect] ].map( function(ab) { dali.__propertyValueCtor[ ab[0] ] = ab[1]; } );

dali.propertyTypeFromJsValue = function(value) {
  return dali.__propertyTypeJsLut[ typeof(value) ];
};

dali.propertyValueCtor = function(propertyType) {
  return dali.__propertyValueCtor[ propertyType.value ];
};

/**
 * Create a Dali PropertyValue from a javascript value.
 * @method DaliPropertyValue
 * @param {object} object to retrieve the property type when value is an object
 * @param {string} name The name of the property
 * @return {Dali.PropertyValue} A Dali PropertyValue which must be '.delete()' when finished with
 */
dali.DaliPropertyValue = function(object, name, value) {
  "use strict";

  var setProperty = false;
  if (typeof (value) === "boolean") {
    setProperty = new dali.PropertyValueBoolean(value);
  } else if (typeof (value) === "number") {
    setProperty = new dali.PropertyValueFloat(value);
  } else if (typeof (value) === "string") {
    setProperty = new dali.PropertyValueString(value);
  } else if (typeof (value) === "object") {
    if (value.constructor === dali.PropertyValueValue) {
      setProperty = value;
    } else {
      var type = false;
      if (object) {
        type = object.getPropertyTypeFromName(name);
      }
      if (type === dali.PropertyType.ROTATION) {
        if (value.length === 3) {
          setProperty = new dali.PropertyValueEuler(value);
        } else {
          setProperty = new dali.PropertyValueAxisAngle(value);
        }
      } else if (value.length) {
        if (type === dali.PropertyType.ARRAY) {
          setProperty = new dali.PropertyValueArray(value);
        } else {
          if (value.length === 2) {
            setProperty = new dali.PropertyValueVector2(value);
          } else if (value.length === 3) {
            setProperty = new dali.PropertyValueVector3(value);
          } else if (value.length === 4) {
            if (type === dali.PropertyType.RECTANGLE) {
              setProperty = new dali.PropertyValueIntRect(value[0], value[1], value[2], value[3]);
            } else {
              setProperty = new dali.PropertyValueVector4(value);
            }
          } else if (value.length === 9) {
            setProperty = new dali.PropertyValueMatrix3(value);
          } else if (value.length === 16) {
            setProperty = new dali.PropertyValueMatrix(value);
          } else {
            throw new Error("Cannot set property");
          }
        }
      } else {
        // todo; I think a map has a length....
        setProperty = new dali.PropertyValueMap(value);
      }
    }
  } else {
    if (object) {
      throw object.toString() + " " + name.toString() + " " + value.toString();
    } else {
      throw name.toString() + " " + value.toString();
    }
  }

  return setProperty;
};

/**
 * Get the value type held in a PropertyValue and call '.delete()' to drop the C++ backing object
 * @method marshallProperty
 * @param {Dali.PropertyValue} p A Dali PropertyValue. This will be '.delete()'ed.
 * @return {any} The value held
 */
/** @private */
dali.marshallProperty = function(p) {
  "use strict";

  var ret;
  var type = p.getType();

  if (type === 0) {
    // raise?
    throw new Error("Property has no type?");
  } else if (type === dali.PropertyType.BOOLEAN.value) {
    ret = p.getBoolean();
  } else if (type === dali.PropertyType.FLOAT.value) {
    ret = p.getFloat();
  } else if (type === dali.PropertyType.INTEGER.value) {
    ret = p.getInteger();
  } else if (type === dali.PropertyType.VECTOR2.value) {
    ret = p.getVector2();
  } else if (type === dali.PropertyType.VECTOR3.value) {
    ret = p.getVector3();
  } else if (type === dali.PropertyType.VECTOR4.value) {
    ret = p.getVector4();
  } else if (type === dali.PropertyType.MATRIX3.value) {
    ret = p.getMatrix3();
  } else if (type === dali.PropertyType.MATRIX.value) {
    ret = p.getMatrix();
  } else if( type === dali.PropertyType.RECTANGLE ) {
    ret = p.getIntRect();
  } else if (type === dali.PropertyType.ROTATION.value) {
    ret = p.getRotation();
  } else if (type === dali.PropertyType.STRING.value) {
    ret = p.getString();
  } else if (type === dali.PropertyType.ARRAY.value) {
    ret = p.getArray();
  } else if (type === dali.PropertyType.MAP.value) {
    ret = p.getMap();
  }
  p.delete();
  return ret;
};

/**
 * Set a value on an object by creating and deleting a Dali PropertyValue object
 * @method marshallSetProperty
 * @param {object} object The object who's property to set from
 * @param {string} name The property name
 * @param {any} value The Javascript value
 */
/** @private */
dali.marshallSetProperty = function(object, name, value) {
  "use strict";
  var setProperty = new dali.DaliPropertyValue(object, name, value);

  if (setProperty) {

    object.setProperty(name, setProperty);

    //console.log("marshallSetProperty set property" + setProperty );
    setProperty.delete();

    if(dali.internalPropertyCacheEnable) {
      // set in cache to fix dali get/set problem
      if("getId" in object ) // only with actors
      {
        var uniqueId = object.getId(); // _uniqueId;
        if (uniqueId !== undefined) {
          var objectDict = dali.internalPropertyCache[uniqueId];
          if (objectDict === undefined) {
            dali.internalPropertyCache[uniqueId] = {};
          }
          dali.internalPropertyCache[uniqueId][name] = value;
        }
      }
    }
  }
};

/**
 * Get a Javascript value from an object by creating and deleting a PropertyValue
 * @method marshallGetProperty
 * @param {object} object The object who's property to get from
 * @param {string} name The property name
 * @return {any} The value of the property
 */
/** @private */
dali.marshallGetProperty = function(object, name) {
  "use strict";

  if(dali.internalPropertyCacheEnable) {
    // is the value live in the cache? if so use that value instead
    if("getId" in object) { // only with actors
      var uniqueId = object.getId(); // _uniqueId;
      if (uniqueId !== undefined) {
        var objectDict = dali.internalPropertyCache[uniqueId];
        if (objectDict !== undefined) {
          var value = dali.internalPropertyCache[uniqueId][name];
          if (value !== undefined) {
            return value;
          }
        }
      }
    }
  }

  var ret;
  var p;
  p = object.getProperty(name);
  if (!p) {
    throw new Error("Property doesnt exist?");
  }
  var type = p.getType();

  if (type === 0) {
    // raise?
    throw new Error("Property has no type?");
  } else if (type === dali.PropertyType.BOOLEAN.value) {
    ret = p.getBoolean();
  } else if (type === dali.PropertyType.FLOAT.value) {
    ret = p.getFloat();
  } else if (type === dali.PropertyType.INTEGER.value) {
    ret = p.getInteger();
  } else if (type === dali.PropertyType.VECTOR2.value) {
    ret = p.getVector2();
  } else if (type === dali.PropertyType.VECTOR3.value) {
    ret = p.getVector3();
  } else if (type === dali.PropertyType.VECTOR4.value) {
    // VECTOR4
    ret = p.getVector4();
  } else if (type === dali.PropertyType.MATRIX3.value) {
    // MATRIX3
    ret = p.getMatrix3();
  } else if (type === dali.PropertyType.MATRIX.value) {
    // MATRIX
    ret = p.getMatrix();
  } else if( type === dali.PropertyType.RECTANGLE ) {
    ret = p.getIntRect();
  } else if (type === dali.PropertyType.ROTATION.value) {
    ret = p.getRotation();
  } else if (type === dali.PropertyType.STRING.value) {
    ret = p.getString();
  } else if (type === dali.PropertyType.ARRAY.value) {
    ret = p.getArray();
  } else if (type === dali.PropertyType.MAP.value) {
    ret = p.getMap();
  }
  p.delete();
  return ret;
};


/**
 * Set property accessors on the given handle for each property found on the handle
 * @method internalSetupProperties
 * @param {Dali.BaseHandle} handle A Dali property holding object
 * @return {Dali.BaseHandle} The same handle which has property accessors.
 */
/** @private */
dali.internalSetupProperties = function(handle) {
  "use strict";
  if(handle.ok()) {
    var props = handle.getProperties();

    for (var i = 0; i < props.size(); i++) {
      // anon function because of closure with defineProperty
      // (if just variable in loop then the variable 'address' is captured, not the value
      //  so it becomes last value set)
      (function(name, object) {
        // @todo Dali error?? name lengths should never be zero
        if (name.length) {
          Object.defineProperty(handle, name, {
            enumerable: true,
            configurable: false,
            get: function() {
              return dali.marshallGetProperty(handle, name);
            },
            set: function(newValue) {
              dali.marshallSetProperty(handle, name, newValue);
            }
          });
        }
      })(props.get(i), handle);
    }

    // handle._uniqueId = dali.internalUniqueId.prototype.generateId();
  }

  return handle;
};

//------------------------------------------------------------------------------
//
// Handle API Module
//
// API Wrappers for some Dali.Handle methods to marshall properties
//
//------------------------------------------------------------------------------

/**
 * Register a new property and add JS style property accessors
 * @method registerProperty
 * @param {string} name The property name
 * @param {any} value Any Javascript value
 * @return {int} The registered properties ID
 */
dali.Handle.prototype.registerProperty = function(name, value) {
  "use strict";
  var ret = -1;

  var propertyValue = new dali.DaliPropertyValue(null, name, value);
  ret = this.__registerProperty(name, propertyValue);
  propertyValue.delete(); // wrapper
  Object.defineProperty(this, name, {
    enumerable: true,
    configurable: false,
    get: function() {
      return dali.marshallGetProperty(this, name);
    },
    set: function(newValue) {
      dali.marshallSetProperty(this, name, newValue);
    }
  });

  return ret;
};

/**
 * Register a new animated property
 * @method registerAnimatedProperty
 * @param {string} name The property name
 * @param {any} value Any Javascript value
 * @return {int} The registered properties ID
 */
dali.Handle.prototype.registerAnimatedProperty = function(name, value) {
  "use strict";
  var ret = -1;

  var propertyValue = new dali.DaliPropertyValue(null, name, value);
  ret = this.__registerAnimatedProperty(name, propertyValue);
  propertyValue.delete(); // wrapper
  Object.defineProperty(this, name, {
    enumerable: true,
    configurable: false,
    get: function() {
      return dali.marshallGetProperty(this, name);
    },
    set: function(newValue) {
      dali.marshallSetProperty(this, name, newValue);
    }
  });
  return ret;
};

//------------------------------------------------------------------------------
//
// Stage Module
//
//------------------------------------------------------------------------------
dali.Stage.prototype.getRootLayer = function() {
  "use strict";
  var root = this.__getRootLayer();
  dali.internalSetupProperties(root);
  return root;
};

//------------------------------------------------------------------------------
//
// PropertyMap Module
//
// API Wrappers for some Dali.PropertyMap methods to marshall properties
//
//------------------------------------------------------------------------------

/**
 * Insert a value into the PropertyMap
 * @method insert
 * @param {string} key The key
 * @param {any} value Any Javascript value
 * @param {PropertyType} propertyType The Dali property type
 */
dali.PropertyMap.prototype.insert = function(key, value, propertyType) {
  "use strict";

  var type = propertyType;

  if( propertyType === undefined ) { // can be optional
    propertyType = dali.propertyTypeFromJsValue(value);
  }

  var constructor = dali.propertyValueCtor(propertyType);

  var setProperty = constructor( value );

  if(setProperty) {
    this.__insert(key, setProperty);
    setProperty.delete();
  }
};

/**
 * Get a value from the PropertyMap
 * @method get
 * @param {string} key The key
 * @return The Javascript value
 */
dali.PropertyMap.prototype.get = function(key) {
  "use strict";
  var p = this.__get(key);

  var ret = dali.marshallProperty(p);

  // p.delete(); // @todo should we delete here?

  return ret;
};

//------------------------------------------------------------------------------
//
// PropertyBuffer Module
//
// API Wrappers for some Dali.PropertyBuffer methods to marshall properties
//
//------------------------------------------------------------------------------
var _propertyTypeInfoList = [
  [ dali.PropertyType.FLOAT.value, { size: 4, length: 1, dataView: Float32Array }],
  [ dali.PropertyType.INTEGER.value, { size: 4, length: 1, dataView: Int32Array }],
  [ dali.PropertyType.VECTOR2.value, { size: 2 * 4, length: 2, dataView: Float32Array }],
  [ dali.PropertyType.VECTOR3.value, { size: 3 * 4, length: 3, dataView: Float32Array }],
  [ dali.PropertyType.VECTOR4.value, { size: 4 * 4, length: 4, dataView: Float32Array }],
  [ dali.PropertyType.MATRIX3.value, { size: 9 * 4, length: 9, dataView: Float32Array }],
  [ dali.PropertyType.MATRIX.value, { size: 16 * 4, length: 16, dataView: Float32Array }]
];

var _propertyTypeInfo = {};
function _createPropertyBuffer() {
  "use strict";
  for(var i = 0; i < _propertyTypeInfoList.length; i++) {
    _propertyTypeInfo[ _propertyTypeInfoList[i][0] ] = _propertyTypeInfoList[i][1];
  }
}

_createPropertyBuffer();

/**
 * Create a Dali.PropertyBuffer from an info dictionary
 * @method createPropertyBuffer
 * @param {object} info
 * @param {any} value Any Javascript value
 * @param {PropertyType} propertyType The Dali property type
 * @example
 * var verts = createPropertyBuffer( {format: [ ["apos", dali.PropertyType.VECTOR2],
 *                                              ["acol", dali.PropertyType.VECTOR4] ],
 *                                    data: { "apos": [ [-halfQuadSize, -halfQuadSize],
 *                                                      [+halfQuadSize, -halfQuadSize],
 *                                                      [-halfQuadSize, +halfQuadSize],
 *                                                      [+halfQuadSize, +halfQuadSize]
 *                                                    ],
 *                                            "acol": [ [0, 0, 0, 1],
 *                                                      [1, 0, 1, 1],
 *                                                      [0, 1, 0, 1],
 *                                                      [1, 1, 1, 1]
 *                                                    ]
 *                                          }
 *                                   }
 */
dali.createPropertyBuffer = function(info) {
  "use strict";
  var format = new dali.PropertyMap();
  var dataLength;
  var recordSize = 0;
  var i;
  for(i = 0; i < info.format.length; i++) {
    format.insert(info.format[i][0], info.format[i][1].value, dali.PropertyType.INTEGER);
    if(dataLength === undefined) {
      dataLength = info.data[info.format[i][0]].length;
    }
    assert(info.data[info.format[i][0]]);
    assert(dataLength === info.data[info.format[i][0]].length);
    recordSize += _propertyTypeInfo[info.format[i][1].value].size;
  }

  var buffer = new ArrayBuffer(dataLength * recordSize);

  var recordOffset = 0;
  var offset = 0;
  for(i = 0; i < dataLength; i++) {
    for(var j = 0; j < info.format.length; j++) {
      var name = info.format[j][0];
      var type = info.format[j][1].value;
      var length = _propertyTypeInfo[type].length;
      var DataView = _propertyTypeInfo[type].dataView;
      var view = new DataView(buffer, recordOffset + offset, length);
      offset += _propertyTypeInfo[type].size;
      if(length === 1) {
        view[0] = info.data[name][i];
      } else {
        for(var k = 0; k < length; k++) {
          view[k] = info.data[name][i][k];
        }
      }
    }
    offset = 0;
    recordOffset += recordSize;
  }

  var propertyBuffer = new dali.PropertyBuffer(format, dataLength);

  propertyBuffer.setData(buffer);

  format.delete(); //

  return propertyBuffer;
};

//------------------------------------------------------------------------------
//
// Actor Module
//
// API Wrappers for some Dali.PropertyBuffer methods to marshall properties
//
//------------------------------------------------------------------------------

/**
 * Gets a parent with JS style property accessors
 * @method getParent
 * @return The parent
 */
dali.Actor.prototype.getParent = function() {
  "use strict";
  var bareActor = this.__getParent();
  if(!bareActor.ok()) {
    bareActor.delete(); // wrapper
    bareActor = null;
  } else {
    // add properties to the bare Actor
    dali.internalSetupProperties(bareActor);
  }
  return bareActor;
};

/**
 * Finds child by ID and adorns with JS style property accessors
 * @method findChildById
 * @param {int} index The ID of the child
 * @return The found child or null
 */
dali.Actor.prototype.findChildById = function(index) {
  "use strict";
  var bareActor = this.__findChildById(index);
  if(!bareActor.ok()) {
    bareActor.delete(); // wrapper
    bareActor = null;
  } else {
    dali.internalSetupProperties(bareActor);
  }
  return bareActor;
};

/**
 * Finds child by name and adorns with JS style property accessors
 * @method findChildByName
 * @param {string} name The ID of the child
 * @return The found child or null
 */
dali.Actor.prototype.findChildByName = function(name) {
  "use strict";
  var bareActor = this.__findChildByName(name);
  if(!bareActor.ok()) {
    bareActor.delete(); // wrapper
    bareActor = null;
  } else {
    dali.internalSetupProperties(bareActor);
  }
  return bareActor;
};

/**
 * Gets child at child index and adorns with JS style property accessors
 * @method getChildAt
 * @param {int} index The ID of the child
 * @return The found child or null
 */
dali.Actor.prototype.getChildAt = function(index) {
  "use strict";
  var bareActor = this.__getChildAt(index);
  if(!bareActor.ok()) {
    bareActor.delete(); // wrapper
    bareActor = null;
  } else {
    dali.internalSetupProperties(bareActor);
  }
  return bareActor;
};

/*
 * add children of actor to collection in depth first manner
 */
/** @private */
dali.internalDepthFirstCollection = function(actor, collection) {
  "use strict";
  for (var i = 0; i < actor.getChildCount(); i++) {
    var a = actor.getChildAt(i); // adds properties in dotted
    collection.push(a);
    dali.internalDepthFirstCollection(a, collection);
  }
};

/**
 * Finds all children of the actor and adorns with JS style property accessors
 * @method findAllChildren
 * @return A list of children
 */
dali.Actor.prototype.findAllChildren = function() {
  "use strict";
  var col = [];
  dali.internalDepthFirstCollection(this, col);
  return col;
};

/**
 * Gets a childFinds all children of the actor and adorns with JS style property accessors
 * @method getChildren
 * @return A list of children
 */
dali.Actor.prototype.getChildren = function() {
  "use strict";
  var col = [];
  for (var i = 0, len = this.getChildCount(); i < len; i++) {
    var c = this.getChildAt(i);
    col.push(c);
  }
  return col;
};

/**
 * 'directChildren' kept for GUIBuilder support
 * @deprecated
 */
dali.Actor.prototype.directChildren = dali.Actor.prototype.getChildren;

/**
 * Connects a callback to a signal by name
 * @method connect
 * @param {string} signalName The signal to connect to
 * @param {function} callback The callback to call
 * @param {Dali.SignalHolder} The signal holder object that can signal connection deletion
 * @return true if connection was possible
 */
dali.Actor.prototype.connect = function(signalName, callback, signalHolder) {
  "use strict";
  // wrap in closure so we can setup properties in . notation
  // and add actor methods to c++ raw Actor
  if(signalHolder === undefined) {
    // default js signal holder if none provided
    signalHolder = dali.jsSignalHolder;
  }

  return this.__connect( signalHolder,
                         signalName,
                         (function(cb) {
                           return function() {
                             var args = [dali.internalSetupProperties(arguments[0])];
                             for(var i = 1; i < arguments.length; i++) {
                               args.push( arguments[i] );
                             }
                             cb.apply(null, args);
                           };
                         })(callback)
                       );
};

/**
 * Connects a callback to a property notification
 * @method setPropertyNotification
 * @param {string} property The property name
 * @param {string} condition The condition [False,LessTHan,GreaterThan,Inside,Outside,Step,VariableStep]
 * @param {any} arg0 The first property notification argument
 * @param {any} arg1 The second property notification argument
 * @param {function} callback The callback function
 * @param {Dali.SignalHolder} The signal holder object that can signal connection deletion
 * @return true if connection was possible
 */
dali.Actor.prototype.setPropertyNotification = function(property, condition, arg0, arg1, callback, signalHolder) {
  "use strict";

  if(signalHolder === undefined) {
    // default js signal holder if none provided
    signalHolder = dali.jsSignalHolder;
  }

  var index = this.getPropertyIndex(property);

  this.__setPropertyNotification(signalHolder, index, condition, arg0, arg1, callback);
};

/**
 * Gets the renderer by index
 * @method getRendererAt
 * @param {int} index The index of the renderer
 * @return The Render or null
 */
dali.Actor.prototype.getRendererAt = function(index) {
  "use strict";
  var renderer = this.__getRendererAt(index);
  if(!renderer.ok()) {
    renderer.delete(); // wrapper
    renderer = null;
  } else {
    dali.internalSetupProperties(renderer);
  }
  return renderer;
};

/** private */
dali.__ActorConstructor = dali.Actor;

/**
 * Construtor that adorns with JS style property accessors
 * @return The wrapped Dali.Actor object
 */
dali.Actor = function() {
  "use strict";
  var a = new dali.__ActorConstructor();
  dali.internalSetupProperties(a);
  return a;
};

//------------------------------------------------------------------------------
//
// ShaderEffect Module
//
//------------------------------------------------------------------------------
dali.__ShaderEffectConstructor = dali.ShaderEffect;
dali.ShaderEffect = function() {
  "use strict";
  var a = new dali.__PathConstructor();
  dali.internalSetupProperties(a);
  return a;
};

//------------------------------------------------------------------------------
//
// New Mesh Module
//
//------------------------------------------------------------------------------
dali.__ShaderConstructor = dali.Shader;
dali.Shader = function(vertex, fragment, hints) {
  "use strict";
  var a = new dali.__ShaderConstructor(vertex, fragment, hints);
  dali.internalSetupProperties(a);
  return a;
};

dali.__MaterialConstructor = dali.Material;
dali.Material = function(shader) {
  "use strict";
  var a = new dali.__MaterialConstructor(shader);
  dali.internalSetupProperties(a);
  return a;
};

dali.__RendererConstructor = dali.Renderer;
dali.Renderer = function(geometry, material) {
  "use strict";
  var a = new dali.__RendererConstructor(geometry, material);
  dali.internalSetupProperties(a);
  return a;
};

//------------------------------------------------------------------------------
//
// Animation Module
//
//------------------------------------------------------------------------------
dali.__PathConstructor = dali.Path;
dali.Path = function() {
  "use strict";
  var a = new dali.__PathConstructor();
  dali.internalSetupProperties(a);
  return a;
};

/**
 * animateTo a value
 * @method animateTo
 * @param {object} The object
 * @param {string} propertyName The objects property name
 * @param {any} value The value
 * @param {string} alphaFunction The alpha function
 * @param {float} delay The delay
 * @param {float} duration The duration
 */
dali.Animation.prototype.animateTo = function(object, propertyName, value, alphaFunction, delay, duration) {
  "use strict";
  var propertyValue = new dali.DaliPropertyValue(object, propertyName, value);
  if (propertyValue) {
    this.__animateTo(object, propertyName, propertyValue, alphaFunction, delay, duration);
    propertyValue.delete();
  } else {
    throw new Error("Unknown property?");
  }
};

/**
 * animateBy a value
 * @method animateBy
 * @param {object} The object
 * @param {string} propertyName The objects property name
 * @param {any} value The value
 * @param {string} alphaFunction The alpha function
 * @param {float} delay The delay
 * @param {float} duration The duration
 */
dali.Animation.prototype.animateBy = function(object, propertyName, value, alphaFunction, delay, duration) {
  "use strict";
  var propertyValue = new dali.DaliPropertyValue(object, propertyName, value);
  if (propertyValue) {
    this.__animateBy(object, propertyName, propertyValue, alphaFunction, delay, duration);
    propertyValue.delete();
  } else {
    throw new Error("Unknown property?");
  }
};

/**
 * Animate a Path
 * @method animatePath
 * @param {object} The object
 * @param {Dali.Path} pathObject The path object
 * @param {array} forward The path forward vector
 * @param {string} alphaFunction The alpha function
 * @param {float} delay The delay
 * @param {float} duration The duration
 */
dali.Animation.prototype.animatePath = function(object, pathObject, forward, alphaFunction, delay, duration) {
  "use strict";
  this.__animatePath(object, pathObject, forward, alphaFunction, delay, duration);
};

/**
 * animateBetween a value
 * @method animateBetween
 * @param {object} The object
 * @param {string} propertyName The objects property name
 * @param {dali.KeyFrames} keyFrames The keyframes
 * @param {string} alphaFunction The alpha function
 * @param {float} delay The delay
 * @param {float} duration The duration
 */
dali.Animation.prototype.animateBetween = function(object, propertyName, keyFrames, alphaFunction, delay, duration, interpolation) {
  "use strict";
  var propertyValue;

  var daliKeyFrames = new dali.KeyFrames();

  for(var i = 0; i < keyFrames.length; i++) {
    if(keyFrames[i].length > 2) { // has alpha
      propertyValue = dali.DaliPropertyValue(null, null, keyFrames[i][1]);
      if(!propertyValue) {
        throw new Error("Unknown property?");
      }
      daliKeyFrames.add(keyFrames[i][0], propertyValue, keyFrames[i][2]);
      propertyValue.delete();
    } else {
      propertyValue = dali.DaliPropertyValue(null, null, keyFrames[i][1]);
      if(!propertyValue) {
        throw new Error("Unknown property?");
      }
      daliKeyFrames.add(keyFrames[i][0], propertyValue);
      propertyValue.delete();
    }
  }

  this.__animateBetween(object, propertyName, daliKeyFrames, alphaFunction, delay, duration, interpolation);

  daliKeyFrames.delete();

};

//------------------------------------------------------------------------------
//
// RenderTask Module
//
//------------------------------------------------------------------------------
dali.RenderTask.prototype.getCameraActor = function() {
  "use strict";
  var a = this.__getCameraActor();
  if (a.ok()) {
    dali.internalSetupProperties(a);
  }
  return a;
};

Object.defineProperty(dali.RenderTask.prototype, "x", {
  enumerable: true,
  configurable: false,
  get: function() {
    return this.getCurrentViewportPosition()[0];
  },
  set: function(v) {
    var pos = this.getCurrentViewportPosition();
    this.setViewportPosition(v, pos[1]);
  }
});

Object.defineProperty(dali.RenderTask.prototype, "y", {
  enumerable: true,
  configurable: false,
  get: function() {
    return this.getCurrentViewportPosition()[1];
  },
  set: function(v) {
    var pos = this.getCurrentViewportPosition();
    this.setViewportPosition(pos[0], v);
  }
});

Object.defineProperty(dali.RenderTask.prototype, "width", {
  enumerable: true,
  configurable: false,
  get: function() {
    return this.getCurrentViewportSize()[0];
  },
  set: function(v) {
    var pos = this.getCurrentViewportSize();
    this.setViewportSize(v, pos[1]);
  }
});

Object.defineProperty(dali.RenderTask.prototype, "height", {
  enumerable: true,
  configurable: false,
  get: function() {
    return this.getCurrentViewportSize()[1];
  },
  set: function(v) {
    var pos = this.getCurrentViewportSize();
    this.setViewportSize(pos[0], v);
  }
});

//------------------------------------------------------------------------------
//
// Solid Actor Module
//
//------------------------------------------------------------------------------

/**
 * Create a solid color actor
 * @method createSolidColorActor
 * @param {array} color The color
 * @param {bool} border Whether to add a border
 * @param {array} color The border color
 * @param {float} borderSize The size of a border
 * @return {Dali.Actor} The Dali actor
 */
dali.createSolidColorActor = function(color, border, borderColor, borderSize) {
  "use strict";
  var a = dali.__createSolidColorActor(color, border, borderColor, borderSize);
  dali.internalSetupProperties(a);
  return a;
};

//------------------------------------------------------------------------------
//
// Mesh import support Module
//
//------------------------------------------------------------------------------
function ObjectLoader(fileObject) {
  "use strict";
  // cached
  this.self = this;
  this.meshByUUID = {};
  this.geomByUUID = {};
  this.matByUUID = {};

  this.fileObject = fileObject;
}

function __longToArray(v) {
  "use strict";
  return [((v >> 24) & 0xFF) / 255.0, ((v >> 16) & 0xFF) / 255.0, ((v >> 8) & 0xFF) / 255.0, (v & 0xFF) / 255.0];
}

function __isBitSet(value, bit) {
  "use strict";
  return (value & (1 << bit));
}

ObjectLoader.prototype.__getMaterial = function(uuid) {
  "use strict";
  if (!(uuid in this.matByUUID)) {
    for (var i = 0, len = this.fileObject.materials.length; i < len; i++) {
      var f_mat = this.fileObject["materials"][i];
      skewer.log(i + ":" + f_mat["uuid"] + " " + (f_mat["uuid"] === uuid));
      if (f_mat["uuid"] === uuid) {
        assert(f_mat["type"] === "MeshPhongMaterial");
        var mat = new dali.MaterialWrapper(uuid);
        mat.setDiffuseColor(__longToArray(f_mat["color"]));
        mat.setAmbientColor(__longToArray(f_mat["ambient"]));
        mat.setSpecularColor(__longToArray(f_mat["specular"]));
        mat.setEmissiveColor(__longToArray(f_mat["emmissive"]));
        mat.setShininess(f_mat["shininess"]);
        this.matByUUID[uuid] = mat;
        break;
      }
    }
  }
  return this.matByUUID[uuid];
};

ObjectLoader.prototype.__getMeshData = function(uuid, uuid_material) {
  "use strict";
  if (!(uuid in this.meshByUUID)) {
    for (var i = 0, len = this.fileObject["geometries"].length; i < len; i++) {
      var f_geom = this.fileObject["geometries"][i];
      if (f_geom["uuid"] === uuid) {
        var f_indices, // file data
            f_posns,
            f_norms,
            f_uvs,
            f_faces;

        if (!("metadata" in f_geom)) {
          f_geom["metadata"] = {
            "type": ""
          }; // Warning: modified input!?
        }

        if ("formatVersion" in f_geom["metadata"]) // then version 3.1
        {
          f_indices = f_geom["indices"];
          f_posns = f_geom["vertices"];
          f_norms = f_geom["normals"];
          f_uvs = f_geom["uvs"];
          f_faces = f_geom["faces"];
        } else if (f_geom["type"] === "Geometry") // V4 clara io output? not standard???
        {
          f_indices = f_geom["data"]["indices"];
          f_posns = f_geom["data"]["vertices"];
          f_norms = f_geom["data"]["normals"];
          f_uvs = f_geom["data"]["uvs"];
          f_faces = f_geom["data"]["faces"];
        } else if (f_geom["metadata"]["type"] === "Geometry") // V4
        {
          f_indices = f_geom["indices"];
          f_posns = f_geom["vertices"];
          f_norms = f_geom["normals"];
          f_uvs = f_geom["uvs"];
          f_faces = f_geom["faces"];
        } else if (f_geom["metadata"]["type"] === "BufferGeometry") // V4
        {
          f_posns = f_geom["data"]["attributes"]["position"]["array"];
          f_norms = f_geom["data"]["attributes"]["norms"]["array"];
          f_uvs = f_geom["data"]["attributes"]["uv"]["array"];
        }

        var nUvLayers = 0;

        // disregard empty arrays
        for (var i = 0; i < this.fileObject.uvs.length; i++) {
          if (this.fileObject.uvs[i].length)
            nUvLayers++;
        }

        var verts = new dali.VectorVertex();
        var vert = []; //new dali.Vertex();
        for (var i = 0, len = f_posns.length / 3; i < len; i++) {
          vert.push(f_posns[(i * 3) + 0]);
          vert.push(f_posns[(i * 3) + 1]);
          vert.push(f_posns[(i * 3) + 2]);

          vert.push(0); // norm
          vert.push(0);
          vert.push(0);

          vert.push(0); // uvs
          vert.push(0);

          verts.push_back(vert);
        }

        var mesh = new dali.MeshDataWrapper();
        var faces = new dali.VectorFaceIndex();
        var faceSets = {};
        //faceSets.length = this.fileObject.materials;
        for (var i = 0, len = this.fileObject.materials.length; i < len; ++i) {
          // get materials and force them to be loaded up
          var mat = this.__getMaterial(this.fileObject.materials[i]["uuid"]);
        }

        var idx = 0;
        var idx_len = f_faces.length;
        var materialUUID = undefined;
        while (idx < idx_len) {
          var f_type = f_faces[idx++];
          var isQuad = __isBitSet(f_type, 0);
          var hasMaterial = __isBitSet(f_type, 1);
          var hasFaceUv = __isBitSet(f_type, 2);
          var hasFaceVertexUv = __isBitSet(f_type, 3);
          var hasFaceNormal = __isBitSet(f_type, 4);
          var hasFaceVertexNormal = __isBitSet(f_type, 5);
          var hasFaceColor = __isBitSet(f_type, 6);
          var hasFaceVertexColor = __isBitSet(f_type, 7);

          var nVertices = 3;
          var faceVertexIndices;
          if (isQuad) {
            faces.push_back(f_faces[idx]);
            faces.push_back(f_faces[idx + 1]);
            faces.push_back(f_faces[idx + 2]);

            faces.push_back(f_faces[idx]);
            faces.push_back(f_faces[idx + 2]);
            faces.push_back(f_faces[idx + 3]);

            faceVertexIndices = [f_faces[idx],
                                 f_faces[idx + 1],
                                 f_faces[idx + 2]
                                ];

            idx += 4;
            nVertices = 4;
          } else {
            faces.push_back(f_faces[idx]);
            faces.push_back(f_faces[idx + 1]);
            faces.push_back(f_faces[idx + 2]);

            faceVertexIndices = [f_faces[idx],
                                 f_faces[idx + 1],
                                 f_faces[idx + 2]
                                ];

            idx += 3;
          }

          if (hasMaterial) {
            if (materialUUID === undefined) {
              materialUUID = this.fileObject.materials[f_faces[idx]]["uuid"];
            } else {
              // different material per face is bonkers - I'm not going to support it.
              if (this.fileObject.materials[f_faces[idx]]["uuid"] !== materialUUID) {
                throw "Faces with different materials is not supported";
              }
            }
            idx++;
          }


          if (hasFaceUv) {
            for (var i = 0; i < nUvLayers; i++) {
              var uvLayer = self.fileObject.uvs[i];
              var uvIndex = f_faces[idx++];
              var u = uvLayer[uvIndex * 2];
              var v = uvLayer[uvIndex * 2 + 1];
              // discarded - tbd ?
            }
          }

          if (hasFaceVertexUv) {
            for (var i = 0; i < nUvLayers; i++) {
              var uvLayer = f_geom.uvs[i];
              var uvs = [];
              for (var j = 0; j < nVertices; j++) {
                var uvIndex = f_faces[idx++];
                var u = uvLayer[uvIndex * 2];
                var v = uvLayer[uvIndex * 2 + 1];
                // discarded- tbd ?
              }
            }
          }

          if (hasFaceNormal) {
            var normalIndex = f_faces[idx++] * 3;

            var x = f_geom.normals[normalIndex++];
            var y = f_geom.normals[normalIndex++];
            var z = f_geom.normals[normalIndex];

            for (var i = 0; i < faceVertexIndices.length; i++) {
              var v = vert.get(faceVertexIndices[i]);

              v[4] += x;
              v[5] += y;
              v[6] += z;
            }
          }

          if (hasFaceVertexNormal) {
            for (var i = 0; i < nVertices; i++) {
              var normalIndex = faces[idx] * 3;
              var x = f_geom.normals[normalIndex++];
              var y = f_geom.normals[normalIndex++];
              var z = f_geom.normals[normalIndex];

              var v = vert.get(faces[idx]);

              v[4] += x;
              v[5] += y;
              v[6] += z;

              idx += 1;
              // face.vertexNormals.push( normal );
            }
          }

          if (hasFaceColor) {
            var color = f_faces[idx++];
          }

          if (hasFaceVertexColor) {
            for (var i = 0; i < nVertices; i++) {
              var colorIndex = faces[idx++];
              var color = f_geom.colors[colorIndex]; // ??? f_geom.colors?
              // face.vertexColors.push( color );
            }
          }

          var faces = null;
          if (f_faces) {
            for (var i = 1, len = f_faces.length; i < len; i++) {
              faces.push_back(f_faces[i]);
            }
          }

          if (f_indices) {
            faces = new dali.VectorFaceIndex();
            for (var i = 1, len = f_indices.length; i < len; i++) {
              faces.push_back(f_indices[i]);
            }
          }

          if (!faces) {
            faces = [];
            for (var i = 0, len = f_posns.length; i < len; i++) {
              faces.push(i);
            }
          }

          console.log(verts.size() + ":" + faces.size() + ":" + uuid_material);

          var material = this.__getMaterial(uuid_material);
          mesh.setLineData(verts, faces, material);
        }

        this.meshByUUID[uuid] = mesh;
        verts.delete();
        faces.delete();
        break;
      } // if uuid found
    } // for geom in geometries
  } // if uid ! in meshByUUID

  return this.meshByUUID[uuid];
};

ObjectLoader.prototype.delete = function() {
  "use strict";
  for (var a in this.meshByUUID) {
    a.delete();
  }
  this.meshByUUID = {};
  for (var b in this.matByUUID) {
    b.delete();
  }
  this.matByUUID = {};
};

ObjectLoader.prototype.createMeshActors = function() {
  "use strict";
  var ret = [];
  if ("object" in this.fileObject) {
    for (var i = 0, len = this.fileObject["object"]["children"].length; i < len; i++) {
      var child = this.fileObject["children"];
      if (child["type"] === "Mesh") {
        var meshData = this.__getMeshData(child["geometry"],
                                          child["material"]);
        ret.push(dali.__createMeshActor(meshData));
        meshData.delete();
      }
    }
  }

  var parent;

  if (ret) {
    parent = new dali.Actor();
    for (var a in ret) {
      parent.add(a);
      a.delete();
    }
  }

  return parent;
};

dali.createMeshActor = function(threeDjs_formatV4) {
  "use strict";
  var loader = new ObjectLoader(threeDjs_formatV4);
  return loader.createMeshActor();
};



//------------------------------------------------------------------------------
//
// Hit test
//
//------------------------------------------------------------------------------
dali.hitTest = function(x, y) {
  "use strict";
  var a = dali.__hitTest(x, y);
  if (a.ok()) {
    dali.internalSetupProperties(a);
    return a;
  } else {
    return null;
  }
};


//------------------------------------------------------------------------------
//
// Shader support
//
//------------------------------------------------------------------------------

/**
 * ShaderInfo class to get shader metadata.
 */
dali.ShaderInfo = function() {
  "use strict";
};

// supported uniforms
dali.ShaderInfo.prototype._supportedUniformTypes = ["bool",
                                                    "int",
                                                    "float",
                                                    "vec2", "vec3", "vec4",
                                                    "bvec2", "bvec3", "bvec4",
                                                    "ivec2", "ivec3", "ivec4",
                                                    "mat2", "mat3", "mat4",
                                                    "sampler2D",
                                                    "samplerCube"
                                                   ];

// need to add a value to uniform registration call
dali.ShaderInfo.prototype._supportedUniformValues = [0,
                                                     0,
                                                     0.0,
                                                     [0.0, 0.0], [0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0],
                                                     [0, 0], [0, 0, 0], [0, 0, 0, 0],
                                                     [0, 0], [0, 0, 0], [0, 0, 0, 0],
                                                     [1.0, 0.0,
                                                      0.0, 1.0
                                                     ],
                                                     [1.0, 0.0, 0.0,
                                                      0.0, 1.0, 0.0,
                                                      0.0, 0.0, 1.0
                                                     ],
                                                     [1.0, 0.0, 0.0, 0.0,
                                                      0.0, 1.0, 0.0, 0.0,
                                                      0.0, 0.0, 1.0, 0.0,
                                                      0.0, 0.0, 0.0, 1.0
                                                     ]
                                                    ];


/**
 * Get shader metadata from compilation.
 *
 * Compiles the shader. On error set 'hasError' and error strings. On Success
 * query gl for the attributes and uniforms in the shaders.
 *
 * @param {object} gl ie from canvas.getContext("webgl")
 * @param {string} vertex shader
 * @param {string} fragment shader
 * @return {Object} shader metadata (see 'var info' below)
 */
dali.ShaderInfo.prototype.fromCompilation = function(gl, vertex, fragment) {
  "use strict";
  var i;
  var info = {
    vertex: vertex,     // vertex source code
    fragment: fragment, // fragment source code
    attributes: {},     // {aName1: {name:"aName1", ... }
    uniforms: {},       // {uName1: {name:"uName1", type:"vec2", ...}
    uniformUISpec: {},  // {uName1: {ui:"slider", min:0, max:1, ...}
    attributeCount: 0,  // Number of attributes
    uniformCount: 0,    // Number of uniforms
    hasError: false,    // compiles without error
    vertexError: "",    // Vertex compilation error
    fragmentError: "",  // Fragment compilation error
    linkError: ""       // Linker error
  };

  var vertexShader = gl.createShader(gl.VERTEX_SHADER);
  gl.shaderSource(vertexShader, vertex);
  gl.compileShader(vertexShader);

  // Check the compile status, return an error if failed
  if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
    info.hasError = true;
    info.vertexError = gl.getShaderInfoLog(vertexShader);
  }

  var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
  gl.shaderSource(fragmentShader, fragment);
  gl.compileShader(fragmentShader);

  // Check the compile status, return an error if failed
  if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
    info.hasError = true;
    info.fragmentError = gl.getShaderInfoLog(fragmentShader);
  }

  if(info.hasError) {
    gl.deleteShader(vertexShader);
    gl.deleteShader(fragmentShader);
    return info; // ==> out
  } else {
    var program = gl.createProgram();
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);

    gl.linkProgram(program);

    if(!gl.getProgramParameter(program, gl.LINK_STATUS)) {
      info.hasError = true;
      info.linkError = gl.getProgramInfoLog(program);
      gl.deleteProgram(program);
      gl.deleteShader(vertexShader);
      gl.deleteShader(fragmentShader);
      return info; // ==> out
    }
  }

  var activeUniforms = gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS);
  var activeAttributes = gl.getProgramParameter(program, gl.ACTIVE_ATTRIBUTES);

  // Taken from the WebGl spec:
  // http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.14
  var enums = {
    0x8B50: "FLOAT_VEC2",
    0x8B51: "FLOAT_VEC3",
    0x8B52: "FLOAT_VEC4",
    0x8B53: "INT_VEC2",
    0x8B54: "INT_VEC3",
    0x8B55: "INT_VEC4",
    0x8B56: "BOOL",
    0x8B57: "BOOL_VEC2",
    0x8B58: "BOOL_VEC3",
    0x8B59: "BOOL_VEC4",
    0x8B5A: "FLOAT_MAT2",
    0x8B5B: "FLOAT_MAT3",
    0x8B5C: "FLOAT_MAT4",
    0x8B5E: "SAMPLER_2D",
    0x8B60: "SAMPLER_CUBE",
    0x1400: "BYTE",
    0x1401: "UNSIGNED_BYTE",
    0x1402: "SHORT",
    0x1403: "UNSIGNED_SHORT",
    0x1404: "INT",
    0x1405: "UNSIGNED_INT",
    0x1406: "FLOAT"
  };

  // Loop through active uniforms
  for (i = 0; i < activeUniforms; i++) {
    var uniform = gl.getActiveUniform(program, i);
    info.uniforms[uniform.name] = {name: uniform.name,
                                   type: uniform.type,
                                   typeName: enums[uniform.type],
                                   size: uniform.size};
    info.uniformCount += uniform.size;
  }

  // Loop through active attributes
  for (i = 0; i < activeAttributes; i++) {
    var attribute = gl.getActiveAttrib(program, i);
    info.attributes[attribute.name] = {name: attribute.name,
                                       type: attribute.type,
                                       typeName: enums[attribute.type],
                                       size: attribute.size};
    info.attributeCount += attribute.size;
  }

  // uniformUISpec
  this._addUniformMetaData(vertex, info);
  this._addUniformMetaData(fragment, info);

  return info;
};

/*
 * add unform metadata from shader source comments
 *  ie return as an object the comment following a uniform
 *     uniform float uAlpha; // {"min":0, "max":1}
 */
/** private */
dali.ShaderInfo.prototype._addUniformMetaData = function(src, metadata) {
  "use strict";
  // Loop through active uniforms
  for(var name in metadata.uniforms) {
    var reguniform = new RegExp(name + "[^;]*;(.*)");

    var tmp = reguniform.exec(src);
    if(tmp && tmp[1]) {
      var meta;
      var uComments = tmp[1].trim();
      if(uComments.startsWith("//")) { // meta data in comments
        try {
          meta = eval("(" + uComments.substr(2) + ")"); // brackets to be expression not opening statement
          if(typeof meta !== typeof ({})) {
            throw ("Uniform UI Spec in comments must be an object");
          }
        } catch (e) {
          meta = {};
        }
      } else {
        meta = {};
      }
      metadata.uniformUISpec[name] = meta;
    }
  }
};

/**
 * Get shader metadata from regex search.
 *
 * Attempts a regex search to get the shader meta data.
 * Use fromCompilation() instead of this function wherever compilation is
 * possible as this approach will never work for all shaders.
 * Does no compilation or error checking but retains fields for
 * compatibility with .fromCompilation(...)
 * May return an error if the regex fails.
 *
 * @param {string} vertex shader
 * @param {string} fragment shader
 * @return  {Object} shader metadata (see 'var info' below)
 */
dali.ShaderInfo.prototype.fromRegEx = function(vertex, fragment) {
  "use strict";
  var info = {          // similar to this.fromCompilation()
    vertex: vertex,     // source code
    fragment: fragment,
    attributes: {},     // {aName1: {name:"aName1", ... }
    uniforms: {},       // {uName1: {name:"uName1", type:"vec2", ...}
    attributeCount: 0,
    uniformCount: 0,
    uniformUISpec: {},  // {uName1: {ui:"slider", min:0, max:1, ...}
    hasError: false,    // compiles without error
    vertexError: "",
    fragmentError: "",
    linkError: ""
  };

  var metaVertex;
  try {
    metaVertex = this._getRegExMetaData(vertex);
  } catch(e) {
    info.hasError = true;
    info.vertexError = e.message;
    return info;
  }

  var metaFragment;
  try {
    metaFragment = this._getRegExMetaData(fragment);
  } catch(e) {
    info.hasError = true;
    info.fragmentError = e.message;
    return info;
  }

  var name;

  // merge
  info.uniforms = metaVertex.uniformMetaData;
  info.uniformUISpec = metaVertex.uniformUISpec;

  for(name in metaFragment.uniformMetaData) {
    if( name in info.uniforms ) {
      info.uniforms[name] = dali.mergeObjects(info.uniforms[name], metaVertex.uniformMetaData);
    } else {
      info.uniforms[name] = metaFragment.uniformMetaData[name];
    }
    if( name in info.uniformUISpec ) {
      info.uniformUISpec[name] = dali.mergeObjects(info.uniformUISpec[name], metaVertex.uniformUISpec);
    } else {
      info.uniformUISpec[name] = metaFragment.uniformUISpec[name];
    }
  }

  info.attributes = metaVertex.attributeMetaData;
  for(name in metaFragment.attributeMetaData) {
    if( name in metaVertex.attributeMetaData ) {
      info.attributes[name] = dali.mergeObjects(info.attributes[name], metaVertex.attributeMetaData);
    } else {
      info.attributes[name] = metaFragment.attributeMetaData[name];
    }
  }

  return info;
};

/*
 * Returns a string with all comments removed
 */
/** private */
dali.ShaderInfo.prototype._removeComments = function(str) {
  "use strict";
  var uid = "_" + new Date(),
      primatives = [],
      primIndex = 0;

  return (
    str
    /* Remove strings */
      .replace(/(['"])(\\\1|.)+?\1/g, function(match){
        primatives[primIndex] = match;
        return (uid + "") + primIndex++;
      })

    /* Remove Regexes */
      .replace(/([^\/])(\/(?!\*|\/)(\\\/|.)+?\/[gim]{0,3})/g, function(match, $1, $2){
        primatives[primIndex] = $2;
        return $1 + (uid + "") + primIndex++;
      })

    /*
     - Remove single-line comments that contain would-be multi-line delimiters
     E.g. // Comment /* <--
     - Remove multi-line comments that contain would be single-line delimiters
     E.g. /* // <--
     */
      .replace(/\/\/.*?\/?\*.+?(?=\n|\r|$)|\/\*[\s\S]*?\/\/[\s\S]*?\*\//g, "")

    /*
     Remove single and multi-line comments,
     no consideration of inner-contents
     */
      .replace(/\/\/.+?(?=\n|\r|$)|\/\*[\s\S]+?\*\//g, "")

    /*
     Remove multi-line comments that have a replace ending (string/regex)
     Greedy, so no inner strings/regexes will stop it.
     */
      .replace(RegExp("\\/\\*[\\s\\S]+" + uid + "\\d+", "g"), "")

    /* Bring back strings & regexes */
      .replace(RegExp(uid + "(\\d+)", "g"), function(match, n){
        return primatives[n];
      })
  );
};

/*
 * Returns true if value is in the array
 */
/** private */
dali.ShaderInfo.prototype._contains = function(array, value) {
  "use strict";
  for(var i = 0; i < array.length; i++) {
    if(array[i] === value) {
      return true;
    }
  }
  // else
  return false;
};

/*
 * Get the src meta data for unforms and attributes armed only with a regexp
 */
/** private */
dali.ShaderInfo.prototype._getRegExMetaData = function(src) {
  "use strict";
  var ret = {"uniforms": [],         // ["uName1", ["uName2"]
             "uniformMetaData": {},  // {uName1: {type:"vec3,...}
             "uniformUISpec": {},    // {ui:"slider", min:..., max:...}
             "attributes": [],       // ["aName2"]
             "attributeMetaData": {} // ["aName2"]
            };

  // Undoubtedly this approach will be wrong. Hopefully on not too many corner cases...
  // A better way is to compile the source see (fromCompilation())
  // but that requres a gl context.
  var tmp;

  var definesOut = /#define[ \t]+([A-Za-z_0-9]*)[ \t]+(.*)/g;

  var reg = /[ \t]?uniform[ ]*((?:lowp|mediump|highp)?[ \t]*(bool|int|uint|float|[biu]?vec[234]|mat[234]x?[234]?|[^ ]*)[ \t]*([A-Za-z0-9]*))[ \t]*(;|\[.*\][ \t]*;)(.*)/gi;

  var regAttrib = /[ \t]?attribute[ ]*((?:lowp|mediump|highp)?[ \t]*(bool|int|uint|float|[biu]?vec[234]|mat[234]x?[234]?|[^ ]*)[ \t]*([A-Za-z0-9]*))[ \t]*(;|\[.*\][ \t]*;)(.*)/gi;

  // 1. no commented out uniforms
  var noCommentSource = this._removeComments(src);

  var validUniforms = [];
  while ((tmp = reg.exec(noCommentSource))) {
    validUniforms.push( tmp[3] );
  }

  while ((tmp = regAttrib.exec(noCommentSource))) {
    ret.attributes.push( tmp[3] );
    ret.attributeMetaData[ tmp[3] ] = {name: tmp[3], type: tmp[2] };
  }

  // 2. replace defines
  var defines = [];
  while ((tmp = definesOut.exec(noCommentSource))) {
    defines.push([tmp[1], tmp[2]]);
  }
  var defineDict = {};
  var defineList = [];
  while(defines.length) {
    var p = defines.pop();
    var n = p[0];
    var v = p[1];
    try {
      defineDict[n] = eval(v);
      defineList.push([n, defineDict[n]]);
    } catch(e) {
      var d = /([A-Za-z]+[A-Za-z0-9]*)/g;
      while ((tmp = d.exec(v))) {
        if(tmp[0] in defineDict) {
          v = v.replace(tmp[0], defineDict[tmp[0]]);
        } else {
          defines.push(p); // stick it back to try again. ...and endless loop if we can't(!)
        }
      }
    }
  }

  for(var i = 0; i < defineList.length; i++) {
    var re = new RegExp(defineList[i][0], "g");
    src = src.replace(re, defineList[i][1]);
  }

  // 3. get uniforms
  while ((tmp = reg.exec(src))) {
    if(!this._contains(validUniforms, tmp[3])) {
      continue;
    }
    var uType = tmp[2];
    var uName = tmp[3];
    var uArray = tmp[4].slice(0, -1);
    var uComments = tmp[5].trim();
    var meta;
    var uiSpecMeta = null;
    if(uComments.startsWith("//")) { // meta data in comments
      uiSpecMeta = eval("(" + uComments.substr(2) + ")"); // brackets to be expression not opening statement
      if(typeof uiSpecMeta !== typeof ({})) {
        throw ("Uniform UI Spec in comments must be an object");
      }
    }

    if(uiSpecMeta) {
      uiSpecMeta.name = tmp[3];
      ret.uniformUISpec[uName] = uiSpecMeta;
    }

    meta = {};
    meta.type = tmp[2];
    meta.name = tmp[3];
    meta.count = 0;

    var name;
    if(uArray.search("[[]") >= 0) { // an array
      meta.count = Number(uArray.slice(1, -1));
    }

    if(this._contains( this._supportedUniformTypes, uType) ) {
      if(meta.count !== 0) { // array
        for(var j = 0; j < meta.count; j++) {
          ret.uniforms.push( meta.name );
          ret.uniformMetaData[ meta.name ] = {type: meta.type,
                                              name: meta.name + "[" + j + "]",
                                              index: j,
                                              count: meta.count};
        }
      } else {
        ret.uniforms.push( meta.name );
        ret.uniformMetaData[ meta.name ] = {type: meta.type,
                                            name: meta.name,
                                            index: 0,
                                            count: 0};
      }
    } else {
      // not a base type so need to get the compound type
      var structFind = new RegExp( "(struct[ \t\n]*" + uType + "[^{]*{)([^}]*)", "g");
      var structLines = structFind.exec(src)[2].split(";");
      var structUniforms = [];
      var tmpStruct;
      var k;
      for(var lineNo = 0; lineNo < structLines.length; lineNo++) {
        var line = structLines[lineNo].replace(/\n/g, "") + ";";
        if(line !== ";") {
          var structReg = /[ \t\n]*((?:lowp|mediump|highp)?[ \t\n]*(bool|int|uint|float|[biu]?vec[234]|mat[234]x?[234]?|[^ ]*)[ \t\n]*([A-Za-z0-9]*))[ \t\n]*(;|\[.*\][ \t\n]*;)/gi;
          while ((tmpStruct = structReg.exec(line))) {
            structUniforms.push( { type: tmpStruct[2],
                                   name: tmpStruct[3],
                                   count: meta.count } );
          }
        }
      }
      if(meta.count === 0) {
        for(k = 0; k < structUniforms.length; k++) {
          name = uName + "." + structUniforms[k].name;
          ret.uniforms.push( name );
          ret.uniformMetaData[ name ] = {type: structUniforms[k].type,
                                         name: name,
                                         count: meta.count,
                                         index: 0,
                                         structType: meta.type,
                                         structName: meta.name};
        }
      } else { // array
        for(var l = 0; l < meta.count; l++) {
          for(k = 0; k < structUniforms.length; k++) {
            name = uName + "[" + l + "]" + "." + structUniforms[k].name;
            ret.uniforms.push( name );
            ret.uniformMetaData[ name ] = {type: structUniforms[k].type,
                                           name: name,
                                           count: meta.count,
                                           index: l,
                                           structType: meta.type,
                                           structName: meta.name};
          }
        }
      }
    }
  }

  return ret;
};

//------------------------------------------------------------------------------
//
// Debug Module
//
//------------------------------------------------------------------------------
dali.Debug = function() {
  "use strict";
};

dali.Debug.prototype.printTypeProperties = function(typeName) {
  "use strict";
  var t = new dali.TypeRegistry();
  var info = t.getTypeInfo(typeName);
  var props = info.getProperties();
  for (var i = 0; i < props.size(); i++) {
    console.log(i + ":" + props.get(i));
  }
  info.delete(); // wrapper
  t.delete(); // wrapper
};

dali.Debug.prototype.printProperties = function(o) {
  "use strict";
  var props = o.getProperties();

  var len = props.size();
  for(var i = 0; i < len; i++) {
    var name = props.get(i);
    var type = o.getPropertyTypeName(name);
    if(type !== "NONE") {
      console.log(i + ":" + name + " " + type);
    } else {
      type = o.getPropertyTypeName(name);
      console.log(i + ":" + name + " " + type + " (Not mangled)");
    }
  }
  props.delete(); // wrapper
};

dali.Debug.prototype.printTypes = function() {
  "use strict";

  var t = new dali.TypeRegistry();
  for (var i = 0; i < t.getTypeNameCount(); i++) {
    console.log(t.getTypeName(i));
  }
  t.delete(); // wrapper
};


dali._debugPrintParents = function(actor, list) {
  "use strict";
  var p = null;

  if (!actor.ok()) {
    return;
  }

  try {
    p = actor.getParent();
    if (!p.ok()){
      p = null;
    }
  } catch (e) {
    // console.log("Cannot get parent", e);
  }

  if (p) {
    list.push(p);
    dali._debugPrintParents(p, list);
  }
};

dali.Debug.prototype.printTree = function(actor) {
  "use strict";
  var l = [];
  dali._debugPrintParents(actor, l);
  var a;
  var ti;
  console.log("---");
  for (var i = l.length - 1; i >= 0; i--) {
    a = l[i];
    ti = a.getTypeInfo();
    console.log("|", Array(l.length - i).join("-"), ti.getName(), "P", a.position, "R", a.orientation, a.name);
    ti.delete();
  }
  ti = actor.getTypeInfo();
  console.log("*", Array(l.length + 1).join("*"), ti.getName(), "P", actor.position, "R", actor.orientation, actor.name);
  ti.delete();

  var children = actor.getChildren();
  for (var j = 0; j < children.length; j++) {
    a = children[j];
    ti = a.getTypeInfo();
    console.log("|", Array(l.length + 1 + 1 + j).join("-"), ti.getName(), "P", a.position, "R", a.orientation, a.name);
    ti.delete();
  }
};

dali.Debug.prototype.printRenderTask = function(rendertask) {
  "use strict";
  console.log("[X,Y]", rendertask.getCurrentViewportPosition());
  console.log("[W,H]", rendertask.getCurrentViewportSize());

  var c = rendertask.getCameraActor();
  if (!c.ok()) {
    console.log("No Camera");
  } else {
    console.log("Camera Pos:", c.position);
    console.log("Camera Rot:", c.orientation);
    console.log("Camera Inherit:", c.inheritRotation);
    console.log("Camera ParentOrigin:", c.parentOrigin);
    console.log("Camera AnchorPoint:", c.anchorPoint);
    var p = null;
    try {
      p = c.getParent();
      if(!p.ok()) {
        p = null;
      }
    } catch (e) {
      console.log("Cannot get parent", e);
    }

    if (!p) {
      console.log("Camera has no parent?");
    } else {
      var ti = p.getTypeInfo();
      console.log("Parent Name", ti.getName());
      ti.delete();
      p.delete();
    }
  }
};

dali.Debug.prototype.printRenderTasks = function() {
  "use strict";
  var stage = dali.stage;
  var taskList = stage.getRenderTaskList();
  for (var i = 0; i < taskList.getTaskCount(); i++) {
    var t = taskList.getTask(i);
    console.log("RenderTask:", i);
    this.printRenderTask(t);
    t.delete(); // wrapper
  }
  taskList.delete(); // wrapper
};

dali.Debug.prototype.findFirstActor = function(actor, predicateFunction) {
  "use strict";
  for (var i = 0, len = actor.getChildCount(); i < len; i++) {
    var a = actor.getChildAt(i);
    var found = predicateFunction(a);
    if (found) {
      return a;
    }
    var child = this.findFirstActor(a, predicateFunction);
    if (child) {
      return child;
    }
    a.delete();
  }
  return null;
};

dali.Debug.prototype.depthVisit = function(actor, operation, dontDelete) {
  "use strict";
  for (var i = 0, len = actor.getChildCount(); i < len; i++) {
    var a = actor.getChildAt(i);
    var done = operation(a);
    if (!done) {
      return false;
    }
    if (!this.depthVisit(a, operation, dontDelete)) {
      return false;
    }
    var doit = true;
    if (dontDelete !== undefined) {
      if (dontDelete) {
        doit = false;
      }
    }
    if (doit) {
      a.delete();
    }
  }
  return true;
};

dali.operationPrintProperty = function(property, all) {
  "use strict";
  return (function(actor) {
    if (property in actor) {
      dali.log(actor.getId() + "property:" + actor[property]);
    } else {
      dali.log(actor.getId() + "property:n/a");
    }
    return all;
  });
};

dali.predicatePropertyEquals = function(property, value) {
  "use strict";
  return (function(actor) {
    if (property in actor) {
      if (actor[property] === value) {
        return true;
      }
    }
    return false;
  });
};

dali.typeInheritsFrom = function(type, basename) {
  var inherits = false;

  var registry = new dali.TypeRegistry();

  var base = registry.getTypeInfo( type.getBaseName() );

  if(base.ok())
  {
    inherits = (base.getName() === basename);

    while(!inherits)
    {
      base = registry.getTypeInfo( base.getBaseName() );
      if(base.ok())
      {
        inherits = (base.getName() === basename);
      }
      else
      {
        break;
      }
    }
  }

  return inherits;
};



//------------------------------------------------------------------------------
//
// View Module
//
// Helper functions for creating front/top/left views with RenderTasks
//
//------------------------------------------------------------------------------

/**
 * Sets the clear colour in a RenderTask
 * @method setClearColor
 * @param {int} renderTaskIndex
 * @param {array} color The rgba colour array
 */
dali.setClearColor = function(renderTaskIndex, color) {
  "use strict";
  var stage = dali.stage;
  var taskList = stage.getRenderTaskList();
  if (renderTaskIndex >= taskList.getTaskCount()) {
    console.log("RenderTaskIndex out of bounds:", renderTaskIndex);
    taskList.delete(); // wrapper
    return;
  }
  var rendertask = taskList.getTask(renderTaskIndex);
  rendertask.setClearEnabled(true);
  rendertask.setClearColor(color);
};

/**
 * Gets the clear colour of a RenderTask
 * @method setClearColor
 * @param {int} renderTaskIndex
 * @return {array} The rgba colour array
 */
dali.getClearColor = function(renderTaskIndex) {
  "use strict";
  var stage = dali.stage;
  var taskList = stage.getRenderTaskList();
  if (renderTaskIndex >= taskList.getTaskCount()) {
    console.log("RenderTaskIndex out of bounds:", renderTaskIndex);
    taskList.delete(); // wrapper
    return null;
  }
  var rendertask = taskList.getTask(renderTaskIndex);
  return rendertask.getClearColor();
};

/**
 * Set a front view camera with viewport x,y,w,h
 * @method setFrontView
 * @param {int} renderTaskIndex
 * @param {int} x Viewport X
 * @param {int} y Viewport Y
 * @param {int} w Viewport W
 * @param {int} h Viewport H
 */
dali.setFrontView = function(renderTaskIndex, x, y, w, h) {
  "use strict";
  var stage = dali.stage;
  var taskList = stage.getRenderTaskList();
  if (renderTaskIndex >= taskList.getTaskCount()) {
    console.log("RenderTaskIndex out of bounds:", renderTaskIndex);
    taskList.delete(); // wrapper
    return;
  }
  var rendertask = taskList.getTask(renderTaskIndex);

  var c = rendertask.getCameraActor();
  assert(c.ok(), "Rendertask has no valid camera actor");

  rendertask.setViewportPosition([x, y]);
  rendertask.setViewportSize([w, h]);
  c.position = [0, 0, 800];
  c.orientation = [0, 1, 0, 180];
  c.aspectRatio = w / h;

  c.delete(); // wrapper
  rendertask.delete(); // wrapper
  taskList.delete(); // wrapper
};

/**
 * Set a top view camera with viewport x,y,w,h
 * @method setTopView
 * @param {int} renderTaskIndex
 * @param {int} x Viewport X
 * @param {int} y Viewport Y
 * @param {int} w Viewport W
 * @param {int} h Viewport H
 */
dali.setTopView = function(renderTaskIndex, x, y, w, h) {
  "use strict";
  var stage = dali.stage;
  var taskList = stage.getRenderTaskList();
  if (renderTaskIndex >= taskList.getTaskCount()) {
    console.log("RenderTaskIndex out of bounds:", renderTaskIndex);
    taskList.delete(); // wrapper
    return;
  }
  var rendertask = taskList.getTask(renderTaskIndex);

  var c = rendertask.getCameraActor();
  assert(c.ok(), "Rendertask has no valid camera actor");

  rendertask.setViewportPosition([x, y]);
  rendertask.setViewportSize([w, h]);

  var q1 = dali.axisAngleToQuaternion([0, 1, 0, dali.radian(180)]); // yaw around to look at scene down -ve z
  var q2 = dali.axisAngleToQuaternion([1, 0, 0, dali.radian(-90)]); // pitch to look at scene
  var q = dali.quaternionToAxisAngle(dali.quatByQuat(q1, q2));

  c.position = [0, -800, 0]; // @todo; get 800 from dali not hard coded here
  c.orientation = [q[0], q[1], q[2], dali.degree(q[3])]; // @todo; should really all be in radians
  c.aspectRatio = w / h;

  c.delete(); // wrapper
  rendertask.delete(); // wrapper
  taskList.delete(); // wrapper
};

/**
 * Set a right view camera with viewport x,y,w,h
 * @method setRightView
 * @param {int} renderTaskIndex
 * @param {int} x Viewport X
 * @param {int} y Viewport Y
 * @param {int} w Viewport W
 * @param {int} h Viewport H
 */
dali.setRightView = function(renderTaskIndex, x, y, w, h) {
  "use strict";
  var stage = dali.stage;
  var taskList = stage.getRenderTaskList();
  if (renderTaskIndex >= taskList.getTaskCount()) {
    console.log("RenderTaskIndex out of bounds:", renderTaskIndex);
    taskList.delete(); // wrapper
    return;
  }
  var rendertask = taskList.getTask(renderTaskIndex);

  var c = rendertask.getCameraActor();
  assert(c.ok(), "Rendertask has no valid camera actor");

  rendertask.setViewportPosition([x, y]);
  rendertask.setViewportSize([w, h]);

  var q1 = dali.axisAngleToQuaternion([0, 1, 0, dali.radian(180)]); // yaw around to look at scene down -ve z
  var q2 = dali.axisAngleToQuaternion([0, 1, 0, dali.radian(90)]); // yaw again to look from right
  var q = dali.quaternionToAxisAngle(dali.quatByQuat(q1, q2));

  c.position = [800, 0, 0];
  c.orientation = [q[0], q[1], q[2], dali.degree(q[3])]; // @todo; should really all be in radians
  c.aspectRatio = w / h;

  c.delete(); // wrapper
  rendertask.delete(); // wrapper
  taskList.delete(); // wrapper
};

/**
 * Remove all but one render task. Presumes RenderTasks are being use only for viewing windows.
 * @method onePane
 */
dali.onePane = function() {
  "use strict";
  var stage = dali.stage;
  var taskList = stage.getRenderTaskList();
  var tasks = [];
  var i, len;

  for (i = 1, len = taskList.getTaskCount(); i < len; i++) {
    tasks.push(taskList.getTask(i));
  }

  for (i = 0, len = tasks.length; i < len; i++) {
    var task = tasks[i];
    // delete the camera actors we created in twoPane and threePane
    var c = task.getCameraActor();
    if (c.ok()) {
      var p = c.getParent();
      if (p.ok()) {
        p.remove(c);
      }
      p.delete(); // wrapper
    }
    c.delete(); // wrapper

    taskList.removeTask(task);
    task.delete(); // wrapper
  }

  taskList.delete();
};

/**
 * Creates render tasks and cameras for a two pane view.
 * Use setFrontView/Top/Right with 0-2 index to setup the actual views.
 * (in a separate function to allow window gutters)
 * @method twoPane
 */
dali.twoPane = function() {
  "use strict";
  dali.onePane();

  var stage = dali.stage;
  var taskList = stage.getRenderTaskList();

  var defaultTask = taskList.getTask(0);
  var defaultCamera = defaultTask.getCameraActor();
  var defaultCameraParent = defaultCamera.getParent();

  var t;
  t = taskList.createTask();

  var c = new dali.CameraActor(); // add camera for different viewpoint
  c.position = [0, 0, 800];
  c.orientation = [0, 1, 0, 180];
  c.parentOrigin = [0.5, 0.5, 0.5];
  c.anchorPoint = [0.5, 0.5, 0.5];
  t.setCameraActor(c);
  defaultCameraParent.add(c);
  c.delete(); // wrapper

  t.delete(); // wrapper

  defaultCameraParent.delete(); // wrapper
  defaultCamera.delete(); // wrapper
  defaultTask.delete(); // wrapper

  taskList.delete(); // wrapper
};

/**
 * Creates render tasks and cameras for a three pane view.
 * Use setFrontView/Top/Right with 0-2 index to setup the actual views.
 * (in a separate function to allow window gutters)
 * @method threePane
 */
dali.threePane = function() {
  "use strict";
  dali.onePane();

  var stage = dali.stage;
  var taskList = stage.getRenderTaskList();

  var defaultTask = taskList.getTask(0);
  var defaultCamera = defaultTask.getCameraActor();
  var defaultCameraParent = defaultCamera.getParent();

  var t;
  t = taskList.createTask();

  var c = new dali.CameraActor(); // add camera for different viewpoint
  c.position = [0, 0, 800];
  c.orientation = [0, 1, 0, 180];
  c.parentOrigin = [0.5, 0.5, 0.5];
  c.anchorPoint = [0.5, 0.5, 0.5];
  t.setCameraActor(c);
  defaultCameraParent.add(c);
  c.delete(); // wrapper

  t.delete(); // wrapper

  t = taskList.createTask();

  c = new dali.CameraActor(); // add camera for different viewpoint
  c.position = [0, 0, 800];
  c.orientation = [0, 1, 0, 180];
  c.parentOrigin = [0.5, 0.5, 0.5];
  c.anchorPoint = [0.5, 0.5, 0.5];
  t.setCameraActor(c);
  defaultCameraParent.add(c);
  c.delete(); // wrapper

  t.delete(); // wrapper

  defaultCameraParent.delete(); // wrapper
  defaultCamera.delete(); // wrapper
  defaultTask.delete(); // wrapper

  taskList.delete(); // wrapper
};

//------------------------------------------------------------------------------
//
// Dali Initialization Module
//
//------------------------------------------------------------------------------

/**
 * Create a Dali object by type name
 * @method create
 * @param {string} name The type name to create
 * @return A Dali handle to the Dali object
 */
dali.create = function(name) {
  "use strict";

  var handle = dali.__createActor(name);

  if (!handle.ok()) {
    handle.delete(); // handle
    handle = dali.__createHandle(name);
  }

  dali.internalSetupProperties(handle);

  return handle;
};

dali.updateFrame = function() {
  dali.__updateOnce();
  dali.__renderOnce();
};

/**
 * Creates constructors for objects found in the TypeRegistry. Some objects are
 * individually wrapped. Sets some global objects eg. debug/stage.
 */
/** private */
dali.init = function() {
  "use strict";

  console.log( dali.VersionString() );

  dali.jsSignalHolder = new dali.SignalHolder(); // for js callbacks

  dali.debug = new dali.Debug();

  dali.stage = new dali.Stage();

  dali.getStage = function() { // duplication of dali.stage to stop regressions
    return dali.stage;
  };

  //
  // Add constructor functions to dali from the type registry
  //
  // Uses separate create functions to add methods for the different base classes.
  // Other generic access is by properties. Currently
  //
  //              +------------+
  //              | BaseHandle |
  //              +------+-----+
  //                     |
  //      |--------------+------------|
  //      |              |            |
  // +----+------+  +----+---+   +----+--+
  // | Animation |  | Handle |   | Image |
  // +-----------+  +--------+   +-------+
  //

  var t = new dali.TypeRegistry();

  // use the emscripten wrapping for these and not the typeregisitry creation function
  var useWrapping = { RenderTask: 1, RenderTaskList: 1, CameraActor: 1,
                      TypeInfo: 1,
                      Path: 1, Animation: 1,
                      Handle: 1, Actor: 1,
                      PropertyMap: 1, PropertyBuffer: 1,
                      ShaderEffect: 1,
                      Image: 1, BufferImage: 1, EncodedBufferImage: 1,
                      Geometry: 1, Material: 1, Shader: 1, Sampler: 1, Renderer: 1
                    };

  for (var i = 0; i < t.getTypeNameCount(); i++) {
    // anon function because of closure with defineProperty
    // (if just variable in loop then the variable 'address' is captured, not the value
    //  so it becomes last value set)
    (function(name) {
      var createFunc;
      var info = t.getTypeInfo(name);

      if(dali.typeInheritsFrom(info, "Actor")) {
        createFunc = dali.__createActor;
      } else if(dali.typeInheritsFrom(info, "Handle")) {
        createFunc = dali.__createHandle;
      }

      // @todo Dali error?? name lengths should never be zero
      if (name.length && !(name in useWrapping) ) {
        Object.defineProperty(dali, name, {
          enumerable: true,
          configurable: false,
          get: function() {
            return function() {
              // console.log(name);
              return dali.create(name);
            };
          }
        });
      }
    })(t.getTypeName(i));
  }

  dali.updateFrame();

}(); // call init


//------------------------------------------------------------------------------
//
// Post run
//
// Call postDaliWrapperRun() to indicate dali-wrapper.js has loaded
// and other sequential tasks can run (js files can load async)
//
//------------------------------------------------------------------------------
if(Module)
{
  if (Module.postDaliWrapperRun) {
    Module.postDaliWrapperRun();
  }
}
