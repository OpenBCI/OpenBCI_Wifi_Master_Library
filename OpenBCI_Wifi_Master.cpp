/***************************************************
This is a library for the OpenBCI Cyton and Ganglion

OpenBCI and Push The World invest time and resources providing this open source code,
please support OpenBCI and open-source hardware by purchasing
products from OpenBCI or donating on our downloads page!

Written by AJ Keller of Push The World LLC

MIT license
****************************************************/

#include "OpenBCI_Wifi_Master.h"

// CONSTRUCTOR
OpenBCI_Wifi_Master_Class::OpenBCI_Wifi_Master_Class() {
  // Set defaults
  debug = true;
  present = false;
  rx = false;
  seekingWifi = false;
  soughtWifiShield = false;
  toggleWifiCS = false;
  toggleWifiReset = false;
  tx = false;

  attachAttempts = 0;
  timeOfWifiToggle = 0;
  timeOfWifiStart = 0;
  bufferTxPosition = 0;

  bufferReadFromClear();
  bufferTxClear();

}

/**
* @description The function that the radio will call in setup()
* @param: mode {unint8_t} - The mode the radio shall operate in
* @author AJ Keller (@pushtheworldllc)
*/
boolean OpenBCI_Wifi_Master_Class::begin(void) {
  begin(true, true);
}

/**
* @description The function that the radio will call in setup()
* @param: mode {unint8_t} - The mode the radio shall operate in
* @author AJ Keller (@pushtheworldllc)
*/
boolean OpenBCI_Wifi_Master_Class::begin(boolean _rx, boolean _tx) {

  // if (debug) {
  //   Serial0.print("begin: Wifi rx: ");
  //   Serial0.print(_rx);
  //   Serial0.print(" tx: ");
  //   Serial0.println(_tx);
  // }

  rx = _rx;
  tx = _tx;

  pinMode(WIFI_SS, INPUT);
  pinMode(WIFI_RESET,OUTPUT); digitalWrite(WIFI_RESET, LOW);

  reset();
}

/**
 * Used to attach a wifi shield, only if there is actuall a wifi shield present
 */
boolean OpenBCI_Wifi_Master_Class::attach(void) {
  present = smell();
  if(!present) {
    rx = false;
    tx = false;
    return false;
    // if(!isRunning) Serial0.print("no wifi shield to attach!"); sendEOT();
  } else {
    rx = true;
    tx = true;
    return true;
    // if(!isRunning) Serial0.println("wifi attached"); sendEOT();
  }
}

/**
 * Clear the wifi tx buffer
 */
void OpenBCI_Wifi_Master_Class::bufferRxClear(void) {
  for (uint8_t i = 0; i < WIFI_SPI_MAX_PACKET_SIZE; i++) {
    bufferRx[i] = 0;
  }
}

/**
 * Clear the wifi tx buffer
 */
void OpenBCI_Wifi_Master_Class::bufferTxClear(void) {
  for (uint8_t i = 0; i < WIFI_SPI_MAX_PACKET_SIZE; i++) {
    bufferTx[i] = 0;
  }
  bufferTxPosition = 0;
}

/**
 * Clear the wifi tx buffer
 */
void OpenBCI_Wifi_Master_Class::bufferReadFromClear(void) {
  for (uint8_t i = 0; i < WIFI_SPI_MAX_PACKET_SIZE; i++) {
    bufferReadFrom[i] = 0;
  }
}

//SPI chip select method
void OpenBCI_Wifi_Master_Class::csLow() {
#if defined(__PIC32MX2XX__)
  spi.setMode(DSPI_MODE0);
  spi.setSpeed(10000000);
#endif
  digitalWrite(WIFI_SS, LOW);
}

void OpenBCI_Wifi_Master_Class::csHigh() {
  digitalWrite(WIFI_SS, HIGH);

  // DEFAULT TO SD MODE!
#if defined(__PIC32MX2XX__)
  spi.setMode(DSPI_MODE0);
#endif
}

/**
 * Flush the 32 byte buffer to the wifi shield. Set byte id too...
 */
void OpenBCI_Wifi_Master_Class::flushBufferTx() {
  writeData(bufferTx, WIFI_SPI_MAX_PACKET_SIZE);
  bufferTxPosition = 0;
}

/**
 * Used to read a char from the wifi recieve buffer
 * @return  [char] - A single char from the RX buffer
 */
char OpenBCI_Wifi_Master_Class::getChar(void) {
  uint8_t numChars = bufferReadFrom[0];
  char output = bufferReadFrom[1];
  if (numChars == 1) {
    bufferReadFrom[0] = 0;
    bufferReadFrom[1] = 0;
  } else if (numChars > 1) {
    bufferReadFrom[0] = numChars - 1;
    for (uint8_t i = 1; i < numChars; i++) {
      bufferReadFrom[i] = bufferReadFrom[i+1];
    }
    bufferReadFrom[numChars] = 0;
  }
  return output;
}

boolean OpenBCI_Wifi_Master_Class::hasData(void) {
  return bufferReadFrom[0] > 0;
}

/**
 * OpenBCI_32bit_Library::loop used to run internal library looping functions
 *  mainly used for timers and such.
 */
void OpenBCI_Wifi_Master_Class::loop(void) {
  if (toggleWifiReset) {
    if (millis() > timeOfWifiToggle + 500) {
      digitalWrite(WIFI_RESET, HIGH);
      toggleWifiReset = false;
      toggleWifiCS = true;
    }
  }
  if (toggleWifiCS) {
    if (millis() > timeOfWifiToggle + 3000) {
      // digitalWrite(OPENBCI_PIN_LED, HIGH);
      pinMode(WIFI_SS, OUTPUT);
      digitalWrite(WIFI_SS, HIGH); // Set back to high
      toggleWifiCS = false;
      timeOfWifiToggle = millis();
      seekingWifi = true;
    }
  }
  if (seekingWifi) {
    if (millis() > timeOfWifiToggle + 4000) {
      seekingWifi = false;
      if (!attach()) {
        attachAttempts++;
        if (attachAttempts < 10) {
          seekingWifi = true;
          timeOfWifiToggle = millis();
        }
      }
    }
  }
  if (rx) {
    if (millis() > timeOfLastRead + 20) {
      readData();
      uint8_t numChars = (uint8_t)bufferRx[0];
      if (numChars > 0 && numChars < WIFI_SPI_MAX_PACKET_SIZE) {
        // Copy to the read from buffer
        memcpy(bufferReadFrom, bufferRx, WIFI_SPI_MAX_PACKET_SIZE);
        // Clear the rx recieve buffer
        bufferRxClear();
      }
      timeOfLastRead = millis();
    }
  }
}

/**
 * [OpenBCI_Wifi_Master_Class::storeByteBufTx description]
 * @param  b {uint8_t} A single byte to store
 * @return   {boolean} True if the byte was stored, false if the buffer is full.
 */
boolean OpenBCI_Wifi_Master_Class::storeByteBufTx(uint8_t b) {
  if (bufferTxPosition >= WIFI_SPI_MAX_PACKET_SIZE) return false;
  bufferTx[bufferTxPosition] = b;
  bufferTxPosition++;
  return true;
}

/**
 * Used to read data into the wifi input buffer
 */
void OpenBCI_Wifi_Master_Class::readData() {
  csLow();
  xfer(0x03);
  xfer(0x00);
  for(uint8_t i = 0; i < 32; i++) {
    bufferRx[i] = xfer(0);
  }
  csHigh();
}

/**
 * Used to read the status register from the ESP8266 wifi shield
 * @return uint32_t the status
 */
uint32_t OpenBCI_Wifi_Master_Class::readStatus(void){
  csLow();
  xfer(0x04);
  uint32_t status = (xfer(0x00) | ((uint32_t)(xfer(0x00)) << 8) | ((uint32_t)(xfer(0x00)) << 16) | ((uint32_t)(xfer(0x00)) << 24));
  csHigh();
  return status;
}

/**
 * Used to detach the wifi shield, sort of.
 */
boolean OpenBCI_Wifi_Master_Class::remove(void) {
  if (present) {
    rx = false;
    tx = false;
    present = false;
    return true;
  }
  return false;
}

/**
 * Used to power on reset the ESP8266 wifi shield. Used in conjunction with `.loop()`
 */
void OpenBCI_Wifi_Master_Class::reset(void) {
  // Always keep pin low or else esp will fail to boot.
  // See https://github.com/esp8266/Arduino/blob/master/libraries/SPISlave/examples/SPISlave_SafeMaster/SPISlave_SafeMaster.ino#L12-L15
  pinMode(WIFI_SS, INPUT);
  digitalWrite(WIFI_RESET, LOW); // Reset the ESP8266
// #ifdef CYTON
//   digitalWrite(OPENBCI_PIN_LED, LOW); // Good visual indicator of what's going on
// #endif
  rx = false;
  tx = false;
  present = false;
  toggleWifiReset = true;
  timeOfWifiToggle = millis();
}

void OpenBCI_Wifi_Master_Class::sendGains(uint8_t numChannels, uint8_t *gains) {
  if (!present) return;
  if (!tx) return;

  // Clear the wifi buffer
  bufferTxClear();

  storeByteBufTx(WIFI_SPI_MSG_GAINS);
  storeByteBufTx(WIFI_SPI_MSG_GAINS); // Redundancy
  storeByteBufTx(numChannels);
  for (uint8_t i = 0; i < numChannels; i++) {
    storeByteBufTx(gains[i]);
  }
  flushBufferTx();
}

/**
 * This will tell the Wifi shield to send the contents of this message to the requesting
 *  client, if there was one... if this functions sister function, sendStringMulti was used
 *  then this will indicate the end of a multi byte message.
 * @param str [description]
 */
void OpenBCI_Wifi_Master_Class::sendStringLast(void) {
  sendStringLast("");
}

/**
 * This will tell the Wifi shield to send the contents of this message to the requesting
 *  client, if there was one... if this functions sister function, sendStringMulti was used
 *  then this will indicate the end of a multi byte message.
 * @param str [description]
 */
void OpenBCI_Wifi_Master_Class::sendStringLast(const char *str) {
  if (!present) return;
  if (!tx) return;
  if (str == NULL) return;
  int len = strlen(str);
  if (len > WIFI_SPI_MAX_PACKET_SIZE - 1) {
    return; // Don't send more than 31 bytes at a time -o-o- (deal with it)
  }
  bufferTxClear();
  storeByteBufTx(WIFI_SPI_MSG_LAST);
  for (int i = 0; i < len; i++) {
    storeByteBufTx(str[i]);
  }
  flushBufferTx();
  bufferTxClear();
}

/**
 * Will send a const char string (less than 32 bytes) to the wifi shield, call
 *  sendStringLast to indicate to the wifi shield the multi part transmission is over.
 * @param str const char * less than 32 bytes to be sent over SPI
 */
void OpenBCI_Wifi_Master_Class::sendStringMulti(const char *str) {
  if (!present) return;
  if (!tx) return;
  if (str == NULL) return;
  int len = strlen(str);
  if (len > WIFI_SPI_MAX_PACKET_SIZE - 1) {
    return; // Don't send more than 31 bytes at a time -o-o- (deal with it)
  }

  bufferTxClear();
  storeByteBufTx(WIFI_SPI_MSG_MULTI);
  for (int i = 0; i < len; i++) {
    storeByteBufTx(str[i]);
  }
  flushBufferTx();
  bufferTxClear();
}

/**
 * Used to check and see if the wifi is present
 * @return  [description]
 */
boolean OpenBCI_Wifi_Master_Class::smell(void){
  boolean isWifi = false;
  uint32_t uuid = readStatus();

#ifdef DEBUG
  USE_SERIAL.print("Wifi ID 0x");
  USE_SERIAL.println(uuid,HEX);
  USE_SERIAL.println(micros());
#endif
  // }

  if(uuid == 209) {isWifi = true;} // should read as 0x3E
  return isWifi;
}

/**
 * Used to write data out
 * @param data [description]
 * @param len  [description]
 */
void OpenBCI_Wifi_Master_Class::writeData(uint8_t * data, size_t len) {
  uint8_t i = 0;
  byte b = 0;
  csLow();
  xfer(0x02);
  xfer(0x00);
  while(len-- && i < WIFI_SPI_MAX_PACKET_SIZE) {
    xfer(data[i++]);
  }
  while(i++ < WIFI_SPI_MAX_PACKET_SIZE) {
    xfer(0); // Pad with zeros till 32
  }
  csHigh();
}

//SPI communication method
byte OpenBCI_Wifi_Master_Class::xfer(byte _data) {
  byte inByte;
#if defined(__PIC32MX2XX__)
  inByte = spi.transfer(_data);
#else
  inByte = SPI.transfer(_data);
#endif
  return inByte;
}

OpenBCI_Wifi_Master_Class wifi;
