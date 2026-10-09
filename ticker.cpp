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
 *   Sleeping after every instruction cannot hold a realistic clock rate: at
 *   1 MHz an instruction lasts 2-7 microseconds, well below what nanosleep()
 *   reliably delivers, and every oversleep adds up. Instead the ticker counts
 *   cycles against a wall-clock start time and, once about a millisecond's
 *   worth of cycles has run, sleeps until the moment those cycles should have
 *   finished. An oversleep in one batch is made up in the next, so the
 *   average rate holds without drift.
 *
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "ticker.h"

static const uint64_t kNanoSeconds = 1000000000ULL;

// Emulated time to run between sleeps, and so the granularity of the throttle.
static const uint64_t kBatchNanos = 1000000ULL; // 1 ms

// Falling further behind than this (the debugger waiting for input, the
// process being suspended) restarts the schedule rather than running
// unthrottled until the emulator catches up.
static const uint64_t kMaxLagNanos = 100000000ULL; // 100 ms

static unsigned int rate = 0;       // Hz; 0 means run unthrottled
static uint64_t batchCycles = 1;    // cycles per batch at this rate
static uint64_t startNanos = 0;     // wall clock when the schedule began
static uint64_t elapsedCycles = 0;  // cycles run since startNanos
static uint64_t pendingCycles = 0;  // cycles run since the last sleep

static uint64_t nowNanos()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * kNanoSeconds + (uint64_t)ts.tv_nsec;
}

static void sleepNanos(uint64_t nanos)
{
    struct timespec ts;
    ts.tv_sec = nanos / kNanoSeconds;
    ts.tv_nsec = nanos % kNanoSeconds;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
        ;
}

/*
 * Sleep until the wall clock catches up with the cycles run so far.
 */
static void catchUp()
{
    pendingCycles = 0;

    // Split the division so elapsedCycles * kNanoSeconds cannot overflow.
    uint64_t target = startNanos
        + (elapsedCycles / rate) * kNanoSeconds
        + (elapsedCycles % rate) * kNanoSeconds / rate;
    uint64_t now = nowNanos();

    if (target > now)
    {
        sleepNanos(target - now);
    }
    else if (now - target > kMaxLagNanos)
    {
        ticker_reset();
    }
}

int ticker_init(unsigned int rateHz)
{
    rate = rateHz;
    batchCycles = (uint64_t)rate * kBatchNanos / kNanoSeconds;
    if (batchCycles == 0) batchCycles = 1;
    ticker_reset();
    return 0;
}

void ticker_reset()
{
    startNanos = nowNanos();
    elapsedCycles = 0;
    pendingCycles = 0;
}

int ticker_wait(unsigned int cycles)
{
    if (rate == 0) return 0;

    elapsedCycles += cycles;
    pendingCycles += cycles;
    if (pendingCycles >= batchCycles) catchUp();
    return 0;
}

int ticker_flush()
{
    if (rate != 0 && pendingCycles != 0) catchUp();
    return 0;
}

int ticker_cleanup()
{
    return 0;
}
