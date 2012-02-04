
#include "ticker.h"

#include <assert.h>
#include <Winsock2.h>

#pragma comment(lib, "ws2_32.lib")

const unsigned int kNanoSeconds = 1000000000;

static unsigned int s_uiRate = 0;

int ticker_init(unsigned int uiRateMhz)
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

    s_uiRate = (kNanoSeconds / uiRateMhz);

    return 0;
}

int ticker_wait(unsigned int cycles)
{
  struct timeval stDelay;

  stDelay.tv_sec = 5;
  stDelay.tv_usec = cycles * s_uiRate;

  // @fixme select needs at least one FD set to be non-null
  return select(0,0,0,0,&stDelay);
}

int ticker_cleanup()
{
    return WSACleanup();
}
