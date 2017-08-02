#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

float flat, flon, alti;
unsigned long age;
int satellites;

TinyGPS gps;
SoftwareSerial ss(3, 4); // Arduino RX, TX , 

void setup() {
  
  Serial.begin(9600);  // Serial to print out GPS info in Arduino IDE
  ss.begin(9600); // SoftSerial port to get GPS data. 

  if (!LoRa.begin(866E6)) {                       // starting LoRa at 868Mhz
    Serial.println("Starting LoRa failed!");      // did not start 
    while (1);                                    // put debug here, led or display
  }

   smartdelay(2000);
}

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
    satellites = gps.satellites();
    
    Serial.print(satellites);
    Serial.print("-");
    
    if(satellites != 0) {
      gps.f_get_position(&flat, &flon, &age);
      
      Serial.print(flat);
      Serial.print("-");
      Serial.print(flon);
      
      alti = gps.f_altitude();
  
      send_gps(flat, flon, alti);
    } else {
      Serial.print("Nothing to see here");
      
      lora_send_string("nothing");
    }
    Serial.println();
    smartdelay(1000);
    
}


void send_gps(float lat, float lon, float alti) {
  LoRa.beginPacket();
  LoRa.print('a');
  LoRa.print(lat * 10000);
  LoRa.print('o');
  LoRa.print(lon * 10000);
  LoRa.print('t');
  LoRa.print(alti * 10000);
  LoRa.endPacket();
}

/*
 *  Function:    void lora_send_num(size_t data)
 *  Parameters:  size_t data -> a number 
 *  Description: Sends a integer or a number through the LoRa system
 */
void lora_send_num(size_t data) {
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
}

/*
 *  Function:    void lora_send_float(float data)
 *  Parameters:  float data -> float 
 *  Description: Sends a float through the LoRa system
 *  Problems: it will not send 1.23456 it will send 1.23
 */
void lora_send_float(float data) {
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
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

