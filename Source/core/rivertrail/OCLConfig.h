/*
 * Copyright (c) 2013, Intel Corporation
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

#ifndef OCLConfig_h
#define OCLConfig_h

#define INTERFACE_VERSION           2   /* running version number of the interface */
#define NUMBER_OF_ARTIFICIAL_ARGS   1   /* number of internal kernel arguments used by runtime */

#define CLPROFILE               /* enable cl profiling support */
#undef CLPROFILE_ASYNC          /* use event callbacks for profiling */
#undef OUTOFORDERQUEUE          /* enable out of order execution. Needs to be off on certain platforms. */
#undef USE_LOCAL_WORKSIZE       /* whether the tile argument is passed down to opencl */
#define BUILDLOG_MAX 1024       /* size of buildlog buffer */
#define DEBUG_OFF               /* disable debugging code */
#undef WINDOWS_ROUNDTRIP        /* enable code to measure rounttrip time of kernel run using windows API */
#undef PREALLOCATE_IN_JS_HEAP   /* allocate buffers in the JS heap and use CL_MEM_USE_HOST_POINTER */
#undef SUPPORT_MAPPING_ARRAYS   /* allow dense arrays to be mapped to JavaScript */
#define INCREMENTAL_MEM_RELEASE /* defer free of memObjs to disrtibute GC time */
#define DIRECT_WRITE            /* support helper function for fast write to canvas */

#define INITIAL_BUILDLOG_SIZE   256                          /* initial size for buildlog */
#define MAX_BUILDLOG_SIZE       (INITIAL_BUILDLOG_SIZE << 8) /* maxium size of buildlog in bytes */

#define DPO_PREFERENCE_BRANCH "extensions.dpointerface."    /* preference branch to use */
#define DPO_DEFAULT_PLATFORM_PREFNAME "defaultPlatform"     /* preference name for default platform */

#define DEFER_LIST_LENGTH 4096  /* maximum number of in flight buffers */
#define DEFER_CHUNK_SIZE 20     /* granularity of free operation */

#if defined(WTF_OS_WINDOWS)
#define RivertrailExport   __declspec( dllexport )
#else
#define RivertrailExport   
#endif

#endif /* OCLConfig_h */
