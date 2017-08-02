#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <stdint.h>

SoftwareSerial ss(4, 3);
TinyGPS gps;

void setup() {
  Serial.begin(9600);                             // starting serial, will be used for gps
  ss.begin(4800);

  if (!LoRa.begin(866E6)) {                       // starting LoRa at 868Mhz
    Serial.println("Starting LoRa failed!");      // did not start 
    while (1);                                    // put debug here, led or display
  }
  
}



void loop() {
  /*
   * LoRa sending data, fake data
   */
  float flat, flon;
  //45.248618, 19.840049
  flat = 45.248618; flon = 19.840049; 
  send_gps(flat, flon);

  delay(5000);
  
  /*
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print(" PREC=");
    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
  }
  
  gps.stats(&chars, &sentences, &failed);
  Serial.print(" CHARS=");
  Serial.print(chars);
  Serial.print(" SENTENCES=");
  Serial.print(sentences);
  Serial.print(" CSUM ERR=");
  Serial.println(failed);
  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");
  */
}

void send_gps(float lat, float lon) {
  LoRa.beginPacket();
  LoRa.print('a');
  LoRa.print(lat * 10000);
  LoRa.print('o');
  LoRa.print(lon * 10000);
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

