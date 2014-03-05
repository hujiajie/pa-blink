/*
 * Copyright (c) 2011, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

"use strict";

(function () {
    var enableFallback = false;

    // Check whether the OpenCL interface is available.
    var useFFInterface = false;
    var useCrInterface = false;
    try {
        if (RiverTrail.Helper.hasFirefoxInterface()) {
            useFFInterface = true;
        } else if (RiverTrail.Helper.hasChromeInterface()) {
            useCrInterface = true;
        }
    } catch (ignore) {
        // useFFInterface = false;
        // useCrInterface = false;
    }
    var useInterface = useFFInterface || useCrInterface;

    // Check whether the OpenCL implementation supports double.
    var enable64BitFloatingPoint = false;
    if (useInterface) {
        var extensions;
        try {
            extensions = RiverTrail.compiler.openCLContext.extensions;
        } catch (ignore) {
            // extensions = undefined;
        }
        if (!extensions) {
            var dpoI;
            var dpoP;
            var dpoC;
            try {
                if (useFFInterface)
                    dpoI = new DPOInterface();
                else if (useCrInterface)
                    dpoI = new CInterface();
                dpoP = dpoI.getPlatform();
                dpoC = dpoP.createContext();

                extensions = dpoC.extensions || dpoP.extensions;
            } catch (e) {
                console.log("Unable to create new DPOInterface()/CInterface(): "
                    + e);
            }
        }
        enable64BitFloatingPoint = (extensions.indexOf("cl_khr_fp64") !== -1);
    }

    // Add the parallel APIs to Array.prototype as properties. To make them non-
    // enumerable, 'Object.defineProperty' is used here.

    // mapPar
    //
    // Synopsis
    //     myArray.mapPar(elementalFunction)
    //
    // Arguments
    //     elementalFunction - described below
    //
    // Elemental Function
    //     function (element, index, source)
    //     element - The element from the source.
    //     index   - The index in source where element is located as well as
    //               where the result will be placed.
    //     source  - The source holding the elements.
    //     The result of the function will be placed in mapPar's result at
    //     index.
    //
    // Returns
    //     A freshly minted Array. Elements are the results of applying the
    //     elemental function to the elements in the original Array coerced to
    //     the type grain of the source.
    //
    // Throws
    //     TypeError when elementalFunction is not a function.
    //
    // Discussion
    //     One functionally correct implementation of mapPar would be to use the
    //     sequential map.
    //
    // Example: an identity function
    //     result = myArray.mapPar(function(val){return val;});
    //
    Object.defineProperty(Array.prototype, "mapPar", { value : function (f) {
        var result;

        // 1. 'f' needs to be a function.
        if (typeof(f) !== "function") {
            throw new TypeError(f + " is not a function");
        }

        // 2. The array should only contain number elements and it must be
        //    regular. Also, it should not be empty. All these requirements are
        //    checked when it's converted to a FlatArray object, so we actually
        //    do nothing here.

        if (useInterface) {
            try {
                result = RiverTrail.compiler.compileAndGo(this, f, "mapPar", enable64BitFloatingPoint);
            } catch (e) {
                console.log(e);
                if (enableFallback) {
                    // Try to execute 'f' in sequential.
                    result = this.map(f);
                }
            }
        } else if (enableFallback) {
            result = this.map(f);
        }

        return result;
    } });
})();
