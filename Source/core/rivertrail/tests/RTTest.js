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
'An identity function (1-D source)',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) { return val; });
},
[1, 2, 3, 4]
],
[
'An identity function (2-D source)',
function ()
{
  var dim1 = 2;
  var dim2 = 3;
  var i, j;

  var accu = 1;
  var source = new Array(dim1);
  for (i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (j = 0; j < dim2; j++) {
      source[i][j] = accu++;
    }
  }
  return source.mapPar(function (val) { return val; });
},
[[1, 2, 3], [4, 5, 6]]
],
[
'An identity function (4-D source)',
function ()
{
  var dim1 = 2;
  var dim2 = 3;
  var dim3 = 4;
  var dim4 = 5;
  var i, j, k, l;

  var accu = 1;
  var source = new Array(dim1);
  for (i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (j = 0; j < dim2; j++) {
      source[i][j] = new Array(dim3);
      for (k = 0; k < dim3; k++) {
        source[i][j][k] = new Array(dim4);
        for (l = 0; l < dim4; l++) {
          source[i][j][k][l] = accu++;
        }
      }
    }
  }
  return source.mapPar(function (val) { return val; });
},
(function () {
  var dim1 = 2;
  var dim2 = 3;
  var dim3 = 4;
  var dim4 = 5;
  var i, j, k, l;

  var accu = 1;
  var source = new Array(dim1);
  for (i = 0; i < dim1; i++) {
    source[i] = new Array(dim2);
    for (j = 0; j < dim2; j++) {
      source[i][j] = new Array(dim3);
      for (k = 0; k < dim3; k++) {
        source[i][j][k] = new Array(dim4);
        for (l = 0; l < dim4; l++) {
          source[i][j][k][l] = accu++;
        }
      }
    }
  }
  return source;
})()
],
[
'Simple addition',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) { return val + 1; });
},
[2, 3, 4, 5]
],
[
'Read index',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val, idx) { return val * idx; });
},
[0, 2, 6, 12]
],
[
'Read source',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val, idx, src) { return src[3 - idx]; });
},
[4, 3, 2, 1]
],
[
'The elemental function returns an object with arrayish properties',
function ()
{
  return [1, 2].mapPar(function (val) { return {prop1 : val,
                                                prop2 : [val, val + 10, val + 20],
                                                prop3 : [[val +  100, val +  200, val +  300],
                                                         [val +  400, val +  500, val +  600],
                                                         [val +  700, val +  800, val +  900],
                                                         [val + 1000, val + 1100, val + 1200]]}; });
},
[{prop1 : 1, prop2 : [1, 11, 21], prop3 : [[101, 201, 301], [401, 501, 601], [701, 801, 901], [1001, 1101, 1201]]},
 {prop1 : 2, prop2 : [2, 12, 22], prop3 : [[102, 202, 302], [402, 502, 602], [702, 802, 902], [1002, 1102, 1202]]}]
],
[
'The elemental function returns an object with arrayish properties from the source',
function ()
{
  var source = [[[ 1,  2,  3,  4], [ 5,  6,  7,  8], [ 9, 10, 11, 12]],
                [[13, 14, 15, 16], [17, 18, 19, 20], [21, 22, 23, 24]]];
  return source.mapPar(function (val) { return {prop1 : val[1][2],
                                                prop2 : val[2],
                                                prop3 : val}; });
},
[{prop1 :  7, prop2 : [ 9, 10, 11, 12], prop3 : [[ 1,  2,  3,  4], [ 5,  6,  7,  8], [ 9, 10, 11, 12]]},
 {prop1 : 19, prop2 : [21, 22, 23, 24], prop3 : [[13, 14, 15, 16], [17, 18, 19, 20], [21, 22, 23, 24]]}]
]
];

var tests = [group_mapPar];
