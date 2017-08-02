#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

float flat, flon, alti;                           // latitude, longitude and altitude for gps
unsigned long age;                                // age for gps
int satellites;                                   // num of satellites

TinyGPS gps;
SoftwareSerial ss(3, 4); // Arduino RX, TX , 

void setup() {
  
  Serial.begin(9600);                             // Serial to print out in Arduino IDE
  ss.begin(9600);                                 // SoftSerial port to get GPS data. 

  if (!LoRa.begin(866E6)) {                       // starting LoRa at 868Mhz
    Serial.println("Starting LoRa failed!");      // did not start 
    while (1);                                    // put debug here, led or display
  }
  
   smartdelay(2000);                              // wait for gps to stabalize
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

void loop() {
    satellites = gps.satellites();                  // gets the num of satellites

    // debug stuff
    Serial.print(satellites);
    Serial.print("-");
    
    if(satellites != 0) {                           // if the satellites are not 0
      gps.f_get_position(&flat, &flon, &age);       // get position

      // debug stuff
      Serial.print(flat, 5);
      Serial.print("-");
      Serial.print(flon, 5);
      
      alti = gps.f_altitude();                      // get altitude
  
      send_gps(flat, flon, alti);                   // send with lora
    } else {
      // whoops, error!
      Serial.print("Nothing to see here");
      
      lora_send_string("nothing");  
    }
    Serial.println();
    smartdelay(1000);                               // smarty delay
    
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
 *  Function:    void lora_send_string(String data)
 *  Parameters:  String data -> a text
 *  Description: Sends a string through the LoRa system
 */
void lora_send_string(String data) {
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
}

