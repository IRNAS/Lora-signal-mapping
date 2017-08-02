#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(9600);                         // serial for debugging
  while (!Serial);                            // waiting for serial

  Serial.println("LoRa Receiver");            // print welcome message
  
  if (!LoRa.begin(866E6)) {                   // LoRa setup at 868MHz
    Serial.println("Starting LoRa failed!");  // failed
    while (1);                                // put some debug
  }
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();        // read the packetsize
  if (packetSize) {                           // if there is a packet
    // received a packet
    Serial.print("Received packet '");        // debug received

    // read packet
    while (LoRa.available()) {                // while there is data
      Serial.print((char)LoRa.read());        // convert to char and print it to the console
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");             // deubg for rssi
    Serial.println(LoRa.packetRssi());        // print rssi to the console
  }
}
