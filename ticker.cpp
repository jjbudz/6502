/**
 * @section copyright_sec Copyright and License
 *
 * Copyright (c) 2011-2012 Jeff Budzinski
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
 *   Implementation of cycle timing simulation.
 *
 */

#include <assert.h>
#include <stdio.h>

#include "ticker.h"

const unsigned int kNanoSeconds = 1000000000;

static unsigned int rate = 0;

#ifdef WIN32

#include <Winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int ticker_init(unsigned int rateMhz)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);
    rate = (kNanoSeconds / rateMhz);

    return (err == SOCKET_ERROR) ? WSAGetLastError() : 0;    
}

int ticker_wait(unsigned int cycles)
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    static fd_set fds;
    struct timeval stDelay;
    int err = 0;

    // rate = nanoseconds per MHz = nanoseconds per 1,000,000 cycles
    // So: microseconds = (cycles * nanoseconds per 1,000,000 cycles) / (1,000 nanoseconds per microsecond * 1,000,000)
    //                  = (cycles * rate) / 1,000,000,000
    unsigned long long micros = ((unsigned long long)cycles * rate) / 1000000000;
    
    FD_ZERO(&fds);
    FD_SET(s, &fds);
    stDelay.tv_sec = micros / 1000000;
    stDelay.tv_usec = micros % 1000000;
    err = select(0, &fds, NULL, NULL, &stDelay);
    err |= closesocket(s);

    return (err == SOCKET_ERROR) ? WSAGetLastError() : 0;
}

int ticker_cleanup()
{
    return (WSACleanup() == SOCKET_ERROR) ? WSAGetLastError() : 0;    
}

#else

#include <time.h>

int ticker_init(unsigned int rateMhz)
{
    rate = (kNanoSeconds / rateMhz);
    return 0;
}

int ticker_wait(unsigned int cycles)
{
    struct timespec ts;
    // rate = nanoseconds per MHz = nanoseconds per 1,000,000 cycles
    // So: nanoseconds = cycles * (nanoseconds per 1,000,000 cycles) / 1,000,000
    unsigned long long nanos = ((unsigned long long)cycles * rate) / 1000000;
    
    ts.tv_sec = nanos / kNanoSeconds;
    ts.tv_nsec = nanos % kNanoSeconds;
    
    nanosleep(&ts, NULL);
    return 0;
}

int ticker_cleanup()
{
    return 0;
}

#endif

