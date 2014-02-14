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
'An identity function',
function ()
{
  return [1, 2, 3, 4].mapPar(function (val) { return val; });
},
'1,2,3,4'
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
]
];

var tests = [group_mapPar];
