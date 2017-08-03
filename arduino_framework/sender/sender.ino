#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

float flat, flon, alti;                           // latitude, longitude and altitude for gps
unsigned long age;                                // age for gps
int satellites;                                   // num of satellites
long lastSendTime = 0;                            // last send time
int interval = 400;                               // interval between sends

TinyGPS gps;
SoftwareSerial ss(3, 4); // Arduino RX, TX , 

void setup() {
  
  Serial.begin(9600);                             // Serial to print out in Arduino IDE
  ss.begin(9600);                                 // SoftSerial port to get GPS data. 

  if (!LoRa.begin(866E6)) {                       // starting LoRa at 868Mhz
    Serial.println("Starting LoRa failed!");      // did not start 
    while (1);                                    // put debug here, led or display
  }
  
  delay(1000);                                     // wait for gps to stabalize
}

/*
 *  Function: static void smartdelay(unsigned long ms)
 *  Description: waits a certain time defined by the ms and reads the gps
 */
static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
    {
      //ss.print(Serial.read());
      gps.encode(ss.read());
    }
  } while (millis() - start < ms);
}

void gps_loop() {
  satellites = gps.satellites();                  // gets the num of satellites

  if(satellites != 255) {                         // satellites return 255 when not 3d fixed
    gps.f_get_position(&flat, &flon, &age);       // get position

    // debug stuff
    /*Serial.print(satellites);
    Serial.print("-");
    Serial.print(flat, 5);
    Serial.print("-");
    Serial.print(flon, 5);
    Serial.println();*/
      
    alti = gps.f_altitude();                      // get altitude
  
    send_gps(flat, flon, alti);                   // send with lora
  } else {
    // whoops, error!
    Serial.print("Nothing to see here");
      
    lora_send_char('n');  
  }

  smartdelay(1000);                               // smarty delay
}


void loop() {
  if (millis() - lastSendTime > interval) {
    gps_loop();
    lastSendTime = millis();            // timestamp the message
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());

}

void onReceive(int packetSize) {
  if(packetSize > 0) {
    char data[packetSize];                                                                        // packet data           
  
    for (int i = 0; i < packetSize; i++) {                  
      data[i] =  ((char)LoRa.read());                                                             // put the data into the buffer
    }

    if(data[0] == 'd') {                                                                          // check indentifier
      if(data[1] != 's') {                                                                        // if it hasnt succeeded
        Serial.println("Smarty delay, returned status error");
        smartdelay(100);                                                                          // smarty delay
      }
    }
  }
}

/*
 *  Function: void send_gps(float lat, float lon, float alti)
 *  Description: send lat, lon and alti through lora
 */
void send_gps(float lat, float lon, float alti) {
  LoRa.beginPacket();                               // begin packet
  LoRa.print('a');                                  // to spot lat beginning
  LoRa.print(lat * 10000);                          // multiply for easier transfer
  LoRa.print('o');                                  // to spot lon beginning
  LoRa.print(lon * 10000);                          // multiply for easier transfer
  LoRa.print('t');                                  // to spot alti beginning
  LoRa.print(alti * 10000);                         // multiply for easier transfer
  LoRa.endPacket();                                 // end packet
}

/*
 *  Function:    void lora_send_char(char data)
 *  Parameters:  char data -> data
 *  Description: Sends a char 'character' through the LoRa system
 */
void lora_send_char(char data) {
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
}

