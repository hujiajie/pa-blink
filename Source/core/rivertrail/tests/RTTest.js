/*

var group_name_1 = ['group name',           // start of the first group
[                                           // start of the first test item in this group
'description',
function ()                                 // the test program
{
  // do something
  // return the result
},
'expected result',
'(Optional) If result is equal to this,
this test will be marked as unsupported'
],                                          // end of the first test item in this group
[                                           // start of the second test item in this group
'description',
function ()                                 // the test program
{
  // do something
  // return the result
},
'expected result',
'(Optional) If result is equal to this,
this test will be marked as unsupported'
],                                          // end of the second test item in this group
...                                         // other test items in this group
];                                          // end of the first group

...                                         // other groups

var tests = [group_name_1, group_name_2, ...];

*/

var group_mapPar = ['mapPar',
[
'The source is one dimensional, the elemental function has one parameter',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) { return val + 1; });
},
[2, 3, 4, 5]
],
[
'The source is one dimensional, the elemental function has two parameters',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val, idx) { return val + idx; });
},
[1, 3, 5, 7]
],
[
'The source is one dimensional, the elemental function has three parameters',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val, idx, src) { return val + src[idx]; });
},
[2, 4, 6, 8]
],
[
'The source is nested, the elemental function has one parameter',
function ()
{
  var dim1 = 2;
  var dim2 = 3;
  var dim3 = 4;
  var dim4 = 5;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = new Array(dim3);
      for (var k = 0; k < dim3; k++) {
        source[i][j][k] = new Array(dim4);
        for (var l = 0; l < dim4; l++) {
          source[i][j][k][l] = accu++;
        }
      }
    }
  }
  return source.mapPar(function (val) { return val[1][2][3]; });
},
[34, 94]
],
[
'The source is nested, the elemental function has two parameters',
function ()
{
  var dim1 = 2;
  var dim2 = 3;
  var dim3 = 4;
  var dim4 = 5;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = new Array(dim3);
      for (var k = 0; k < dim3; k++) {
        source[i][j][k] = new Array(dim4);
        for (var l = 0; l < dim4; l++) {
          source[i][j][k][l] = accu++;
        }
      }
    }
  }
  return source.mapPar(function (val, idx) { return val[1][2][3] + idx; });
},
[34, 95]
],
[
'The source is nested, the elemental function has three parameters',
function ()
{
  var dim1 = 2;
  var dim2 = 3;
  var dim3 = 4;
  var dim4 = 5;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = new Array(dim3);
      for (var k = 0; k < dim3; k++) {
        source[i][j][k] = new Array(dim4);
        for (var l = 0; l < dim4; l++) {
          source[i][j][k][l] = accu++;
        }
      }
    }
  }
  return source.mapPar(function (val, idx, src) { return src[idx][1][2][3]; });
},
[34, 94]
],
[
'The return value is a one-dimensional array',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) { return [val, 0]; });
},
(function ()
{
  return [1, 2, 3, 4].map(function (val) { return [val, 0]; });
})()
],
[
'The return value is a one-dimensional array from the source',
function ()
{
  var dim1 = 2;
  var dim2 = 3;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = accu++;
    }
  }
  return source.mapPar(function (val) { return val; });
},
(function ()
{
  var dim1 = 2;
  var dim2 = 3;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = accu++;
    }
  }
  return source.map(function (val) { return val; });
})()
],
[
'The return value is a nested array',
function ()
{
  return [1, 2, 3, 4, 5].mapPar(function (val) { return [[[val,  1], [ 2,  3], [ 4,  5]],
                                                         [[  6,  7], [ 8,  9], [10, 11]],
                                                         [[ 12, 13], [14, 15], [16, 17]],
                                                         [[ 18, 19], [20, 21], [22, 23]]]; });
},
(function ()
{
  return [1, 2, 3, 4, 5].map(function (val) { return [[[val,  1], [ 2,  3], [ 4,  5]],
                                                      [[  6,  7], [ 8,  9], [10, 11]],
                                                      [[ 12, 13], [14, 15], [16, 17]],
                                                      [[ 18, 19], [20, 21], [22, 23]]]; });
})()
],
[
'The return value is a nested array from the source',
function ()
{
  var dim1 = 2;
  var dim2 = 3;
  var dim3 = 4;
  var dim4 = 5;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = new Array(dim3);
      for (var k = 0; k < dim3; k++) {
        source[i][j][k] = new Array(dim4);
        for (var l = 0; l < dim4; l++) {
          source[i][j][k][l] = accu++;
        }
      }
    }
  }
  return source.mapPar(function (val) { return val; });
},
(function ()
{
  var dim1 = 2;
  var dim2 = 3;
  var dim3 = 4;
  var dim4 = 5;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = new Array(dim3);
      for (var k = 0; k < dim3; k++) {
        source[i][j][k] = new Array(dim4);
        for (var l = 0; l < dim4; l++) {
          source[i][j][k][l] = accu++;
        }
      }
    }
  }
  return source.map(function (val) { return val; });
})()
],
[
'The return value is an object with one-dimensional arrayish properties',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) { return {prop1 : val, prop2 : [val, 0]}; });
},
(function ()
{
  return [1, 2, 3, 4].map(function (val) { return {prop1 : val, prop2 : [val, 0]}; });
})()
],
[
'The return value is an object with one-dimensional arrayish properties from the source',
function ()
{
  var dim1 = 2;
  var dim2 = 3;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = accu++;
    }
  }
  return source.mapPar(function (val) { return {prop1 : val[1], prop2 : val}; });
},
(function ()
{
  var dim1 = 2;
  var dim2 = 3;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = accu++;
    }
  }
  return source.map(function (val) { return {prop1 : val[1], prop2 : val}; });
})()
],
[
'The return value is an object with nested arrayish properties',
function ()
{
  return [1, 2, 3, 4, 5].mapPar(function (val) { return {prop1 : val,
                                                         prop2 : [[[val,  1], [ 2,  3], [ 4,  5]],
                                                                  [[  6,  7], [ 8,  9], [10, 11]],
                                                                  [[ 12, 13], [14, 15], [16, 17]],
                                                                  [[ 18, 19], [20, 21], [22, 23]]]}; });
},
(function ()
{
  return [1, 2, 3, 4, 5].map(function (val) { return {prop1 : val,
                                                      prop2 : [[[val,  1], [ 2,  3], [ 4,  5]],
                                                               [[  6,  7], [ 8,  9], [10, 11]],
                                                               [[ 12, 13], [14, 15], [16, 17]],
                                                               [[ 18, 19], [20, 21], [22, 23]]]}; });
})()
],
[
'The return value is an object with nested arrayish properties from the source',
function ()
{
  var dim1 = 2;
  var dim2 = 3;
  var dim3 = 4;
  var dim4 = 5;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = new Array(dim3);
      for (var k = 0; k < dim3; k++) {
        source[i][j][k] = new Array(dim4);
        for (var l = 0; l < dim4; l++) {
          source[i][j][k][l] = accu++;
        }
      }
    }
  }
  return source.mapPar(function (val) { return {prop1 : val[1][2][3], prop2 : val}; });
},
(function ()
{
  var dim1 = 2;
  var dim2 = 3;
  var dim3 = 4;
  var dim4 = 5;

  var accu = 1;
  var source = new Array(dim1);
  for (var i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (var j = 0; j < dim2; j++) {
      source[i][j] = new Array(dim3);
      for (var k = 0; k < dim3; k++) {
        source[i][j][k] = new Array(dim4);
        for (var l = 0; l < dim4; l++) {
          source[i][j][k][l] = accu++;
        }
      }
    }
  }
  return source.map(function (val) { return {prop1 : val[1][2][3], prop2 : val}; });
})()
]
];

var tests = [group_mapPar];
