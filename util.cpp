/**
 * @section copyright_sec Copyright and License
 *
 * Copyright (c) 1998-2011 Jeff Budzinski
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
 */

#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "util.h"
#include "platform.h"

/**
 * Uppercase a character string.
 */
char* uppercase(char* str)
{
    assert(str);

    for (unsigned int i=0; i < strlen(str); i++) str[i] = toupper(str[i]);

    return str;
}

/** 
 * Turn a hexadecimal string into a numeric value.
 */
uint16_t getHex(const char* str)
{
    uint16_t value = 0;

    assert(str);

    for (unsigned int i=0; i < strlen(str); i++)
    {
        value *= i?16:1;
        value += (str[i]>='A')?(10+str[i]-'A'):(str[i]-'0');
    }

    return value;
}

