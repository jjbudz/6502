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
