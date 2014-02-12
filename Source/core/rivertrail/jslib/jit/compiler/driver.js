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
//
// Create top level compiler object
//

// This pattern is used when we want to hide internal detail and have only one such variable.
// It basically hides everything in the local scope of the function, executes the function and
// returns only the external needed outside of the function.

if (RiverTrail === undefined) {
    var RiverTrail = {};
}

var useFF4Interface;
var useCrInterface;

RiverTrail.compiler = (function () {
    // This is the compiler driver proper. 
    
    // The ast is opaque at this point so the Narcissus constants aren't needed.
    
    // whether to use kernel caching or not
    var useKernelCaching = true;

    var openCLContext; 
    var dpoInterface;
    var dpoPlatform;
    try {
        var Sys = {};
        var ua = navigator.userAgent.toLowerCase();
        var s;
        var chrome = false;
        var firefox = false;
        (s = ua.match(/firefox\/([\d.]+)/)) ? firefox = true :
        (s = ua.match(/chrome\/([\d.]+)/)) ? chrome = true : 0;

        if (firefox && Components.interfaces.dpoIInterface !== undefined) {
            useFF4Interface = true;
        }else if (chrome && CInterface !== undefined) {
            useCrInterface = true;
        }
    } catch (e) {
        console.log("Cannot initialise OpenCL interface. Please check the whether the extension was installed and try again.");
        throw Error("Cannot initialise OpenCL Interface: " + JSON.stringify(e));
   }

    try {
        if (useFF4Interface)
            dpoInterface = new DPOInterface();
        else if (useCrInterface)
            dpoInterface = new CInterface();
        dpoPlatform = dpoInterface.getPlatform(); 
        openCLContext = dpoPlatform.createContext();
    } catch (e) {
        console.log ("Cannot initialise OpenCL interface. Please check the whether the extension was installed and try again.");
        throw Error("Cannot initialise OpenCL Interface: " + JSON.stringify(e));
    }

    // check whether we have the right version of the extension; as the user has some extension installed, he probably wants to use
    // the right one for this library, so we alert him
    if (dpoInterface.version !== 2) {
        alert("This webpage requires a newer version of the RiverTrail Firefox extension. Please visit http://github.com/rivertrail/rivertrail/downloads.");
        throw Error("RiverTrail extension out of date");
    }

    // main hook to start the compilation/execution process for running a construct using OpenCL
    // source -> the source holding the elements
    // f -> function to run
    // construct -> 'mapPar' only so far
    var compileAndGo = function compileAndGo (source, f, construct, enable64BitFloatingPoint) {
        var result = null;
        var kernelString;
        var lowPrecision;
        var ast;
        lowPrecision = !enable64BitFloatingPoint;

        // First convert the source into suitable flat representations that can be passed to
        // the OpenCL side

        var flatSource = new RiverTrail.Helper.FlatArray( lowPrecision ? Float32Array : Float64Array, source);

        if (f.openCLCache !== undefined) {
            if (useKernelCaching) {
                var cacheEntry = getCacheEntry(f, construct, flatSource, lowPrecision);
                // try and find a matching kernel from previous runs
                if (cacheEntry != null) {
                    result = RiverTrail.compiler.runOCL(flatSource, cacheEntry.kernel, cacheEntry.ast, f, 
                                      construct, lowPrecision, enable64BitFloatingPoint, useKernelCaching);
                    return result;
                }
            } else {
                // remove cache 
                f.openCLCache = undefined;
            }
        } 
        //
        // NOTE: we only get here if caching has failed!
        //
        if (useKernelCaching && (f.openCLCache === undefined)) {
            // create empty cache
            f.openCLCache = [];
        }
                        
        try {
            ast = parse(flatSource, construct, f.toString(), lowPrecision); // parse, no code gen
            kernelString = RiverTrail.compiler.codeGen.compile(ast, construct); // Creates an OpenCL kernel function
        } catch (e) {
            RiverTrail.Helper.debugThrow(e);
        }
        
        if (RiverTrail.compiler.verboseDebug) {    
            console.log("::parseGenRunOCL:kernelString: ", kernelString);
        }

        try {
            result = RiverTrail.compiler.runOCL(flatSource, kernelString, ast, f, construct,
                                            lowPrecision, enable64BitFloatingPoint, useKernelCaching);
        } catch (e) {
            try {
                RiverTrail.Helper.debugThrow(e + RiverTrail.compiler.openCLContext.buildLog);
            } catch (e2) {
                RiverTrail.Helper.debugThrow(e); // ignore e2. If buildlog throws, there simply is none.
            }
        }
        // NOTE: Do not add general code here. This is not the only exit from this function!
        return result;
    };

    //
    // Driver method to steer compilation process
    //
    function parse(source, construct, kernel, lowPrecision) {
        var ast = RiverTrail.Helper.parseFunction(kernel);
        try {
            RiverTrail.Typeinference.analyze(ast, source, construct, lowPrecision);
            RiverTrail.RangeAnalysis.analyze(ast, source, construct);
            RiverTrail.RangeAnalysis.propagate(ast, construct);
            RiverTrail.InferBlockFlow.infer(ast);
            RiverTrail.InferMem.infer(ast);
        } catch (e) {
            RiverTrail.Helper.debugThrow(e);
        }
        return ast;
    }
    
    var getCacheEntry = function (f, construct, source, lowPrecision) {
        var i;
        var entry;
        // try and find a matching kernel from previous runs
        for (i = 0; i < f.openCLCache.length; i++) {
            entry = f.openCLCache[i];
            if ((construct === entry.construct) &&
                (lowPrecision === entry.lowPrecision) &&
                (entry.source === f) &&
                (source.inferredType === entry.sourceType.inferredType) &&
                (equalsShape(source.dimSize, entry.sourceType.dimSize))
               ) {
                return f.openCLCache[i];
            }
        }
        return null;
    };

    function astTypeConverter (key, value) {
        if (key === 'type' && (typeof value === 'number') ) { 
            if (opTypeNames[tokens[value]]) {
                return opTypeNames[tokens[value]];
            }
            // only do numbers since JSON recurses on returned value.
            return tokens[value];
        }
        if (key === 'tokenizer'  ) {
            return '-- hidden --';
        }
        if (key === 'flowTo') {
            return '-- cyclic --';
        }
        return value;
    }   

    //
    // Method to dump the current ast
    //
    function dumpAst(ast) {
        if (RiverTrail.compiler.verboseDebug) {
            console.log(JSON.stringify(ast, astTypeConverter));
        }
        if (RiverTrail.compiler.debug) {
            console.log(RiverTrail.Helper.wrappedPP(ast));
        }
    }
    
    var equalsShape = function equalsShape (shapeA, shapeB) {
        return ((shapeA.length == shapeB.length) &&
                Array.prototype.every.call(shapeA, function (a,idx) { return a == shapeB[idx];}));
    };
    
    return {
        verboseDebug: false,
        debug: false,
        compileAndGo: compileAndGo,
        openCLContext: openCLContext
    };
}());

