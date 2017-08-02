#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8X8_SSD1306_128X32_UNIVISION_SW_I2C u8x8(SCL, SDA, U8X8_PIN_NONE);


void setup() {
  Serial.begin(9600);                         // serial for debugging
  while (!Serial);                            // waiting for serial

  Serial.println("LoRa Receiver");            // print welcome message
  
  if (!LoRa.begin(866E6)) {                   // LoRa setup at 868MHz
    Serial.println("Starting LoRa failed!");  // failed
    while (1);                                // put some debug
  }

  LoRa.onReceive(onReceive);                  // execute on receive onReceive

  LoRa.receive();                             // put the radio into receive mode
  
  u8x8.begin();

}

void loop() {
 
}

/*
 *  Function: void onReceive(int packetSize)
 *  Description: execute when we receive a packet on LoRa. Process it and update the display
 */
void onReceive(int packetSize) {
  String data;                              // it can be done also with a char buffer[packetSize]
  int rssi; float snr;                      // store the rssi and snr
  
  
  for (int i = 0; i < packetSize; i++) {    // put the data into the buffer
   // data[i] = (char)LoRa.read();          // if you are using a buffer 
    data += (char)LoRa.read();
    Serial.print(data[i]);
  }

  Serial.println(" ");

  rssi = LoRa.packetRssi();                 // get rssi
  snr  = LoRa.packetSnr();                  // get snr

  Serial.println(rssi);
  Serial.println(snr);
  Serial.println(" ");

  update_display(data, rssi, snr);          // update the display
}

/*
 *  Function: void update_display(char* data, int rssi, float snr)
 *  Description: updates the display with the received data, rssi and snr. If using buffer than String data must be char* data
 */
void update_display(String data, int rssi, float snr) {
  
  u8x8.setFont(u8x8_font_chroma48medium8_r);  
  u8x8.setCursor(0,0);
  u8x8.clear();
  u8x8.print(data);
  
  u8x8.setFont(u8x8_font_5x7_f); 
  u8x8.setCursor(0,3);
  u8x8.print("R: ");
  u8x8.print(rssi);
  u8x8.print(" ");
  u8x8.print("S: ");
  u8x8.print(snr);
}


