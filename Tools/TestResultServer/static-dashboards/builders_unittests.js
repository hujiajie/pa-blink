// Copyright (C) 2012 Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

module('builders');

test('loading steps', 4, function() {
    var tests = {}
    var baseUrl = 'http://dummyurl';
    var name = 'dummyname';
    var master = new builders.BuilderMaster(name, baseUrl, tests);

    var builder = 'dummybuilder';
    var buildNumber = 12345;
    equal(master.logPath(builder, buildNumber), baseUrl + '/builders/' + builder + '/builds/' + buildNumber);
    equal(master.builderJsonPath(), baseUrl + '/json/builders');
    equal(master.tests, tests);
    equal(master.name, name);
});

test('builders._builderFilter', 5, function() {
    var filter = builders._builderFilter('@ToT Blink', 'layout-tests');
    equal(filter('WebKit (Content Shell) Linux'), true, 'show content shell builder');
    equal(filter('WebKit Linux'), true, 'show generic webkit builder');
    equal(filter('Android Tests (dbg) '), false, 'don\'t show android tests');

    var filter = builders._builderFilter('@ToT Chromium', 'webkit_unit_tests');
    equal(filter('WebKit Win7 (deps)'), true, 'show DEPS builder');
    equal(filter('WebKit Win7'), false, 'don\'t show non-deps builder');
});

test('builders.groupNamesForTestType', 4, function() {
    var names = builders.groupNamesForTestType('layout-tests');
    equal(names.indexOf('@ToT Blink') != -1, true, 'include layout-tests in ToT');
    equal(names.indexOf('@ToT Chromium') != -1, true, 'include layout-tests in DEPS');

    names = builders.groupNamesForTestType('ash_unittests');
    equal(names.indexOf('@ToT Blink') != -1, false, 'don\'t include interactive_ui_tests in ToT');
    equal(names.indexOf('@ToT Chromium') != -1, true, 'include interactive_ui_tests in DEPS');
});

test('BuilderGroup.isToTBlink', 2, function() {
    var builderGroup = builders.loadBuildersList('@ToT Blink', 'layout-tests');
    equal(builderGroup.isToTBlink, true);
    builderGroup = builders.loadBuildersList('@ToT Chromium', 'layout-tests');
    equal(builderGroup.isToTBlink, false);
});

test('builders.loadBuildersList', 4, function() {
    resetGlobals();

    builders.loadBuildersList('@ToT Blink', 'layout-tests');
    var expectedBuilder = 'WebKit Win';
    equal(expectedBuilder in builders.getBuilderGroup().builders, true, expectedBuilder + ' should be among current builders');

    builders.loadBuildersList('@ToT Chromium', 'layout-tests');
    expectedBuilder = 'WebKit Linux (deps)'
    equal(expectedBuilder in builders.getBuilderGroup().builders, true, expectedBuilder + ' should be among current builders');
    expectedBuilder = 'XP Tests (1)'
    equal(expectedBuilder in builders.getBuilderGroup().builders, false, expectedBuilder + ' should not be among current builders');

    builders.loadBuildersList('@ToT Chromium', 'interactive_ui_tests');
    equal(expectedBuilder in builders.getBuilderGroup().builders, true, expectedBuilder + ' should be among current builders');
});
