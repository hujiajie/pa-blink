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

#ifndef OCLDebug_h
#define OCLDebug_h

#define RT_OK 0
#define RT_ERROR_NOT_IMPLEMENTED 0x80000001
#define RT_ERROR_NOT_AVAILABLE 0x80000002
#define RT_ERROR_OUT_OF_MEMORY 0x80000003

#ifndef DEBUG_OFF
#include "core/platform/Logging.h"
#include <sstream>

#define DEBUG_LOG_ERROR( src, code)                                                              \
{                                                                                                \
    std::stringstream debug;                                                                     \
    debug << "DEBUG @ " << __FILE__ << ":" << __LINE__ << " in " src << " ERROR CODE: " << code; \
    LOG(RiverTrail, debug.str().c_str());                                                        \
}

#define DEBUG_LOG_STATUS( src, status)                                                         \
{                                                                                              \
    std::stringstream debug;                                                                   \
    debug << "DEBUG @ " << __FILE__ << ":" << __LINE__ << " in " src << " STATUS: " << status; \
    LOG(RiverTrail, debug.str().c_str());                                                      \
}

#define DEBUG_LOG_CLERROR( text)                         \
{                                                        \
    std::stringstream debug;                             \
    debug << "DEBUG @ [OPENCLRUNTIME] STATUS: " << text; \
    LOG(RiverTrail, debug.str().c_str());                \
}

#define DEBUG_LOG_CREATE( src, ptr)                                                          \
{                                                                                            \
    std::stringstream debug;                                                                 \
    debug << "DEBUG @ " __FILE__ << ":" << __LINE__ << " CREATION " << src << " at " << ptr; \
    LOG(RiverTrail, debug.str().c_str());                                                    \
}

#define DEBUG_LOG_DESTROY( src, ptr)                                                            \
{                                                                                               \
    std::stringstream debug;                                                                    \
    debug << "DEBUG @ " __FILE__ << ":" << __LINE__ << " DESTRUCTION " << src << " at " << ptr; \
    LOG(RiverTrail, debug.str().c_str());                                                       \
}

#else // DEBUG_OFF

#define DEBUG_LOG_ERROR( src, code) 
#define DEBUG_LOG_STATUS( src, status) 
#define DEBUG_LOG_CLERROR( text) 
#define DEBUG_LOG_CREATE( src, ptr) 
#define DEBUG_LOG_DESTROY( src, ptr) 

#endif // DEBUG_OFF
#endif // OCLDebug_h
