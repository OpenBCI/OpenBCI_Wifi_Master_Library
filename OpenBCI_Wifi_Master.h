/**
* Name: OpenBCI_Radio.h
* Date: 3/15/2016
* Purpose: This is the header file for the OpenBCI radios. Let us define two
*   over arching paradigms: Host and Device, where:
*     Host is connected to PC via USB VCP (FTDI).
*     Device is connectedd to uC (PIC32MX250F128B with UDB32-MX2-DIP).
*
* Author: Push The World LLC (AJ Keller)
*   Much credit must also go to Joel Murphy who with Conor Russomanno and Leif
*     Percifield created the original OpenBCI_32bit_Device.ino and
*     OpenBCI_32bit_Host.ino files in the Summer of 2014. Much of this code base
*     is inspired directly from their work.
*/


#ifndef __OpenBCI_Wifi_Master__
#define __OpenBCI_Wifi_Master__

#include <Arduino.h>
#include "OpenBCI_Wifi_Master_Definitions.h"

#if defined(__PIC32MX2XX__)
#include <DSPI.h>
#define USE_SERIAL Serial0
#define WIFI_RESET 18
#define WIFI_SS 13
// #elif GANGLION
// #include <SPI.h>
// #define USE_SERIAL Serial
#endif

class OpenBCI_Wifi_Master_Class {

public:

  OpenBCI_Wifi_Master_Class();
  boolean attach(void);
  boolean begin(void);
  boolean begin(boolean, boolean);
  void    bufferTxClear(void);
  void    bufferRxClear(void);
  void    csHigh(void);
  void    csLow(void);
  void    flushBufferTx(void);
  char    getChar(void);
  boolean hasData(void);
  void    loop(void);
  void    readData(void);
  uint32_t readStatus(void);
  boolean remove(void);
  void    reset(void);
  void    sendGains(uint8_t, uint8_t *);
  void    sendStringMulti(const char *);
  void    sendStringLast();
  void    sendStringLast(const char *);
  boolean smell(void);
  boolean storeByteBufTx(uint8_t);
  void    writeData(uint8_t *, size_t);

  // VARIABLES

  // wifi variables
  boolean debug;
  boolean present;
  boolean rx;
  boolean tx;

  char bufferRx[WIFI_SPI_MAX_PACKET_SIZE];
  char bufferReadFrom[WIFI_SPI_MAX_PACKET_SIZE];

  uint8_t bufferTx[WIFI_SPI_MAX_PACKET_SIZE];
  uint8_t bufferTxPosition;

#if defined(__PIC32MX2XX__)
  DSPI0 spi;
#endif

private:

  byte xfer(byte _data);

  boolean toggleWifiCS;
  boolean toggleWifiReset;
  boolean soughtWifiShield;
  boolean seekingWifi;

  int     attachAttempts;

  unsigned long timeOfLastRead;
  unsigned long timeOfWifiToggle;
  unsigned long timeOfWifiStart;

};

// Very important, major key to success #christmas
extern OpenBCI_Wifi_Master_Class wifi;

#endif // OPENBCI_Wifi_H
