#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "TeensyMAC.h"

#include <OSCMessage.h>

#include <Bounce2.h>

EthernetUDP Udp;
IPAddress myIp(2,0,0,10);
IPAddress outIp(2,0,0,1);
uint16_t  myPort = 8001;
uint16_t  outPort = 8000;

#define PIN_RESET 9

#define NUM_SWITCH 7

char SWITCH_PIN[NUM_SWITCH] = { A6, 6, A7, 5, A8, 4, A9 };

Bounce debouncer[NUM_SWITCH];

void setup()
{
  for (size_t i = 0; i < NUM_SWITCH; i++) {
    pinMode(SWITCH_PIN[i], INPUT_PULLUP);
    debouncer[i] = Bounce(SWITCH_PIN[i], 10);
  }
  #ifdef PIN_RESET
    pinMode(PIN_RESET, OUTPUT);
    digitalWrite(PIN_RESET, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_RESET, HIGH);
    delay(150);
  #endif
  uint8_t mac[6] = {(teensyMAC()>>40)&0xFF, (teensyMAC()>>32)&0xFF, (teensyMAC()>>24)&0xFF, (teensyMAC()>>16)&0xFF, (teensyMAC()>>8)&0xFF, teensyMAC() & 0xFF,};
  Ethernet.begin(mac, myIp);
  Udp.begin(myPort);
} //setup

void loop() {
  // Update the Bounce instances :
  for (int i = 0; i < NUM_SWITCH; i++) {
    debouncer[i].update();
  }
  // are they in a new state
  for (int i = 0; i < NUM_SWITCH; i++) {
    if ( debouncer[i].fell() || debouncer[i].rose() ) {
      char str[9] = "/switch/";
      str[8] = 49 + i;
      OSCMessage msg(str);
      msg.add((int)debouncer[i].read());
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp); // send the bytes to the SLIP stream
      Udp.endPacket(); // mark the end of the OSC Packet
      msg.empty(); // free space occupied by message
    }
  }
}//loop
