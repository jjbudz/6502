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

#include "ticker.h"

#include <assert.h>
#include <Winsock2.h>

#pragma comment(lib, "ws2_32.lib")

const unsigned int kNanoSeconds = 1000000000;

static unsigned int rate = 0;

int ticker_init(unsigned int rateMhz)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(1, 1);

    err = WSAStartup(wVersionRequested, &wsaData);

    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
//        printf("WSAStartup failed with error: %d\n", err);
        return err;
    }

    rate = (kNanoSeconds / rateMhz);

    return 0;
}

int ticker_wait(unsigned int cycles)
{
  struct timeval stDelay;

  stDelay.tv_sec = 5;
  stDelay.tv_usec = cycles * rate;

  // @fixme select needs at least one FD set to be non-null
  return select(0,0,0,0,&stDelay);
}

int ticker_cleanup()
{
    return WSACleanup();
}
