/**
* Name: OpenBCI_Radio_Definitions.h
* Date: 3/15/2016
* Purpose: This is the header file for the OpenBCI radios definitions.
*
* Author: Push The World LLC (AJ Keller)
*   Much credit must also go to Joel Murphy who with Conor Russomanno and Leif
*     Percifield created the original OpenBCI_32bit_Device.ino and
*     OpenBCI_32bit_Host.ino files in the Summer of 2014. Much of this code base
*     is inspired directly from their work.
*/

#ifndef __OpenBCI_Wifi_Master_Definitions__
#define __OpenBCI_Wifi_Master_Definitions__

/*
* WIFI
*/
#define PCKT_END 0xC0
#define OPENBCI_SAMPLE_RATE_SET '~'
#define WIFI_SPI_CMD_NULL 0x00
#define WIFI_SPI_CMD_DATA_READ 0x03
#define WIFI_SPI_CMD_DATA_WRITE 0x02
#define WIFI_SPI_CMD_STATUS_READ 0x04
#define WIFI_SPI_CMD_STATUS_WRITE 0x01
#define WIFI_SPI_MAX_PACKET_SIZE 32
#define WIFI_SPI_MSG_LAST 0x01
#define WIFI_SPI_MSG_MULTI 0x02
#define WIFI_SPI_MSG_GAINS 0x03


#ifdef CYTON
#define WIFI_RESET 18
#define WIFI_SS 13          // Wifi Chip Select
#elif GANGLION
#define WIFI_RESET 4
#define WIFI_SS 24          // Wifi Chip Select
#endif

#endif
