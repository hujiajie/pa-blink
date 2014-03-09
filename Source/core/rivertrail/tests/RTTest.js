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
],
[
'Read global scalar variable in the top-level elemental function',
function ()
{
  global = 1;
  var result = [1, 2, 3, 4].mapPar(function (val) { return val + global; });
  delete global;
  return result;
},
[2, 3, 4, 5]
],
[
'Read global vector variable in the top-level elemental function',
function ()
{
  global = [1, 2, 3, 4];
  var result = [1, 2, 3, 4].mapPar(function (val) { return val + global[3]; });
  delete global;
  return result;
},
[5, 6, 7, 8]
],
[
'Read nested global array in the top-level elemental function',
function ()
{
  global = [[1, 2, 3], [4, 5, 6]];
  var result = [1, 2, 3, 4].mapPar(function (val) { return val + global[1][2]; });
  delete global;
  return result;
},
[7, 8, 9, 10]
],
[
'Read global scalar variable in a called function which is defined in the elemental function',
function ()
{
  global = 1;
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     function f()
                                     {
                                       return global;
                                     }
                                     return val + f();
                                   });
  delete global;
  return result;
},
[2, 3, 4, 5]
],
[
'Read global vector variable in a called function which is defined in the elemental function',
function ()
{
  global = [1, 2, 3, 4];
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     function f()
                                     {
                                       return global[3];
                                     }
                                     return val + f();
                                   });
  delete global;
  return result;
},
[5, 6, 7, 8]
],
[
'Read nested global array in a called function which is defined in the elemental function',
function ()
{
  global = [[1, 2, 3], [4, 5, 6]];
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     function f()
                                     {
                                       return global[1][2];
                                     }
                                     return val + f();
                                   });
  delete global;
  return result;
},
[7, 8, 9, 10]
],
[
'Read global scalar variable in a called function which is defined outside the elemental function',
function ()
{
  global = 1;
  f = function () { return global; };
  var result = [1, 2, 3, 4].mapPar(function (val) { return val + f(); });
  delete global;
  delete f;
  return result;
},
[2, 3, 4, 5]
],
[
'Read global vector variable in a called function which is defined outside the elemental function',
function ()
{
  global = [1, 2, 3, 4];
  f = function () { return global[3]; };
  var result = [1, 2, 3, 4].mapPar(function (val) { return val + f(); });
  delete global;
  delete f;
  return result;
},
[5, 6, 7, 8]
],
[
'Read nested global array in a called function which is defined outside the elemental function',
function ()
{
  global = [[1, 2, 3], [4, 5, 6]];
  f = function () { return global[1][2]; };
  var result = [1, 2, 3, 4].mapPar(function (val) { return val + f(); });
  delete global;
  delete f;
  return result;
},
[7, 8, 9, 10]
],
[
'A local variable in the called function shares the same name with a local variable in the top-level elemental function',
function ()
{
  f = function () {
        var x;
        x = 1;
        return x;
      };
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     var x;
                                     x = 2;
                                     function g()
                                     {
                                       var x;
                                       x = 3;
                                       return x;
                                     }
                                     return val + x + f() + g();
                                   });
  delete f;
  return result;
},
[7, 8, 9, 10]
],
[
'A local array in the called function shares the same name with a local scalar variable in the top-level elemental function',
function ()
{
  f = function () {
        var x;
        x = [1, 2, 3, 4];
        return x[3];
      };
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     var x;
                                     x = 1;
                                     function g()
                                     {
                                       var x;
                                       x = [[1, 2, 3], [4, 5, 6]];
                                       return x[1][2];
                                     }
                                     return val + x + f() + g();
                                   });
  delete f;
  return result;
},
[12, 13, 14, 15]
],
[
'A global variable referenced in the called function shares the same name with a local variable in the top-level elemental function',
function ()
{
  x = 1;
  f = function () { return x; };
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     var x;
                                     x = 2;
                                     return val + x + f();
                                   });
  delete x;
  delete f;
  return result;
},
[4, 5, 6, 7]
],
[
'A locally defined function shares the same name with a global function',
function ()
{
  f = function (x) { return x + 1; };
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     function f(x)
                                     {
                                       return x - 1;
                                     }
                                     return f(val);
                                   });
  delete f;
  return result;
},
[0, 1, 2, 3]
],
[
'Call Math.abs in the top-level elemental function',
function ()
{
  return [1, -2, 3, -4].mapPar(function (val) { return Math.abs(val); });
},
[1, 2, 3, 4]
],
[
'Call Math.abs in a called function',
function ()
{
  f = function (x) { return Math.abs(x); };
  var result = [1, -2, 3, -4].mapPar(function (val) {
                                       function g(x)
                                       {
                                         return Math.abs(x);
                                       }
                                       return f(val) + g(val);
                                     });
  delete f;
  return result;
},
[2, 4, 6, 8]
],
[
'Assign a local array to another local array',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) {
                               var arr1;
                               var arr2;
                               arr1 = [val + 1, val + 2, val + 3, val + 4];
                               arr2 = [val + 5, val + 6, val + 7, val + 8];
                               arr2 = arr1;
                               return arr2;
                             });
},
(function ()
{
  return [1, 2, 3, 4].map(function (val) {
                            var arr1;
                            var arr2;
                            arr1 = [val + 1, val + 2, val + 3, val + 4];
                            arr2 = [val + 5, val + 6, val + 7, val + 8];
                            arr2 = arr1;
                            return arr2;
                          });
})()
],
[
'Assign a local array to the element of a nested local array',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) {
                               var arr1;
                               var arr2;
                               arr1 = [val + 7, val + 8, val + 9];
                               arr2 = [[1, 2, 3], [4, 5, 6]];
                               arr2[1] = arr1;
                               return arr2;
                             });
},
(function ()
{
  return [1, 2, 3, 4].map(function (val) {
                            var arr1;
                            var arr2;
                            arr1 = [val + 7, val + 8, val + 9];
                            arr2 = [[1, 2, 3], [4, 5, 6]];
                            arr2[1] = arr1;
                            return arr2;
                          });
})()
],
[
'Assign a global array to a local array',
function ()
{
  global = [1, 2, 3, 4];
  var result = [5, 6, 7, 8].mapPar(function (val) {
                                     var local;
                                     local = [val + 1, val + 2, val + 3, val + 4];
                                     local = global;
                                     return local;
                                   });
  delete global;
  return result;
},
(function ()
{
  global = [1, 2, 3, 4];
  var result = [5, 6, 7, 8].map(function (val) {
                                  var local;
                                  local = [val + 1, val + 2, val + 3, val + 4];
                                  local = global;
                                  return local;
                                });
  delete global;
  return result;
})()
],
[
'Assign a global array to the element of a nested local array',
function ()
{
  global = [7, 8, 9];
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     var local;
                                     local = [[1, 2, 3], [4, 5, 6]];
                                     local[1] = global;
                                     return local;
                                   });
  delete global;
  return result;
},
(function ()
{
  global = [7, 8, 9];
  var result = [1, 2, 3, 4].map(function (val) {
                                  var local;
                                  local = [[1, 2, 3], [4, 5, 6]];
                                  local[1] = global;
                                  return local;
                                });
  delete global;
  return result;
})()
],
[
'Assign a local array to a global array',
function ()
{
  global = [0, 0, 0, 0];
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     var local;
                                     local = [5, 6, 7, 8];
                                     global = local;
                                     return global;
                                   });
  delete global;
  return result;
},
(function ()
{
  global = [0, 0, 0, 0];
  var result = [1, 2, 3, 4].map(function (val) {
                                  var local;
                                  local = [5, 6, 7, 8];
                                  global = local;
                                  return global;
                                });
  delete global;
  return result;
})()
],
[
'Assign a local array to the element of a nested global array',
function ()
{
  global = [[1, 2, 3], [4, 5, 6]];
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     var local;
                                     local = [7, 8, 9];
                                     global[1] = local;
                                     return global;
                                   });
  delete global;
  return result;
},
(function ()
{
  global = [[1, 2, 3], [4, 5, 6]];
  var result = [1, 2, 3, 4].map(function (val) {
                                  var local;
                                  local = [7, 8, 9];
                                  global[1] = local;
                                  return global;
                                });
  delete global;
  return result;
})()
],
[
'Assign a global array to another global array',
function ()
{
  arr1 = [1, 2, 3, 4];
  arr2 = [5, 6, 7, 8];
  var result = [0, 0, 0, 0].mapPar(function (val) {
                                     arr2 = arr1;
                                     return arr2;
                                   });
  delete arr1;
  delete arr2;
  return result;
},
(function ()
{
  arr1 = [1, 2, 3, 4];
  arr2 = [5, 6, 7, 8];
  var result = [0, 0, 0, 0].map(function (val) {
                                  arr2 = arr1;
                                  return arr2;
                                });
  delete arr1;
  delete arr2;
  return result;
})()
],
[
'Assign a global array to the element of a nested global array',
function ()
{
  arr1 = [[1, 2, 3], [4, 5, 6]];
  arr2 = [7, 8, 9];
  var result = [0, 0, 0, 0].mapPar(function (val) {
                                     arr1[1] = arr2;
                                     return arr1;
                                   });
  delete arr1;
  delete arr2;
  return result;
},
(function ()
{
  arr1 = [[1, 2, 3], [4, 5, 6]];
  arr2 = [7, 8, 9];
  var result = [0, 0, 0, 0].map(function (val) {
                                  arr1[1] = arr2;
                                  return arr1;
                                });
  delete arr1;
  delete arr2;
  return result;
})()
],
[
'Assign an integer array to a double array',
function ()
{
  return [0, 0, 0, 0].mapPar(function (val) {
                               var arr1;
                               var arr2;
                               arr1 = [1, 2, 3, 4];
                               arr2 = [1.1, 2.2, 3.3, 4.4];
                               arr2 = arr1;
                               return arr2;
                             });
},
(function ()
{
  return [0, 0, 0, 0].map(function (val) {
                            var arr1;
                            var arr2;
                            arr1 = [1, 2, 3, 4];
                            arr2 = [1.1, 2.2, 3.3, 4.4];
                            arr2 = arr1;
                            return arr2;
                          });
})()
],
[
'Assign an unsigned integer array to a double array',
function ()
{
  arr1 = new Uint8ClampedArray([1, 2, 3, 4]);
  arr2 = new Float64Array([1.1, 2.2, 3.3, 4.4]);
  var result = [0, 0, 0, 0].mapPar(function (val) {
                                     var arr3;
                                     var arr4;
                                     arr3 = [arr1[0], arr1[1], arr1[2], arr1[3]];
                                     arr4 = [arr2[0], arr2[1], arr2[2], arr2[3]];
                                     arr4 = arr3;
                                     return arr4;
                                   });
  delete arr1;
  delete arr2;
  return result;
},
(function ()
{
  arr1 = new Uint8ClampedArray([1, 2, 3, 4]);
  arr2 = new Float64Array([1.1, 2.2, 3.3, 4.4]);
  var result = [0, 0, 0, 0].map(function (val) {
                                  var arr3;
                                  var arr4;
                                  arr3 = [arr1[0], arr1[1], arr1[2], arr1[3]];
                                  arr4 = [arr2[0], arr2[1], arr2[2], arr2[3]];
                                  arr4 = arr3;
                                  return arr4;
                                });
  delete arr1;
  delete arr2;
  return result;
})()
],
[
'Assign a float array to a double array',
function ()
{
  arr1 = new Float32Array([1.1, 2.2, 3.3, 4.4]);
  arr2 = new Float64Array([5.5, 6.6, 7.7, 8.8]);
  var result = [0, 0, 0, 0].mapPar(function (val) {
                                     var arr3;
                                     var arr4;
                                     arr3 = [arr1[0], arr1[1], arr1[2], arr1[3]];
                                     arr4 = [arr2[0], arr2[1], arr2[2], arr2[3]];
                                     arr4 = arr3;
                                     return arr4;
                                   });
  delete arr1;
  delete arr2;
  return result;
},
(function ()
{
  arr1 = new Float32Array([1.1, 2.2, 3.3, 4.4]);
  arr2 = new Float64Array([5.5, 6.6, 7.7, 8.8]);
  var result = [0, 0, 0, 0].map(function (val) {
                                  var arr3;
                                  var arr4;
                                  arr3 = [arr1[0], arr1[1], arr1[2], arr1[3]];
                                  arr4 = [arr2[0], arr2[1], arr2[2], arr2[3]];
                                  arr4 = arr3;
                                  return arr4;
                                });
  delete arr1;
  delete arr2;
  return result;
})()
],
[
'Assign a double array to a float array',
function ()
{
  arr1 = new Float64Array([1.1, 2.2, 3.3, 4.4]);
  arr2 = new Float32Array([5.5, 6.6, 7.7, 8.8]);
  var result = [0, 0, 0, 0].mapPar(function (val) {
                                     var arr3;
                                     var arr4;
                                     arr3 = [arr1[0], arr1[1], arr1[2], arr1[3]];
                                     arr4 = [arr2[0], arr2[1], arr2[2], arr2[3]];
                                     arr4 = arr3;
                                     return arr4;
                                   });
  delete arr1;
  delete arr2;
  return result;
},
(function ()
{
  arr1 = new Float64Array([1.1, 2.2, 3.3, 4.4]);
  arr2 = new Float32Array([5.5, 6.6, 7.7, 8.8]);
  var result = [0, 0, 0, 0].map(function (val) {
                                  var arr3;
                                  var arr4;
                                  arr3 = [arr1[0], arr1[1], arr1[2], arr1[3]];
                                  arr4 = [arr2[0], arr2[1], arr2[2], arr2[3]];
                                  arr4 = arr3;
                                  return arr4;
                                });
  delete arr1;
  delete arr2;
  return result;
})()
],
[
'Assign a local array which may be overwrite before it has been read to another local array',
function ()
{
  return [0, 0, 0, 0].mapPar(function (val) {
                               var arr1;
                               var arr2;
                               arr1 = [0, 0, 0, 0];
                               arr2 = [0, 0, 0, 0];
                               for (var i = 0; i < 2; i++) {
                                 if (i === 0) {
                                   arr1 = [1, 2, 3, 4];
                                 } else {
                                   arr2 = [5, 6, 7, 8];
                                 }
                               }
                               return [arr1, arr2];
                             });
},
(function ()
{
  return [0, 0, 0, 0].map(function (val) {
                            var arr1;
                            var arr2;
                            arr1 = [0, 0, 0, 0];
                            arr2 = [0, 0, 0, 0];
                            for (var i = 0; i < 2; i++) {
                              if (i === 0) {
                                arr1 = [1, 2, 3, 4];
                              } else {
                                arr2 = [5, 6, 7, 8];
                              }
                            }
                            return [arr1, arr2];
                          });
})()
],
[
'Initialize a nested local array with flat local arrays',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) {
                               var arr1;
                               var arr2;
                               var arr3;
                               arr1 = [1, 2, 3];
                               arr2 = [4, 5, 6];
                               arr3 = [arr1, arr2];
                               return arr3[1][2];
                             });
},
[6, 6, 6, 6]
],
[
'Initialize a nested local array with a flat local array and a flat global array',
function ()
{
  global = [1, 2, 3];
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     var arr1;
                                     var arr2;
                                     arr1 = [4, 5, 6];
                                     arr2 = [arr1, global];
                                     return arr2[1][2];
                                   });
  delete global;
  return result;
},
[3, 3, 3, 3]
],
[
'Initialize a nested local array with nested local arrays',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) {
                               var arr1;
                               var arr2;
                               var arr3;
                               arr1 = [[1, 2, 3], [ 4,  5,  6]];
                               arr2 = [[7, 8, 9], [10, 11, 12]];
                               arr3 = [arr1, arr2];
                               return arr3[1][1][2];
                             });
},
[12, 12, 12, 12]
],
[
'Initialize a nested local array with a nested local array and a nested global array',
function ()
{
  global = [[1, 2, 3], [4, 5, 6]];
  var result = [1, 2, 3, 4].mapPar(function (val) {
                                     var arr1;
                                     var arr2;
                                     arr1 = [[7, 8, 9], [10, 11, 12]];
                                     arr2 = [arr1, global];
                                     return arr2[1][1][2];
                                   });
  delete global;
  return result;
},
[6, 6, 6, 6]
],
[
'Pass Uint8ClampedArray as an argument to the called function',
function ()
{
  global = new Uint8ClampedArray([1, 2, 3, 4]);
  var result = [0, 0, 0, 0].mapPar(function (val, idx) {
                                     function f(ta, idx)
                                     {
                                       return ta[idx];
                                     }
                                     return f(global, idx);
                                   });
  delete global;
  return result;
},
[1, 2, 3, 4]
],
[
'Pass Float32Array as an argument to the called function',
function ()
{
  global = new Float32Array([1, 2, 3, 4]);
  var result = [0, 0, 0, 0].mapPar(function (val, idx) {
                                     function f(ta, idx)
                                     {
                                       return ta[idx];
                                     }
                                     return f(global, idx);
                                   });
  delete global;
  return result;
},
[1, 2, 3, 4]
],
[
'Pass Float64Array as an argument to the called function',
function ()
{
  global = new Float64Array([1, 2, 3, 4]);
  var result = [0, 0, 0, 0].mapPar(function (val, idx) {
                                     function f(ta, idx)
                                     {
                                       return ta[idx];
                                     }
                                     return f(global, idx);
                                   });
  delete global;
  return result;
},
[1, 2, 3, 4]
],
[
'The called function returns a global array',
function ()
{
  global = [1, 2, 3, 4];
  var result = [0, 0, 0, 0].mapPar(function (val) {
                                     function f()
                                     {
                                       return global;
                                     }
                                     var local;
                                     local = [5, 6, 7, 8];
                                     local = f();
                                     return local;
                                   });
  delete global;
  return result;
},
(function ()
{
  global = [1, 2, 3, 4];
  var result = [0, 0, 0, 0].map(function (val) {
                                  function f()
                                  {
                                    return global;
                                  }
                                  var local;
                                  local = [5, 6, 7, 8];
                                  local = f();
                                  return local;
                                });
  delete global;
  return result;
})()
],
[
'The called function has no return value',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) {
                               function f()
                               {
                               }
                               f();
                               return val;
                             });
},
[1, 2, 3, 4]
]
];

var tests = [group_mapPar];
