#ifndef _FTRACE_H_
#define _FTRACE_H_
///////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000 Jeff Budzinski
//
///////////////////////////////////////////////////////////////////////
//
// Filename/version:
//
//   $Workfile: ofi_trace.h $ 
//   $Revision: 3 $
//
// Description
//
//   Interface to a rudimentary program tracing facility. Enable 
//   tracing by setting the environment variable FTRACE to nonzero.
//   When tracing is enabled, trace output is written to the
//   file <process name>.<process id>.
//
//   Tracing can be safely left in release code; however, if desired,
//   tracing may be completely disabled by defining NOFTRACE
//   during compilation.
//
// Notes

//   It would be useful at some point to provide module-specific
//   trace enabling through the use of additional parameters to
//   the FTRACE macro and FTRACE enviromental variable.
//
//   The FTRACE macro uses vfprintf for output formatting.
//   Insufficient and/or improper argument types may cause run-time
//   errors (core dumps/crashes) when tracing is enabled. Therefore, 
//   tracing statements should be tested before being placed in 
//   production code.
//
//   The maximum trace string is approximately 1K. See trace.cpp
//   for details.
//
// $History: ofi_trace.h $
// 
// *****************  Version 3  *****************
// User: Jeff         Date: 1/06/03    Time: 1:37p
// Updated in $/ofi/include
// Commented as part of major documentation effort.
// 
// *****************  Version 2  *****************
// User: Jeff         Date: 10/18/02   Time: 8:44p
// Updated in $/ofi/include
// Added macro to optionally deactivated tracing code.
// 
// *****************  Version 1  *****************
// User: Jeff         Date: 1/20/00    Time: 8:56p
// Created in $/include
// created
//
///////////////////////////////////////////////////////////////////////

#ifdef NOFTRACE
#define FTRACE(args)
#define ftrace_init(args)
#define ftrace_deinit(args)
#else
//
// Use this macro to create a trace output statement that is
// conditionally executed. The macros __FILE__ and __LINE__ 
// should be part of the statement as follows:
//
//   FTRACE("<printf format specification>",__FILE__,__LINE__,...);
//
#define FTRACE(str,...) if (g_bTrace) ftrace(str,__VA_ARGS__)

//
// Convenience macros to selectively turning tracing on/off
//
#define FTRACE_ON() g_bTrace = true
#define FTRACE_OFF() g_bTrace = false

//
// Controls enabling of trace facility
//
extern bool g_bTrace;

//
// Initializes trace facility by setting global
// and opening trace file.
//
void ftrace_init(const char* const pchFilename=0);

//
// This function performs the trace output. It should not
// be called directly. Always use the FTRACE macro for
// conditional output.
//
void ftrace(const char* const pchFormat, ...);

//
// Cleans up trace facility by closing any open file.
//
void ftrace_cleanup();

#endif

#endif
