#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <SD.h>

#define BETWEEN(value, max, min) (value < max && value > min)   // making it easier to find between values

float flat, flon, alti, speed;                    // latitude, longitude, altitude and speedfor gps
unsigned long age;                                // age for gps
int satellites;                                   // num of satellites
long lastSendTime = 0;                            // last send time
int interval = 200;                               // interval between sends

long lastTimeLora = 0;                            // last millis we received from lora
boolean loraConnection = false;                   // current lora connection
int speedLogger = 1;                              // speed logger counter
float speedAvg = 0;                               // avg speed
float tenSecDif = 0;                              // ten sec difference counter

float rec_power = 0;                              // receiver power

File file;                                        // file object for sd
String file_title = "irnas";                      // always the file title
String file_extension = ".kml";                   // .kml is the file
String file_name = file_title + file_extension;   // the file name structure
int file_current_file = 1;                        // the current file number count
const int chipSelect = 8;                         // chip select for the sd card

boolean header = false;                           // header boolean for checking if we need header
const int buttonPin = 7;                          // button pin
int buttonState = false;                          // the state of the pin

TinyGPS gps;                                      // object for the gps
SoftwareSerial ss(3, 4); // Arduino RX, TX ,      // serial for the gps


void setup() {
  
  Serial.begin(9600);                             // Serial to print out in Arduino IDE
  ss.begin(9600);                                 // SoftSerial port to get GPS data. 

  if (!LoRa.begin(866E6)) {                       // starting LoRa at 868Mhz
    Serial.println("Starting LoRa failed!");      // did not start 
    while (1);                                    // put debug here, led or display
  }

  pinMode(buttonPin, INPUT);                      // button input
  pinMode(6, OUTPUT);                             // led ouput

  if (!SD.begin(SPI_QUARTER_SPEED, 8)) {          // spi quarter speed because too fast
    Serial.println("initialization failed!");     // failed

    // let the people know
    for(int i=0; i < 10; i++) { 
      digitalWrite(6, HIGH);
      delay(100);
      digitalWrite(6, LOW);
      delay(100);
    }
    
  } else {
    Serial.println("initialization done.");
  }
  
  set_file_name(file_title + file_current_file);  // set the file name (actually already set :P)
  destroy_file();                                 // destroy the file to make sure that in the next step we have an empty one
  check_file();                                   // create an empty file
  
  delay(1000);                                    // wait for gps to stabalize
}


void loop() {

  
  check_button();                                 // check state of the button

  // every interval (200ms)
  if (millis() - lastSendTime > interval) {
    
    check_button();                               // check button (again)
    gps_loop();                                   // do gps
    
    long currentTimeLora = millis();              // current time for lora compare
  
    if(currentTimeLora - lastTimeLora > 5000) {
      // we haven't received anything for more than 10 sec so there is no signal
      loraConnection = false;
    } else {
      loraConnection = true;
      
    }
    
    lastSendTime = millis();                      // timestamp the message
    
  }

  
  check_button();                                 // again check button

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());

  check_button();                                 // and again

  //! we are checking button so many times because all the interrupt pins are taken by the gps/lora hat

}

/*
 *  Function: void set_file_name(String title) 
 *  Description: Set the file name and increment file counter
 */
void set_file_name(String title) {
  
  file_name = title + file_extension;
  
 // Serial.println("Setting title to: " + file_name);


  file_current_file++;
}

/*
 *  Function: void check_file()
 *  Description: Check if file excists if not create new one
 */
void check_file() {
  if (SD.exists(file_name)) {
    //Serial.println("Excists");
  } else {
    //Serial.println("It does not excist");
    
   // Serial.println("Creating file...");
    file = SD.open(file_name, FILE_WRITE);
    file.close();
  }
}

/*
 *  Function: void destroy_file()
 *  Description: just destroys it without question
 */
void destroy_file() {
 // Serial.println("Removing file");
  SD.remove(file_name);
}

/*
 *  Function: void writeInfo(float speed, float lon, float lat, float alt)
 *  Description: write the data to the SD card fomratted for google earth kml
 */
void writeInfo(float speed, float lon, float lat, float alt) { 
 
  // onyl log if speed is above 1
  if(speed > 1 /*|| (!loraConnection && tenSecDif > 50)*/) {
    
    file = SD.open(file_name, FILE_WRITE);                                          // open file
    if (header == false) {                                                          // If the header hasn't been written, write it.
      if (file) {
        file.print(F("<?xml version=\"1.0\" encoding=\"UTF-8\"?> <kml xmlns=\"http://earth.google.com/kml/2.0\"> <Document>"));
        header = true;                                                              // header flag set.
      }
    }
    if (file) {                                                                     // write current GPS position data as KML
      file.println(F("<Placemark>"));
      file.print(F("<name>"));
      file.print(rec_power);                                                        // receive powe ris the name
      file.println(F("</name>"));
      file.print(F("<Style id=\"normalPlacemark\">"));
      file.print(F("<IconStyle>"));
      file.print(F("<Icon>"));
      file.print(F("<href>"));
      file.print(get_power());                                                      // icon is the power
      file.print(F("</href>"));
      file.print(F("</Icon>"));
      file.print(F("</IconStyle>"));
      file.print(F("</Style>"));
      file.print(F("<Point>"));
      file.print(F("<altitudeMode>relativeToGround</altitudeMode>"));
      file.print(F("<coordinates>"));
      file.print(lat, 6);                                                           // latitude
      file.print(F(","));
      file.print(lon, 6);                                                           // longitude
      file.print(F(","));
      file.print(alt, 6);                                                           // altitude
      file.print(F("</coordinates>"));
      file.println(F("</Point></Placemark>"));
      file.close(); // close the file:
    }
  }
}

/*
 *  Function: void writeFooter()
 *  Description: if the record switch is now set to off, write the kml footer, and reset the header flag
 */
void writeFooter() {

  file = SD.open(file_name, FILE_WRITE);            // open the file
  header = false;                                   // set header to false
  
  if (file) {                                       // if actually opened the file... print 
    file.println(F("</Document>"));
    file.println (F("</kml>"));
    file.close();
  } else {
    Serial.println("Can't open file, what?");
  }
}

/*
 *  Function: void onReceive(int packetSize) 
 *  Description: receive interrupt function 
 */
void onReceive(int packetSize) {

  
  loraConnection = true;                                                                          // if we got something than we def have a connection
  
  if(packetSize > 0) {
    char data[packetSize];                                                                        // packet data           
  
    for (int i = 0; i < packetSize; i++) {                  
      data[i] =  ((char)LoRa.read());                                                             // put the data into the buffer
    }

    if(data[0] == 'd') {                                                                          // check indentifier
      if(data[1] != 's') {                                                                        // if it hasnt succeeded
        //Serial.println("Smarty delay, returned status error");
        gps_loop();
        //smartdelay(4000);                                                                          // smarty delay
        //gps_loop();
      }
    } else {                                                                                      // we have gotten other data (power)
      char power_char[10];
      for(int i=0; i < packetSize; i++) {
        power_char[i] = data[i];                                                                  // write it to the power char array
      }

      rec_power = (float)atof(power_char);                                                        // convert it to be useful

    }
    
  } else {
    loraConnection = false;                                                                       // if nothing arrived we model it as connection is false
  }

  lastTimeLora = millis();                                                                        // set last time lora
}

/*
 *  Function: void send_gps(float lat, float lon, float alti)
 *  Description: send lat, lon and alti through lora
 */
void send_gps(float lat, float lon, float alti, float speed) {
  loraConnection = true;
  
  LoRa.beginPacket();                               // begin packet
  LoRa.print('a');                                  // to spot lat beginning
  LoRa.print(lat * 10000);                          // multiply for easier transfer
  LoRa.print('o');                                  // to spot lon beginning
  LoRa.print(lon * 10000);                          // multiply for easier transfer
  LoRa.print('t');                                  // to spot alti beginning
  LoRa.print(alti * 10000);                         // multiply for easier transfer
  LoRa.print('s');                                  // to spot speed beginning
  LoRa.print(speed * 10000);                        // multiply for easier transfer
  LoRa.endPacket();                                 // end packet
}

/*
 *  Function: void lora_send_char(char data)
 *  Description: Send char data through lora
 */
void lora_send_char(char data) {
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
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

/*
 *  Function: void gps_loop()
 *  Description: the gps loop that does all the work with gps and lora
 */
void gps_loop() {
  
  satellites = gps.satellites();                  // gets the num of satellites

  if(satellites != 255) {                         // satellites return 255 when not 3d fixed

    
    gps.f_get_position(&flat, &flon, &age);       // get position
  
    speed = gps.f_speed_kmph();                   // get speed

    if(speedLogger > 10) {                        // speed logger counter is above 10
      // compare speedavg
      //Serial.print("m/10s");
      tenSecDif = ((speedAvg / speedLogger) * 0.277778) * 7;  // calculate tensecdif (ten second difference)
      //Serial.println(tenSecDif);
      
      speedLogger = 1;                            // reset logger
      speedAvg = 0;                               // reset avg
    }

    speedAvg = speedAvg + speed;                  // add up the speed
    
    // debug stuff
  /*  Serial.print(satellites);
    Serial.print("-");
    Serial.print(flat, 5);
    Serial.print("-");
    Serial.print(flon, 5);
    Serial.print("-");
    Serial.print(speed);
    Serial.println();*/
      
    alti = gps.f_altitude();                      // get altitude
  
    send_gps(flat, flon, alti, speed);            // send with lora

    writeInfo(speed, flat, flon, alti);           // write to sd

    speedLogger++;                                // speed logger counter
    
  } else {
    
    // whoops, error!
    Serial.print("Nothing to see here");
      
    lora_send_char('n');  
  }

  smartdelay(1000);                               // smarty delay
}

/*
 *  Function: void check_button()
 *  Descritpion: check the button and if high state than write footer and create new file
 */
void check_button() {
  
  buttonState = digitalRead(buttonPin);                 // get the state of the button
  
  if (buttonState == HIGH) {                            // if the button is high
    //Serial.println("BUTTON!");
    digitalWrite(6, HIGH);                              // led on
    writeFooter();                                      // write footer
    delay(1000);                                        // wait a second
    while(file) {                                       // while the file is here close it
      file.close();
    } 
    delay(1000);    
    set_file_name(file_title + file_current_file);      // set new name
    destroy_file();                                     // destroy if it is avaible
    check_file();                                       // check file and create new
    delay(1000);                    
    digitalWrite(6, LOW);                               // led low

    // show how many files are there
    for(int i=1; i< file_current_file; i++) {
      digitalWrite(6, HIGH);
      delay(300);
      digitalWrite(6, LOW);
      delay(300);      
    }
  }

   
}

/*
 *  Function: String get_power()
 *  Description: returns a string that is the url to the power indicator
 */
String get_power() {
  if(loraConnection == true) {                                                              // do all of it if there is a lora connection
    if(rec_power > -50) {                                                                   // power lower than -50 
      if(rec_power == 0) {                                                                  // if 0 than that means that there is no lora connection
        return F("http://maps.google.com/mapfiles/kml/shapes/caution.png");                 // return caution sign
      } else {
        return F("http://maps.google.com/mapfiles/kml/pushpin/wht-pushpin.png");            // very good connection white one
      }
    } else if(BETWEEN(rec_power, -50, -80)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/pink-pushpin.png");             // between -50 and -80 pink pin
    } else if(BETWEEN(rec_power, -80, -100)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/purple-pushpin.png");           // between -80 and -100 purple pin
    } else if(BETWEEN(rec_power, -100, -105)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/red-pushpin.png");              // between -100 and -105 red one (orange)
    } else if(BETWEEN(rec_power, -105, -110)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png");              // between -105 and -110 yellow pin
    } else if(BETWEEN(rec_power, -110, -115)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/grn-pushpin.png");              // between -110 and -115 green pin
    } else if(BETWEEN(rec_power, -115, -120)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/ltblu-pushpin.png");            // between -115 and -120 light blue pin
    } else if(rec_power < -120) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/blue-pushpin.png");             // under -120 blue one
    } 
  } else {  
    return F("http://maps.google.com/mapfiles/kml/shapes/caution.png");                     // no connection caution!
  }
}


