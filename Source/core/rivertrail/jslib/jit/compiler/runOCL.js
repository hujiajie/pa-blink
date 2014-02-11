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


if (RiverTrail === undefined) {
    var RiverTrail = {};
}

// Executes the kernel function with the args for the elemental function
// array        - the source holding the elements
// kernelString - either a JavaScript code string or a precompiled kernel (dpoIKernel/CKernel object)
// ast          - result from parsing
// f            - function to compile
// construct    - outer construct (currently mapPar only)

RiverTrail.compiler.runOCL = function () {
    var reportVectorized = false;

    // Executes the kernel function with the args for the elemental function
    // array        - the source holding the elements
    // kernelString - either a JavaScript code string or a precompiled kernel (dpoIKernel/CKernel object)
    // ast          - result from parsing
    // f            - function to compile
    // construct    - outer construct in (currently mapPar only)
    var runOCL = function runOCL(array, kernelString, ast, f, construct,
                                 lowPrecision, enable64BitFloatingPoint, useKernelCaching) {
        var result;
        var kernelArgs = [];
        var resultMem;
        var sourceType;
        var iterSpace;
        var kernel;
        var kernelName = ast.name;
        if (!kernelName) {
            throw new Error("Invalid ast: Function expected at top level");
        }


        var Sys = {};
        var ua = navigator.userAgent.toLowerCase();
        var s;
        var chrome = false;
        var firefox = false;
        (s = ua.match(/firefox\/([\d.]+)/)) ? firefox = true :
        (s = ua.match(/chrome\/([\d.]+)/)) ? chrome = true : 0;
        var InterfaceData;
        var InterfaceKernel;
        if (firefox) {
            InterfaceData = Components.interfaces.dpoIData;
            InterfaceKernel = Components.interfaces.dpoIKernel;
        } else if (chrome) {
            InterfaceData = CData;
            InterfaceKernel = CKernel;
        }

        if (construct === "mapPar") {
            // the source array should have been converted to a FlatArray object before
            sourceType = {"dimSize": array.shape, "inferredType" : RiverTrail.Helper.inferTypedArrayType(array.data)};
            iterSpace = [array.shape[0]];
            kernelArgs.push(RiverTrail.compiler.openCLContext.mapData(array.data));
        }
        // add memory for result
        // SAH: We have agreed that operations are elemental type preserving, thus I reuse the type
        //      of the argument here.
        var allocateAndMapResult = function (type) {
            var resultElemType = RiverTrail.Helper.stripToBaseType(type.OpenCLType);
            var resShape;
            if (type.properties) {
                resShape = iterSpace.concat(type.getOpenCLShape());
            } else {
                resShape = iterSpace;
            }
            var template = RiverTrail.Helper.elementalTypeToConstructor(resultElemType);
            if (template == undefined) throw new Error("cannot map inferred type to constructor");
            var memObj = RiverTrail.compiler.openCLContext.allocateData(new template(1), shapeToLength(resShape));
            kernelArgs.push(memObj);
            return {mem: memObj, shape: resShape};
        };

        // We allocate whatever the result type says. To ensure portability of 
        // the extension, we need a template typed array. So lets just create one!
        if (ast.typeInfo.result.isObjectType("InlineObject")) {
            // we have multiple return values
            resultMem = {};
            for (var name in ast.typeInfo.result.properties.fields) {
                resultMem[name] = allocateAndMapResult(ast.typeInfo.result.properties.fields[name]);
            }
        } else {
            // allocate and map the single result
            resultMem = allocateAndMapResult(ast.typeInfo.result);
        }
        // build kernel
        if (kernelString instanceof InterfaceKernel) {
            kernel = kernelString;
        } else {
            try {
                if (enable64BitFloatingPoint) {
                    // enable 64 bit extensions
                    kernelString = "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n" + kernelString;
                }
                kernel = RiverTrail.compiler.openCLContext.compileKernel(kernelString, "RT_" + kernelName);
            } catch (e) {
                try {
                    var log = RiverTrail.compiler.openCLContext.buildLog;
                } catch (e2) {
                    var log = "<not available>";
                }
                RiverTrail.Helper.debugThrow("The OpenCL compiler failed. Log was `" + log + "'.");
            }
            if (reportVectorized) {
                try {
                    var log = RiverTrail.compiler.openCLContext.buildLog;
                    if (log.indexOf("was successfully vectorized") !== -1) {
                        console.log(kernelName + "was successfully vectorized");
                    }
                } catch (e) {
                    // ignore
                }
            }
            if (useKernelCaching && (f !== undefined)) {
                // save ast information required for future use
                var cacheEntry = { "ast": ast,
                    "name": ast.name,
                    "source": f,
                    "sourceType": sourceType,
                    "kernel": kernel,
                    "construct": construct,
                    "lowPrecision": lowPrecision,
                    "iterSpace": iterSpace
                };
                f.openCLCache.push(cacheEntry);
            }
        }
        // set arguments
        kernelArgs.reduce(function (kernel, arg, index) {
            try {
                if (arg instanceof InterfaceData) {
                    kernel.setArgument(index, arg);
                } else {
                    throw new Error("unexpected kernel argument type!");
                }
                return kernel;
            } catch (e) {
                console.log("reduce error: ", e, " index: ", index, "arg: ", arg);
                throw e;
            }
        }, kernel);

        if (construct === "mapPar") {
            // The differences are to do with args to the elemental function and are dealt with there so we can use the same routine.
            // kernel.run(rank, shape, tiles)
            try {
                var kernelFailure = kernel.run(1, iterSpace);
            } catch (e) {
                console.log("kernel.run fails: ", e);
                throw e;
            }
            if (kernelFailure) {
                // a more helpful error message would be nice. However, we don't know why it failed. A better exception model is asked for...
                throw new Error("kernel execution failed: " + RiverTrail.compiler.codeGen.getError(kernelFailure));
            }
        } else {
            alert("runOCL only deals with mapPar (so far).");
        }
        var res;
        var resShape;
        var offset;
        function toNestedArrayHelper(source, sourceShape, d) {
            // This helper function relies on an external variable 'offset'. I
            // do not make it as an argument intentionally, though it looks a
            // little ugly.
            // When this function is called, it creates a nested array whose
            // length of the top level is indicated by 'sourceShape[d]'. The
            // elements of the new created array are initialized with the values
            // stored in 'source' starting from 'offset'.
            var elements = new Array(sourceShape[d]);
            for (var i = 0; i < elements.length; i++) {
                elements[i] = (d === sourceShape.length - 1) ? source[offset++] : toNestedArrayHelper(source, sourceShape, d + 1);
            }
            return elements;
        }
        if (resultMem.mem && (resultMem.mem instanceof InterfaceData)) {
            // single result
            res = resultMem.mem.getValue(); // convert the result to a typed array
            resShape = resultMem.shape;
            offset = 0;
            result = toNestedArrayHelper(res, resShape, 0);
        } else {
            // multiple results
            result = new Array(iterSpace[0]);
            for (var i = 0; i < result.length; i++) {
                result[i] = {};
            }
            for (var name in resultMem) {
                res = resultMem[name].mem.getValue(); // convert the result to a typed array
                resShape = resultMem[name].shape;
                offset = 0;
                if (resShape.length === 1) {
                    // the property is a scalar
                    for (var i = 0; i < result.length; i++) {
                        result[i][name] = res[i];
                    }
                } else {
                    // the property is an array (possibly nested)
                    for (var i = 0; i < result.length; i++) {
                        result[i][name] = toNestedArrayHelper(res, resShape, 1);
                    }
                }
            }
        }

        return result;
    };

    // Given the shape of an array return the number of elements.
    var shapeToLength = function shapeToLength(shape) {
        var i;
        var result;
        if (shape.length == 0) {
            return 0;
        }
        result = shape[0];
        for (i = 1; i < shape.length; i++) {
            result = result * shape[i];
        }
        return result;
    };

    return runOCL;
} ();
