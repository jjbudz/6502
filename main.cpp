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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>  
#include <ctype.h>

#include "platform.h"
#include "l6502.h"
#include "ftrace.h"
#include "util.h"

/**
 * Main program
 */
int main(int argc, char** argv)
{     
    ftrace_init(); 

    int nStatus = 0;
    char chOption;
    char* pchSource = 0;
    char* pchLoad = 0;
    char* pchSave = 0;
    uint16_t address = 0x4000;
    uint16_t address2 = 0x0;
    uint8_t value = 0x00;
    unsigned int clockRate = 1000000; // Default 1MHz (1,000,000 Hz)
    bool bRun = false;
    bool bRunFromVector = false; // -r with no address: start at the reset vector
    bool bListing = false;
    bool bDebug = false;
    bool bDumpRegisters = false;
    bool bDumpFlags = false;
    bool bDumpStack = false;
    bool bDumpMemory = false;
    bool bPrintVersion = false;
    bool bPrintInsts = false;
    bool bAssert = false;
    bool bHelp = true;
    
    // Long options
    static struct option long_options[] = {
        {"rate", required_argument, 0, 0},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    while ((chOption = getopt_long(argc, argv, "l:c:s:r::tp::a:vd:hiL", long_options, &option_index)) != -1)
    {
        bHelp = false;
        switch (chOption)
        {
        case 0:
            // Long option
            if (strcmp(long_options[option_index].name, "rate") == 0)
            {
                clockRate = (unsigned int)atoi(optarg);
                if (clockRate == 0)
                {
                    fprintf(stderr, "Warning: invalid clock rate specified, using default 1MHz (1000000 Hz)\n");
                    clockRate = 1000000;
                }
            }
            break;
        case 'r':
            //
            // The address is optional. getopt only attaches an optional
            // argument written as -r4000, so also accept a following
            // hex word (-r 4000); with neither, start at the reset vector.
            //
            bRun = true;
            if (optarg)
            {
                address = (uint16_t)getHex(uppercase(optarg));
            }
            else if (optind < argc && argv[optind][0] != '-' &&
                     strspn(argv[optind], "0123456789abcdefABCDEF") == strlen(argv[optind]))
            {
                address = (uint16_t)getHex(uppercase(argv[optind]));
                optind++;
            }
            else
            {
                bRunFromVector = true;
            }
            break;
        case 'L':
            bListing = true;
            break;
        case 'c':
            pchSource = strdup(optarg);
            break;
        case 'l':
            pchLoad = strdup(optarg);
            break;
        case 's':
            pchSave = strdup(optarg);
            break;
        case 't':
            FTRACE_ON();
            break;
        case 'p':
            //
            // Print/dump params are optional.
            //
            if (optarg) 
            {
                uppercase(optarg);
                bDumpRegisters = (strchr(optarg, 'R') != NULL);
                bDumpFlags = (strchr(optarg, 'F') != NULL);
                bDumpStack = (strchr(optarg, 'S') != NULL);
                bDumpMemory = (strchr(optarg, 'M') != NULL);
            }
            else
            {
                bDumpRegisters = true;
                bDumpFlags = true;
                bDumpMemory = true;
            }
            break;
        case 'v':
            bPrintVersion = true;
            break;            
        case 'i':
            bPrintInsts = true;
            break;            
        case 'd':
            address = (uint16_t)getHex(uppercase(optarg)); 
            bDebug = true;
            break;
        case 'a':
            {
                char* delim = strchr(optarg, ':');
                if (delim)
                {
                    *delim = '\0';
                    delim++;
                    address2 = (uint16_t)getHex(uppercase(optarg)); 
                    value = (uint8_t)getHex(uppercase(delim)); 
                }
                else
                {
                    fprintf(stderr, "Warning: assert parameters malformed\n");
                }
            }
            bAssert = true;
            break;
        case 'h':
        default:
            goto usage;
        }
    }

    if (bHelp == true)
    {
        goto usage;
    }

    if ((nStatus = initialize(clockRate)) != 0) 
    {
        fprintf(stderr, "Error: initialization failed with error %d\n", nStatus);
        exit(nStatus);
    }

    if (bPrintVersion) 
    {
        printVersion();
    }

    if (bPrintInsts) 
    {
        printInstructions();
    }

    if (pchSource && pchLoad)
    {
        fprintf(stderr, "Warning: both -a and -l specified, will ignore load flag\n");
    }

    if (pchSource)
    {
        nStatus = assemble(pchSource, bListing ? stdout : NULL);

        if (nStatus == 0 && pchSave) 
        {
            nStatus = save(pchSave); // @todo logged failed save
        }
    }
    else if (pchLoad)
    {
        nStatus = load(pchLoad); // @todo log failed load
    }

    if (bRun && bDebug)
    {
        fprintf(stderr, "Warning: both -r and -d specified, will ignore debug flag\n");
    }

    if (nStatus == 0 && bRunFromVector)
    {
        address = resetVector();
        if (address == 0)
        {
            fprintf(stderr, "Warning: reset vector at $FFFC is $0000; "
                            "set it with '* = $FFFC' and '.WORD start', or pass -r <address>\n");
        }
    }

    if (nStatus == 0)
    {
        if (bRun)
        {
            nStatus = run(address); // @todo log failed run
        }
        else if (bDebug)
        {
            nStatus = debug(address); // @todo log failed debug
        }
    }

    if (nStatus) perror("Error"); // @todo this is kinda stupid and should use custom error strings

    if (bDumpRegisters || bDumpFlags || bDumpStack || bDumpMemory) 
    {
        dump(bDumpRegisters, bDumpFlags, bDumpStack, bDumpMemory);
    }

    if (bAssert)
    {
        bool bPassed = assertmem(address2, value);
        fprintf(stderr, "Assert $%04x:%02x=%02x %s\n", address2, value,inspect(address2), (bPassed?"true":"false"));
        if (!bPassed) nStatus = 1; // an earlier failure (e.g. assembly) is not masked by a passing assert
    }

    cleanup();
    ftrace_cleanup();

    exit(nStatus);

    return nStatus;

usage:

    printf("Usage: -l <filename> -c <filename> -s <filename> -r [<address>] [-L] [-t] [-p] where:\n");
    printf("\t-h to display command line options\n");
    printf("\t-l <filename> to load an object file\n");
    printf("\t-c <filename> to compile source file\n");
    printf("\t-L to print an assembly listing (address, bytes, source) after -c\n");
    printf("\t-s <filename> to save object file after assembly\n");
    printf("\t-r [<address>] to run code from the address (hexadecimal, e.g. A000);\n");
    printf("\t   with no address, run from the reset vector at $FFFC\n");
    printf("\t-d <address> to debug code from the address (hexadecimal, e.g. A000)\n");
    printf("\t-a <address>:<value> to assert value matches at the given address\n");
    printf("\t-t to turn on trace output\n");
    printf("\t-i to list assembler instructions\n");
    printf("\t-p[rfsm] to print (dump) registers, flags, stack, and memory on exit\n");
    printf("\t-v to print version information\n");
    printf("\t--rate <hz> to set CPU clock rate in Hz (default: 1000000)\n");

    exit(0);
    return 0;

}

