#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8X8_SSD1306_128X32_UNIVISION_SW_I2C u8x8(SCL, SDA, U8X8_PIN_NONE);                             // setup the fancy displaysnr

void setup() {
  Serial.begin(9600);                                                                           // serial for debugging
 
  //Serial.println("LoRa Receiver");                                                              // print welcome message
  
  if (!LoRa.begin(866E6)) {                                                                     // LoRa setup at 868MHz
    Serial.println("Starting LoRa failed!");                                                    // failed
    while (1);                                                                                  // put some debug
  }

  //LoRa.onReceive(onReceive);                                                                    // execute on receive onReceive

  //LoRa.receive();                                                                               // put the radio into receive mode
  
  u8x8.begin();

  display_nodata();

}


long lastSendTime = 0;            // last send time
int interval = 500;               // interval between sends
char status_lora = 'e';           // for beginning error
byte onReceiveStatusCurrent = 0;
byte onReceiveStatusPast = 0;
int packet_size;

void loop() { 
  if (millis() - lastSendTime > interval) {
    lora_send_status(status_lora);
    lastSendTime = millis();            // timestamp the message
  }

    onReceive(LoRa.parsePacket());
  
}

/*
 *  Function: void onReceive(int packetSize)
 *  Description: execute when we receive a packet on LoRa. Process it and update the display
 */
void onReceive(int packetSize) {
  if(packetSize == 0) {
    display_nodata();
    return;
  }
  char data[packetSize];                                                                        // packet data           
  float lat, lon, alti, speed;                                                                  // latitude, longitude, altitude and speed
  int rssi; float snr;                                                                          // rssi and                                                             
  
  for (int i = 0; i < packetSize; i++) {                  
    data[i] =  ((char)LoRa.read());                                                             // put the data into the buffer
  }

  /*for(int i=0; i < packetSize; i++ ){                                                           // debug received data
      Serial.print(data[i]);
  }
  Serial.println();*/

  // if everythin is okay
  if(data[0] == 'a' && data[10] == 'o' && data[20] == 't' && data[30] == 's') {
    
    char lat_char[10]; char lon_char[10]; char alti_char[10]; char speed_char[10];              // create local char arrays
    
    for(int i=1;i<10;i++)           { lat_char[i - 1]   = data[i]; }                             // write it into the local arrays
    for(int i=11;i<20;i++)          { lon_char[i - 11]  = data[i]; }                             // write it into the local arrays
    for(int i=21;i<30;i++)          { alti_char[i - 21] = data[i]; }                                     // write into the local arrays
    for(int i=31;i<packetSize;i++) { speed_char[i - 31] = data[i]; }

    lat = (float)atof(lat_char);   lon = (float)atof(lon_char); 
    alti = (float)atof(alti_char); speed = (float)atof(speed_char);                             // convert it to float
    lat = lat / 10000; lon = lon / 10000; alti = alti / 10000; speed = speed / 10000;           // divide it
    rssi = LoRa.packetRssi();                                                                   // get rssi 
    snr  = LoRa.packetSnr();                                                                    // get snr

    // debug printing with 10 decimal points
    Serial.print(lat, 10);
    Serial.print(',');
    Serial.print('0');
    Serial.print(',');
    Serial.print(lon, 10);
    Serial.print(',');
    Serial.print('0');
    Serial.print(',');
    Serial.print(speed, 10);
    Serial.print(',');
    Serial.print(alti,10);
    Serial.println();
   
    /*Serial.println(rssi,10);
    Serial.println(snr, 10);*/

    status_lora = 's';
    
    update_display(lat, lon, alti, rssi, snr);                                                  // update the display
    
  } else {
    display_nodata();
    
  }
}

/*
 *  Function: void update_display(char* data, int rssi, float snr)
 *  Description: updates the display with the received data, rssi and snr. If using buffer than String data must be char* data
 */
void update_display(float lat, float lon, float alti, int rssi, float snr) {
  
  u8x8.setFont(u8x8_font_chroma48medium8_r);                                                    // set font                                                                       
  u8x8.clearDisplay();                                                                          // clear the display
  
  u8x8.setCursor(0,0);                                                                          // set cursor on the first row
  u8x8.print("Lat: "); u8x8.print(lat, 3);                                                      // print latitude
    
  u8x8.setCursor(0,1);                                                                          // set cursor on the second row
  u8x8.print("Lon: "); u8x8.print(lon, 3);                                                      // print longitude
  
  u8x8.setCursor(0,2);                                                                          // set cursor on the third row
  u8x8.print("Alt: "); u8x8.print(alti, 3);                                                    // print altitude
  
  u8x8.setFont(u8x8_font_5x7_f);                                                                // set smaller font
  
  u8x8.setCursor(0,3);                                                                          // set cursor on the fourth row
  u8x8.print("R: ");
  u8x8.print(rssi);                                                                             // print rssi
  u8x8.print(" ");
  u8x8.print("S: ");
  u8x8.print(snr);                                                                              // print snr
}

/*
 *  Function:    void lora_send_status(char data)
 *  Parameters:  String data -> a text
 *  Description: Sends a char through the LoRa system
 */
void lora_send_status(char data) {
  LoRa.beginPacket();
  LoRa.print('d');                                                                              // indentificator
  LoRa.print(data);
  LoRa.endPacket();
}

void display_nodata() {
  onReceiveStatusCurrent = 1;
  if(onReceiveStatusPast != onReceiveStatusCurrent) {
    u8x8.clearDisplay();                                                                        // clear the display
    u8x8.setFont(u8x8_font_chroma48medium8_r);                                                  // set medium font  
    u8x8.setCursor(0,0);                                                                        // cursor at the beginning
    u8x8.print("NO DATA");                                                                      // NO DATA MAN!
    u8x8.setCursor(0,1);                                                                        // cursor at second line
    u8x8.print("GPS not fixed");                                                                // gps not fixed
    onReceiveStatusPast = onReceiveStatusCurrent;
  }
}


