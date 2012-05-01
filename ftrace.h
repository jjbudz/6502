#ifndef _FTRACE_H_
#define _FTRACE_H_
/**
 * @section copyright_sec Copyright and License
 *
 * Copyright (c) 2002-2012 Jeff Budzinski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * Author: Jeff Budzinski
 *
 * Purpose: 
 *
 *   Interface to a rudimentary program tracing facility. Enable 
 *   tracing by setting the environment variable FTRACE to nonzero.
 *   When tracing is enabled, trace output is written to the
 *   file <process name>.<process id>.
 *
 *   Tracing can be safely left in release code; however, if desired,
 *   tracing may be completely disabled by defining NOFTRACE
 *   during compilation.
 *
 * Notes:
 *
 *   It would be useful at some point to provide module-specific
 *   trace enabling through the use of additional parameters to
 *   the FTRACE macro and FTRACE enviromental variable.
 *
 *   The FTRACE macro uses vfprintf for output formatting.
 *   Insufficient and/or improper argument types may cause run-time
 *   errors (core dumps/crashes) when tracing is enabled. Therefore, 
 *   tracing statements should be tested before being placed in 
 *   production code.
 *
 *   The maximum trace string is approximately 1K. 
 *
 */

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
