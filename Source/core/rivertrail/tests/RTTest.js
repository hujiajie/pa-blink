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

var group_ParallelArray = ['ParallelArray',
[
'Create an empty ParallelArray object',
function ()
{
  return new ParallelArray();
},
'[]'
],
[
'Create a new ParallelArray object with elemental functions',
function ()
{
  return new ParallelArray(4, function (x) { return x + 1; });
},
'[1, 2, 3, 4]'
],
[
'Create a new ParallelArray object with elemental functions and extra arguments',
function ()
{
  return new ParallelArray(4, function (x, arg1) { return x + arg1; }, 1);
},
'[1, 2, 3, 4]'
],
[
'Create a new ParallelArray object from a normal Array object',
function ()
{
  return new ParallelArray([1, 2, 3, 4]);
},
'[1, 2, 3, 4]'
],
[
'Create a new ParallelArray object with Float32Array',
function ()
{
  var pa = new ParallelArray(Float32Array, [1, 2, 3, 4]);
  return pa.data instanceof Float32Array;
},
true
],
[
'Create a new ParallelArray object with Float64Array',
function ()
{
  var pa = new ParallelArray(Float64Array, [1, 2, 3, 4]);
  return pa.data instanceof Float64Array;
},
true
],
[
'Create a new ParallelArray object from separate numbers',
function ()
{
  return new ParallelArray(1, 2, 3, 4);
},
'[1, 2, 3, 4]'
],
[
'Create a new 2-D ParallelArray object from a nested Array object',
function ()
{
  var pa = new ParallelArray([[0, 1], [2, 3], [4, 5]]);
  return [pa.get(0), pa.get(1), pa.get(2)];
},
'[0, 1],[2, 3],[4, 5]'
],
[
'Create a new 2-D ParallelArray object from an existing 2-D ParallelArray object',
function ()
{
  var pa1 = new ParallelArray([[0, 1], [2, 3], [4, 5]]);
  var pa2 = new ParallelArray(pa1);
  return [pa2.get(0), pa2.get(1), pa2.get(2)];
},
'[0, 1],[2, 3],[4, 5]'
],
[
'Create a new 2-D ParallelArray object from separate ParallelArray objects',
function ()
{
  var pa1 = new ParallelArray(0, 1);
  var pa2 = new ParallelArray(2, 3);
  var pa = new ParallelArray(pa1, pa2);
  return [pa.get(0), pa.get(1)];
},
'[0, 1],[2, 3]'
],
[
'Create a new irregular ParallelArray object from a nested Array object',
function ()
{
  var pa = new ParallelArray([[0, 1], [2]]);
  return [pa.get(0), pa.get(1)];
},
'[0, 1],[2]'
],
[
'Create a new irregular ParallelArray object from an array of ParallelArray elements',
function ()
{
  var pa1 = new ParallelArray(0, 1);
  var pa2 = new ParallelArray(2);
  var pa = new ParallelArray([pa1, pa2]);
  return [pa.get(0), pa.get(1)];
},
'[0, 1],[2]'
],
[
'Create a new 2-D ParallelArray object with elemental functions and scalar arguments',
function ()
{
  var pa = new ParallelArray(3, function (i) { return [i, i + 1]; });
  return [pa.get(0), pa.get(1), pa.get(2)];
},
'[0, 1],[1, 2],[2, 3]'
],
[
'Create a new 2-D ParallelArray object with elemental functions and vector arguments',
function ()
{
  var pa = new ParallelArray([3, 2], function (iv) { return iv[0] * iv[1]; });
  return [pa.get(0), pa.get(1), pa.get(2)];
},
'[0, 0],[0, 1],[0, 2]'
],
[
'Create a new 3-D ParallelArray object from a canvas object',
function ()
{
  var canvas = document.createElement('canvas');
  canvas.width = 4;
  canvas.height = 2;
  var context = canvas.getContext('2d');
  var imageData = context.getImageData(0, 0, 4, 2);
  for (var i = 0; i < imageData.data.length; i += 4) {
    imageData.data[i] = i;
    imageData.data[i + 1] = i + 1;
    imageData.data[i + 2] = i + 2;
    imageData.data[i + 3] = 255;
  }
  context.putImageData(imageData, 0, 0);
  var pa = new ParallelArray(canvas);
  return [pa.get([0, 0]), pa.get([0, 1]), pa.get([0, 2]), pa.get([0, 3]), pa.get([1, 0]), pa.get([1, 1]), pa.get([1, 2]), pa.get([1, 3])];
},
'[0, 1, 2, 255],[4, 5, 6, 255],[8, 9, 10, 255],[12, 13, 14, 255],[16, 17, 18, 255],[20, 21, 22, 255],[24, 25, 26, 255],[28, 29, 30, 255]'
]
];

var group_map = ['Map',
[
'An identity function',
function ()
{
  var pa = new ParallelArray(1, 2, 3, 4);
  return pa.map(function (x) { return x; });
},
'[1, 2, 3, 4]'
],
[
'Adding one to each element',
function ()
{
  var pa = new ParallelArray(1, 2, 3, 4);
  return pa.map(function (x) { return x + 1; });
},
'[2, 3, 4, 5]'
],
[
'Adding an extra argument to each element',
function ()
{
  var pa = new ParallelArray(1, 2, 3, 4);
  return pa.map(function (x, arg1) { return x + arg1; }, 1);
},
'[2, 3, 4, 5]'
]
];

var group_combine = ['Combine',
[
'An identity function',
function ()
{
  var pa = new ParallelArray(1, 2, 3, 4);
  return pa.combine(function (i) { return this.get(i); });
},
'[1, 2, 3, 4]'
],
[
'Element-wise addition of two vectors',
function ()
{
  var pa1 = new ParallelArray(1, 2, 3, 4);
  var pa2 = new ParallelArray(5, 6, 7, 8);
  return pa1.combine(function (i, pa) { return this.get(i) + pa.get(i); }, pa2);
},
'[6, 8, 10, 12]'
],
[
'Addition of two arrays of same shape',
function ()
{
  var pa1 = new ParallelArray([0, 1], [2, 3], [4, 5]);
  var pa2 = new ParallelArray([6, 7], [8, 9], [10, 11]);
  var pa = pa1.combine(pa1.getShape().length, function (iv, pa2) { return this.get(iv) + pa2.get(iv); }, pa2);
  return [pa.get(0), pa.get(1), pa.get(2)];
},
'[6, 8],[10, 12],[14, 16]'
],
[
'Increment each element by 1',
function ()
{
  var pa = new ParallelArray([1, 2, 3, 4, 5]);
  return pa.combine(function (i) { return this.get(i) + 1; });
},
'[2, 3, 4, 5, 6]'
],
[
'Reverse the elements',
function ()
{
  var pa = new ParallelArray([1, 2, 3, 4, 5]);
  return pa.combine(function (i) { return this.get(this.length - i[0] - 1); });
},
'[5, 4, 3, 2, 1]'
],
[
'Transpose a matrix using a depth of 2',
function ()
{
  var pa1 = new ParallelArray([4, 4], function (iv) { return iv[0] * iv[1]; });
  var pa2 = pa1.combine(2, function (iv) { return this.get(this.getShape()[0] - iv[0] - 1, this.getShape()[1] - iv[1] - 1); });
  return [pa2.get(0), pa2.get(1), pa2.get(2), pa2.get(3)];
},
'[9, 6, 3, 0],[6, 4, 2, 0],[3, 2, 1, 0],[0, 0, 0, 0]'
]
];

var group_reduce = ['Reduce',
[
'Calculate the sum of elements',
function ()
{
  var pa = new ParallelArray([1, 2, 3, 4, 5]);
  return pa.reduce(function (a, b) { return a + b; });
},
15
],
[
'Reduction of a ParallelArray object with extra arguments',
function ()
{
  var pa = new ParallelArray([6, 4, 2, 3, 5]);
  return pa.reduce(function (a, b, arg1) { var c = (a < b) ? a : b; return (c < arg1) ? c : arg1; }, 1);
},
1
],
[
'Reduction of a 2-D ParallelArray object',
function ()
{
  var pa = new ParallelArray(3, function (x) { var res = [0, 0]; res[0] = x; res[1] = x + 1; return res; });
  return pa.reduce(function (a, b) { return a + b; });
},
'[3, 6]',
'[0, 1][1, 2][2, 3]'
]
];

var group_scan = ['Scan',
[
'An identity function',
function ()
{
  var pa = new ParallelArray(1, 2, 3, 4);
  return pa.scan(function (a, b) { return b; });
},
'[1, 2, 3, 4]'
],
[
'Do a partial sum',
function ()
{
  var pa = new ParallelArray([1, 2, 3, 4, 5]);
  return pa.scan(function (a, b) { return a + b; });
},
'[1, 3, 6, 10, 15]'
],
[
'Scan with extra arguments',
function ()
{
  var pa = new ParallelArray([5, 4, 3, 2, 1]);
  return pa.scan(function (a, b, arg1) { var c = (a < b) ? a : b; return (c < arg1) ? c : arg1; }, 3);
},
'[5, 3, 3, 2, 1]'
]
];

var group_scatter = ['Scatter',
[
'Do the identity function',
function ()
{
  var pa = new ParallelArray([1, 2, 3, 4, 5]);
  var indices = [0, 1, 2, 3, 4];
  return pa.scatter(indices);
},
'[1, 2, 3, 4, 5]'
],
[
'Change the order of the elements',
function ()
{
  var pa = new ParallelArray([1, 2, 3, 4, 5]);
  return pa.scatter([4, 0, 3, 1, 2]);
},
'[2, 4, 5, 3, 1]'
],
[
'If there is a conflict use the max. Use 33 as a default value',
function ()
{
  var pa = new ParallelArray([1, 2, 3, 4, 5]);
  return pa.scatter([4, 0, 3, 4, 2], 33, function max(a, b) { return a > b ? a : b; });
},
'[2, 33, 5, 3, 4]'
],
[
'One way to do histogram',
function ()
{
  var source = new ParallelArray([1, 2, 2, 4, 2, 4, 5]);
  var ones = source.map(function one(v) { return 1; });
  var histogram = ones.scatter(source, 0, function (a, b) { return a + b; }, 6);
  return histogram;
},
'[0, 1, 3, 0, 2, 1]'
]
];

var group_filter = ['Filter',
[
'An identity function',
function ()
{
  var pa = new ParallelArray([1, 2, 3, 4]);
  return pa.filter(function () { return true; });
},
'[1, 2, 3, 4]'
],
[
'Filter out values that are not even',
function ()
{
  var source = new ParallelArray([1, 2, 3, 4, 5]);
  var even = source.filter(function even(iv) { return (this.get(iv) % 2) == 0; });
  return even;
},
'[2, 4]'
],
[
'Filter out values that are smaller than the extra argument',
function ()
{
  var pa = new ParallelArray([1, 2, 3, 4, 5]);
  return pa.filter(function (iv, arg1) { return this.get(iv) >= arg1; }, 3);
},
'[3, 4, 5]'
]
];

var group_flatten = ['Flatten',
[
'Flatten a 2-D ParallelArray object',
function ()
{
  var pa = new ParallelArray([[1, 2], [3, 4]]);
  return pa.flatten();
},
'[1, 2, 3, 4]'
],
[
'Flatten a 3-D ParallelArray object',
function ()
{
  var pa1 = new ParallelArray([[1, 2, 3], [4, 5, 6]], [[7, 8, 9], [10, 11, 12]]);
  var pa2 = pa1.flatten();
  return [pa2.get(0), pa2.get(1), pa2.get(2), pa2.get(3)];
},
'[1, 2, 3],[4, 5, 6],[7, 8, 9],[10, 11, 12]'
]
];

var group_partition = ['Partition',
[
'Partition a 1-D ParallelArray object',
function ()
{
  var pa1 = new ParallelArray([1, 2, 3, 4]);
  var pa2 = pa1.partition(2);
  return [pa2.get(0), pa2.get(1)];
},
'[1, 2],[3, 4]'
],
[
'Partition a 2-D ParallelArray object',
function ()
{
  var pa1 = new ParallelArray([[1, 2], [3, 4], [5, 6], [7, 8], [9, 10], [11, 12]]);
  var pa2 = pa1.partition(3);
  return [pa2.get([0, 0]), pa2.get([0, 1]), pa2.get([0, 2]), pa2.get([1, 0]), pa2.get([1, 1]), pa2.get([1, 2])];
},
'[1, 2],[3, 4],[5, 6],[7, 8],[9, 10],[11, 12]'
]
];

var group_get = ['Get',
[
'Get the inner-most element with an array of length 2',
function ()
{
  var pa = new ParallelArray([0, 1, 2, 3, 4], [10, 11, 12, 13, 14], [20, 21, 22, 23, 24]);
  return pa.get([1, 1]);
},
11
],
[
'Get the outer-most element with an array of length 1',
function ()
{
  var pa = new ParallelArray([0, 1, 2, 3, 4], [10, 11, 12, 13, 14], [20, 21, 22, 23, 24]);
  return pa.get([1]);
},
'[10, 11, 12, 13, 14]'
],
[
'Get the outer-most element with an integer argument',
function ()
{
  var pa = new ParallelArray([0, 1, 2, 3, 4], [10, 11, 12, 13, 14], [20, 21, 22, 23, 24]);
  return pa.get(1);
},
'[10, 11, 12, 13, 14]'
],
[
'Get the inner-most element with multiple integer arguments',
function ()
{
  var pa = new ParallelArray([0, 1, 2, 3, 4], [10, 11, 12, 13, 14], [20, 21, 22, 23, 24]);
  return pa.get(1, 1);
},
11
]
];

var tests = [group_ParallelArray, group_map, group_combine, group_reduce, group_scan, group_scatter, group_filter, group_flatten, group_partition, group_get];
