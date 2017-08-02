#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(9600);                             // starting serial, will be used for gps

  if (!LoRa.begin(866E6)) {                       // starting LoRa at 868Mhz
    Serial.println("Starting LoRa failed!");      // did not start 
    while (1);                                    // put debug here, led or display
  }
  
}

void loop() {
  lora_send_num(1);
  lora_send_string("1");
  lora_send_float(1.23);
  delay(2000);

  
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

