/*
  SD card basic file example

 This example shows how to create and destroy an SD card file
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */
#include <SPI.h>
#include <SD.h>

File file;
String file_title = "irnas";
String file_extension = ".kml";
String file_name = file_title + file_extension;
int file_current_file = 1;


boolean header = false;            // Has the header been written?
const int buttonPin = 7;     // the number of the pushbutton pin
boolean buttonPressed = false;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  Serial.print("Initializing SD card...");

  pinMode(buttonPin, INPUT);
  pinMode(6, OUTPUT);

  if (!SD.begin(SPI_QUARTER_SPEED, 8)) {
    Serial.println("initialization failed!");
    while(1) {
      digitalWrite(6, HIGH);
      delay(100);
      digitalWrite(6, LOW);
      delay(100);
    }
  }
  Serial.println("initialization done.");


}

void loop() {

  set_file_name(file_title + file_current_file);
  destroy_file();
  check_file();

  while(1) {
    writeInfo(10.0, 45.0, 55.0, 69.0);
    // read the state of the pushbutton value:
    buttonState = digitalRead(buttonPin);
  
    // check if the pushbutton is pressed.
    // if it is, the buttonState is HIGH:
    if(!buttonPressed) {
      if (buttonState == HIGH) {
        digitalWrite(6, HIGH);
        buttonPressed = true;
        writeFooter();

        set_file_name(file_title + file_current_file);
        destroy_file();
        check_file();

       
        
        delay(500);
        digitalWrite(6, LOW);
      }
    }
  }
    
  delay(500);
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
    file.print(F("<Point><Coordinates>"));
    file.print(lon, 6);
    file.print(F(","));
    file.print(lat, 6);
    file.print(F("</Coordinates>"));
    file.print(F("<altitude>"));
    file.print(alt, 6);
    file.println(F("</altitude></Point></Placemark>"));
    file.close(); // close the file:
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

void set_file_name(String title) {
  
  file_name = title + file_extension;
  
  Serial.println("Setting title to: " + file_name);


  file_current_file++;
}

