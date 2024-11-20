/**********************************************************************
  Filename    : SerialRW
  Description : Use UART read and write data between ESP32 and PC.
  Auther      : www.freenove.com
  Modification: 2022/10/20
**********************************************************************/
String inputString = "";      //a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup() {
  Serial.begin(115200);
  Serial.println(String("\nESP32S3 initialization completed!\r\n")
                + String("Please input some characters,\r\n")
                + String("select \"Newline\" below and Ctrl + Enter to send message to ESP32S3. \r\n"));
}

void loop() {
  if (Serial.available()) {         // judge whether data has been received
    char inChar = Serial.read();         // read one character
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
  if (stringComplete) {
    Serial.printf("inputString: %s \r\n", inputString);
    inputString = "";
    stringComplete = false;
  }
}
