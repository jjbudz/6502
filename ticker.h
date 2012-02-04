#ifndef _TICKER_H_
#define _TICKER_H_

int ticker_init(unsigned int UiRateMhz); // Mhz
int ticker_wait(unsigned int uiCycles);
int ticker_cleanup();

#endif

