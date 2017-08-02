/*
  tinygps_example for LoRa/GPS Shield
  Support Devices: LoRa/GPS Shield + Arduino UNO 
  
  Example sketch showing how to get GPS data from LoRa GPS Shield with TinyGPS library. 

  Hardware Connection:
  1/ Arduino UNO + LoRa GPS Shield . if use other Arduino board, please check the softserial connection and modify the code
  2/ Wire GPS_RXD to Arduino D3
  3/ Wire GPS_TXD to Arduino D4
  4/ Remove jumper in GPS_RXD/GPX_TXD 1x3 pin header
  a photo for hardware configuration is here: http://wiki.dragino.com/index.php?title=File:Uno_gps_softwareserial.png  

  External Library Require:
  TinyGPS:  http://arduiniana.org/libraries/tinygps/ 

  Test:
  1/ Connect hardware and install necessary 
  2/ Upload the Sketch to Arduino, open the monitor and check the result.  
  result example screen shot: http://wiki.dragino.com/index.php?title=File:Uno_gps_softwareserial_1.png 

  modified 21 Mar 2017
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SoftwareSerial.h>
#include <TinyGPS.h>


float flat, flon, alti;
unsigned long age;
int satellites;

TinyGPS gps;
SoftwareSerial ss(3, 4); // Arduino RX, TX , 

static void smartdelay(unsigned long ms);

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


void setup()
{
  // initialize both serial ports:
  Serial.begin(9600);  // Serial to print out GPS info in Arduino IDE
  ss.begin(9600); // SoftSerial port to get GPS data. 
  
  smartdelay(2000);

}
void loop()
{

  satellites = gps.satellites();
  gps.f_get_position(&flat, &flon, &age);
  alti = gps.f_altitude();

  print_to_console();
  smartdelay(1000);
  
}

void print_to_console() {
  
  Serial.print(satellites);
  Serial.print(" ");
  Serial.print(flat, 6);
  Serial.print(" ");
  Serial.print(flon, 6);
  Serial.print(" ");
  Serial.print(alti);
  Serial.print(" ");
/*  gps.stats(&chars, &sentences, &failed);
  Serial.print(chars, 6);
  Serial.print(" ");
  Serial.print(sentences, 10);
  Serial.print(" ");
  Serial.print(failed,9);*/
  Serial.println();
  
}


