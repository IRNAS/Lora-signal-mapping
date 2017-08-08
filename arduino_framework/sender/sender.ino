#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <SD.h>

float flat, flon, alti, speed;                    // latitude, longitude, altitude and speedfor gps
unsigned long age;                                // age for gps
int satellites;                                   // num of satellites
long lastSendTime = 0;                            // last send time
int interval = 400;                               // interval between sends

File file;
String file_title = "irnas";
String file_extension = ".kml";
String file_name = file_title + file_extension;
int file_current_file = 1;

boolean header = false;            
const int buttonPin = 7;     
boolean buttonPressed = false;
int buttonState = 0;         

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

void set_file_name(String title) {
  
  file_name = title + file_extension;
  
  Serial.println("Setting title to: " + file_name);


  file_current_file++;
}


void check_file() {
  if (SD.exists(file_name)) {
    Serial.println("Excists");
  } else {
    Serial.println("It does not excist");
    
    Serial.println("Creating file...");
    file = SD.open(file_name, FILE_WRITE);
    file.close();
  }
}

void destroy_file() {
  Serial.println("Removing file");
  SD.remove(file_name);
}


void writeInfo(float speed, float lon, float lat, float alt) { //Write the data to the SD card fomratted for google earth kml

  
  buttonPressed = false;
  
  Serial.println("writeinfo");

  if(speed > 1) {
  
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
      file.println(F("</name>"));
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
  }
}

void writeFooter() { // if the record switch is now set to off, write the kml footer, and reset the header flag
  Serial.println("Write footer");

  
  file = SD.open(file_name, FILE_WRITE);
  header = false;
  
  if (file) {
    file.println(F("</Document>"));
    file.println (F("</kml>"));
    file.close ();
  }
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
void send_gps(float lat, float lon, float alti, float speed) {
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
 *  Function:    void lora_send_char(char data)
 *  Parameters:  char data -> data
 *  Description: Sends a char 'character' through the LoRa system
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

void gps_loop() {
  satellites = gps.satellites();                  // gets the num of satellites

  if(satellites != 255) {                         // satellites return 255 when not 3d fixed
    gps.f_get_position(&flat, &flon, &age);       // get position
    speed = gps.f_speed_kmph();
  
    
    // debug stuff
    Serial.print(satellites);
    Serial.print("-");
    Serial.print(flat, 5);
    Serial.print("-");
    Serial.print(flon, 5);
    Serial.print("-");
    Serial.print(speed);
    Serial.println();
      
    alti = gps.f_altitude();                      // get altitude
  
    send_gps(flat, flon, alti, speed);            // send with lora

    writeInfo(speed, flat, flon, alti);

   
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
    buttonState = digitalRead(buttonPin);
  }

  
  check_button();

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());


}

void check_button() {
   if(!buttonPressed) {
      if (buttonState == HIGH) {
        Serial.println("BUTTON!");
        digitalWrite(6, HIGH);
        buttonPressed = true;
        writeFooter();

        set_file_name(file_title + file_current_file);
        destroy_file();
        check_file();
        _delay_ms(5000);
        buttonPressed = false;
        digitalWrite(6, LOW);
        
      }
    
    }
}

