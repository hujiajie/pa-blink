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
'1,2,3,4'
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

  var result = source.mapPar(function (val) { return val; });

  for (i = 0; i < dim1; i++) {
    for (j = 0; j < dim2; j++) {
      if (result[i][j] !== source[i][j]) {
        return false;
      }
    }
  }

  return true;
},
true
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

  var result = source.mapPar(function (val) { return val; });

  for (i = 0; i < dim1; i++) {
    for (j = 0; j < dim2; j++) {
      for (k = 0; k < dim3; k++) {
        for (l = 0; l < dim4; l++) {
          if (result[i][j][k][l] !== source[i][j][k][l]) {
            return false;
          }
        }
      }
    }
  }

  return true;
},
true
],
[
'Simple addition',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) { return val + 1; });
},
'2,3,4,5'
],
[
'Read index',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val, idx) { return val * idx; });
},
'0,2,6,12'
],
[
'Read source',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val, idx, src) { return src[3 - idx]; });
},
'4,3,2,1'
],
[
'The elemental function returns an object with arrayish properties',
function ()
{
  var source = [1, 2];
  var result = source.mapPar(function (val) { return {prop1 : val,
                                                      prop2 : [val, val + 10, val + 20],
                                                      prop3 : [[val +  100, val +  200, val +  300],
                                                               [val +  400, val +  500, val +  600],
                                                               [val +  700, val +  800, val +  900],
                                                               [val + 1000, val + 1100, val + 1200]]}; });
  return result[0].prop1 === 1 &&
         result[0].prop2[0] === 1 &&
         result[0].prop2[1] === 11 &&
         result[0].prop2[2] === 21 &&
         result[0].prop3[0][0] === 101 &&
         result[0].prop3[0][1] === 201 &&
         result[0].prop3[0][2] === 301 &&
         result[0].prop3[1][0] === 401 &&
         result[0].prop3[1][1] === 501 &&
         result[0].prop3[1][2] === 601 &&
         result[0].prop3[2][0] === 701 &&
         result[0].prop3[2][1] === 801 &&
         result[0].prop3[2][2] === 901 &&
         result[0].prop3[3][0] === 1001 &&
         result[0].prop3[3][1] === 1101 &&
         result[0].prop3[3][2] === 1201 &&
         result[1].prop1 === 2 &&
         result[1].prop2[0] === 2 &&
         result[1].prop2[1] === 12 &&
         result[1].prop2[2] === 22 &&
         result[1].prop3[0][0] === 102 &&
         result[1].prop3[0][1] === 202 &&
         result[1].prop3[0][2] === 302 &&
         result[1].prop3[1][0] === 402 &&
         result[1].prop3[1][1] === 502 &&
         result[1].prop3[1][2] === 602 &&
         result[1].prop3[2][0] === 702 &&
         result[1].prop3[2][1] === 802 &&
         result[1].prop3[2][2] === 902 &&
         result[1].prop3[3][0] === 1002 &&
         result[1].prop3[3][1] === 1102 &&
         result[1].prop3[3][2] === 1202;
},
true
],
[
'The elemental function returns an object with arrayish properties from the source',
function ()
{
  var source = [[[ 1,  2,  3,  4], [ 5,  6,  7,  8], [ 9, 10, 11, 12]],
                [[13, 14, 15, 16], [17, 18, 19, 20], [21, 22, 23, 24]]];
  var result = source.mapPar(function (val) { return {prop1 : val[1][2],
                                                      prop2 : val[2],
                                                      prop3 : val}; });
  return result[0].prop1 === 7 &&
         result[0].prop2[0] === 9 &&
         result[0].prop2[1] === 10 &&
         result[0].prop2[2] === 11 &&
         result[0].prop2[3] === 12 &&
         result[0].prop3[0][0] === 1 &&
         result[0].prop3[0][1] === 2 &&
         result[0].prop3[0][2] === 3 &&
         result[0].prop3[0][3] === 4 &&
         result[0].prop3[1][0] === 5 &&
         result[0].prop3[1][1] === 6 &&
         result[0].prop3[1][2] === 7 &&
         result[0].prop3[1][3] === 8 &&
         result[0].prop3[2][0] === 9 &&
         result[0].prop3[2][1] === 10 &&
         result[0].prop3[2][2] === 11 &&
         result[0].prop3[2][3] === 12 &&
         result[1].prop1 === 19 &&
         result[1].prop2[0] === 21 &&
         result[1].prop2[1] === 22 &&
         result[1].prop2[2] === 23 &&
         result[1].prop2[3] === 24 &&
         result[1].prop3[0][0] === 13 &&
         result[1].prop3[0][1] === 14 &&
         result[1].prop3[0][2] === 15 &&
         result[1].prop3[0][3] === 16 &&
         result[1].prop3[1][0] === 17 &&
         result[1].prop3[1][1] === 18 &&
         result[1].prop3[1][2] === 19 &&
         result[1].prop3[1][3] === 20 &&
         result[1].prop3[2][0] === 21 &&
         result[1].prop3[2][1] === 22 &&
         result[1].prop3[2][2] === 23 &&
         result[1].prop3[2][3] === 24;
},
true
]
];

var tests = [group_mapPar];
