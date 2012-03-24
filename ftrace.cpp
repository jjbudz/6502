//
// Copyright (c) 1998-2011 Jeff Budzinski
//
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
// DEALINGS IN THE SOFTWARE.
//
// Author: Jeff Budzinski
//
// Purpose: 
//
//   Implementation of the trace facility/logger
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef WIN32
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "ftrace.h"

#ifndef FNOTRACE

bool g_bTrace = false;

static FILE* s_pTraceFile = 0;

void ftrace_init(const char* const pchFilename)
{
    s_pTraceFile = (pchFilename!=0)?fopen(pchFilename,"a"):stderr;		
}

void ftrace(const char* const pchFormat, ...)
{
    const int kMaxLen = 1024;
    const char* const pchFormatPrefix = "%s:%u - ";
    char pchFormatPlus[kMaxLen+1];
    va_list args;

    assert(s_pTraceFile);
    assert(pchFormat);

    if (strlen(pchFormat) < kMaxLen - strlen(pchFormatPrefix))
    {
        strcpy(pchFormatPlus,pchFormatPrefix);
        strcat(pchFormatPlus,pchFormat);
        strcat(pchFormatPlus,"\n");
        va_start(args,pchFormat);
        vfprintf(s_pTraceFile,pchFormatPlus,args);
        fflush(s_pTraceFile);
        va_end(args);
    }
}

void ftrace_cleanup()
{
    if (g_bTrace && s_pTraceFile != stderr) fclose(s_pTraceFile);
}

#endif
