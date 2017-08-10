#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <SD.h>

#define BETWEEN(value, max, min) (value < max && value > min)

float flat, flon, alti, speed;                    // latitude, longitude, altitude and speedfor gps
unsigned long age;                                // age for gps
int satellites;                                   // num of satellites
long lastSendTime = 0;                            // last send time
int interval = 200;                               // interval between sends

long lastTimeLora = 0;
boolean loraConnection = false;
int speedLogger = 1;
float speedAvg = 0;
float tenSecDif = 0;

float rec_power = 0;

File file;
String file_title = "irnas";
String file_extension = ".kml";
String file_name = file_title + file_extension;
int file_current_file = 1;

boolean header = false;            
const int buttonPin = 7;     
int buttonState = false;         

TinyGPS gps;
SoftwareSerial ss(3, 4); // Arduino RX, TX , 

const int chipSelect = 8;

void setup() {
  
  Serial.begin(9600);                             // Serial to print out in Arduino IDE
  ss.begin(9600);                                 // SoftSerial port to get GPS data. 

  if (!LoRa.begin(866E6)) {                       // starting LoRa at 868Mhz
    Serial.println("Starting LoRa failed!");      // did not start 
    while (1);                                    // put debug here, led or display
  }

  pinMode(buttonPin, INPUT);
  pinMode(6, OUTPUT);

  if (!SD.begin(SPI_QUARTER_SPEED, 8)) {
    Serial.println("initialization failed!");
    for(int i=0; i < 10; i++) {
      digitalWrite(6, HIGH);
      delay(100);
      digitalWrite(6, LOW);
      delay(100);
    }
  }
  Serial.println("initialization done.");

  set_file_name(file_title + file_current_file);
  destroy_file();
  check_file();
  
  delay(1000);                                     // wait for gps to stabalize
}


void loop() {

  
  check_button();

  if (millis() - lastSendTime > interval) {
    
    check_button();
    gps_loop();
    
    long currentTimeLora = millis();
  
    if(currentTimeLora - lastTimeLora > 5000) {
      // we haven't received anything for more than 10 sec so there is no signal
      loraConnection = false;
    } else {
      loraConnection = true;
      
    }
    lastSendTime = millis();            // timestamp the message
    
  }

  
  check_button();

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());

  check_button();


}

void set_file_name(String title) {
  
  file_name = title + file_extension;
  
 // Serial.println("Setting title to: " + file_name);


  file_current_file++;
}


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

void destroy_file() {
 // Serial.println("Removing file");
  SD.remove(file_name);
}


void writeInfo(float speed, float lon, float lat, float alt) { //Write the data to the SD card fomratted for google earth kml

  
  
 // Serial.println("writeinfo");
  
//Serial.println(rec_power);

  if(speed > 1 /*|| (!loraConnection && tenSecDif > 50)*/) {
    
    file = SD.open(file_name, FILE_WRITE);
    if (header == false) { // If the header hasn't been written, write it.
      if (file) {
        file.print(F("<?xml version=\"1.0\" encoding=\"UTF-8\"?> <kml xmlns=\"http://earth.google.com/kml/2.0\"> <Document>"));
        header = true; // header flag set.
      }
    }
    if (file) { // write current GPS position data as KML
      file.println(F("<Placemark>"));
      file.print(F("<name>"));
      file.print(speed);
      file.print(F("km/h, rssi: "));
      file.print(rec_power);
      file.println(F("</name>"));
      file.print(F("<Style id=\"normalPlacemark\">"));
      file.print(F("<IconStyle>"));
      file.print(F("<Icon>"));
      file.print(F("<href>"));
      file.print(get_power());
      file.print(F("</href>"));
      file.print(F("</Icon>"));
      file.print(F("</IconStyle>"));
      file.print(F("</Style>"));
      file.print(F("<Point>"));
      file.print(F("<altitude>"));
      file.print(alt, 6);
      file.print(F("</altitude>"));
      file.print(F("<coordinates>"));
      file.print(lat, 6);
      file.print(F(","));
      file.print(lon, 6);
      file.print(F("</coordinates>"));
      file.println(F("</Point></Placemark>"));
      file.close(); // close the file:
    }

    file.close();
  }
}

void writeFooter() { // if the record switch is now set to off, write the kml footer, and reset the header flag
  //Serial.println("Write footer");

  
  file = SD.open(file_name, FILE_WRITE);
  header = false;
  
  if (file) {
    file.println(F("</Document>"));
    file.println (F("</kml>"));
    file.close();
  }

  file.close();
}

void onReceive(int packetSize) {

  
  loraConnection = true;
  
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
    } else {
      char power_char[10];
      for(int i=0; i < packetSize; i++) {
        power_char[i] = data[i];
      }

      rec_power = (float)atof(power_char);

    }
    
  } else {
    loraConnection = false;
  }

  lastTimeLora = millis();
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



void gps_loop() {
  
  satellites = gps.satellites();                  // gets the num of satellites

  if(satellites != 255) {                         // satellites return 255 when not 3d fixed

    
    gps.f_get_position(&flat, &flon, &age);       // get position
  
    speed = gps.f_speed_kmph();

    if(speedLogger > 10) {
      // compare speedavg
      //Serial.print("m/10s");
      tenSecDif = ((speedAvg / speedLogger) * 0.277778) * 7;
      //Serial.println(tenSecDif);
      
      speedLogger = 1;
      speedAvg = 0;
    }

    speedAvg = speedAvg + speed;
    
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

    writeInfo(speed, flat, flon, alti);

    speedLogger++;
   
  } else {
    
    // whoops, error!
    Serial.print("Nothing to see here");
      
    lora_send_char('n');  
  }

  smartdelay(1000);                               // smarty delay
}




void check_button() {
  
  buttonState = digitalRead(buttonPin);
  
  if (buttonState == HIGH) {    
    //Serial.println("BUTTON!");
    digitalWrite(6, HIGH);
    writeFooter();
    delay(1000);
    while(file) {
      file.close();
    }
    delay(1000);    
    set_file_name(file_title + file_current_file);
    destroy_file();
    check_file();
    delay(1000);
    digitalWrite(6, LOW);
    for(int i=1; i< file_current_file; i++) {
      digitalWrite(6, HIGH);
      delay(300);
      digitalWrite(6, LOW);
      delay(300);      
    }
  }

   
}

String get_power() {
  if(loraConnection == true) {
    if(rec_power > -50) {
      if(rec_power == 0) {
        return F("http://maps.google.com/mapfiles/kml/shapes/caution.png");
      } else {
        return F("http://maps.google.com/mapfiles/kml/pushpin/wht-pushpin.png");
      }
    } else if(BETWEEN(rec_power, -50, -80)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/pink-pushpin.png");
    } else if(BETWEEN(rec_power, -80, -100)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/purple-pushpin.png");
    } else if(BETWEEN(rec_power, -100, -105)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/red-pushpin.png");//eh orange
    } else if(BETWEEN(rec_power, -105, -110)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png");
    } else if(BETWEEN(rec_power, -110, -115)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/grn-pushpin.png");
    } else if(BETWEEN(rec_power, -115, -120)) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/ltblu-pushpin.png");
    } else if(rec_power < -120) {
      return F("http://maps.google.com/mapfiles/kml/pushpin/blue-pushpin.png");
    } 
  } else {
    return F("http://maps.google.com/mapfiles/kml/shapes/caution.png");
  }
}


