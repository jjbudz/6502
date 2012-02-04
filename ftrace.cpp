///////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000-2012 Jeff Budzinski
//
///////////////////////////////////////////////////////////////////////
//
// Filename/version:
//
//   $Workfile: ofi_trace.cpp $ 
//   $Revision: 6 $
//
// Description:
//
//   Implementation of the trace facility.
//
// $History: ofi_trace.cpp $
// 
// *****************  Version 6  *****************
// User: Jeff         Date: 1/06/03    Time: 1:36p
// Updated in $/ofi/src/util
// Commented as part of major cleanup effort.
// 
// *****************  Version 5  *****************
// User: Jeff         Date: 12/13/02   Time: 8:50a
// Updated in $/ofi/src/util
// Ported to WIN32, minor cleanup.
// 
// *****************  Version 4  *****************
// User: Jeff         Date: 10/18/02   Time: 8:46p
// Updated in $/ofi/src/util
// Added macro to control removal of tracing code during compilation.
// 
// *****************  Version 3  *****************
// User: Jeff         Date: 9/13/02    Time: 6:15p
// Updated in $/ofi/src/util
// Corrected #ifdef statement.
// 
// *****************  Version 2  *****************
// User: Jeff         Date: 9/02/02    Time: 3:38p
// Updated in $/ofi/src/util
// Added cross platform #defines to support compilation on Linux.
// 
// *****************  Version 1  *****************
// User: Jeff         Date: 1/20/00    Time: 9:07p
// Created in $/src/util
// created
//
///////////////////////////////////////////////////////////////////////

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
