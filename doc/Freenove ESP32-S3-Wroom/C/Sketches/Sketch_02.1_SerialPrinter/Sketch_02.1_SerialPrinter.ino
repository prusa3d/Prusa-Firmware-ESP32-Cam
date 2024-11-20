/**********************************************************************
  Filename    : SerialPrinter
  Description : Use UART send some data to PC, and show them on serial monitor.
  Auther      : www.freenove.com
  Modification: 2022/10/20
**********************************************************************/

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32S3 initialization completed!");
}

void loop() {
  Serial.printf("Running time : %.1f s\r\n", millis() / 1000.0f);
  delay(1000);
}
