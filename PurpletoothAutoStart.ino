//
//  PurpletoothAutoStart.ino
//
//  Copyright (c) 2015 David Robles
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include <QueueList.h>
#include <bc127.h>
#include <SoftwareSerial.h>

SoftwareSerial swPort = SoftwareSerial(11,10);
// Create a BC127 and attach the software serial port to it.
BC127 jamModule = BC127(&swPort);
#define DESIRED_VOLUME_LEVEL 13

static BC127::opResult connectionState = BC127::DEFAULT_ERR;
static boolean setupSuccessful = false;
QueueList <String> commandQueue;

void setup() {                
  // Serial port configuration. The software port should be at 9600 baud, as that
  //  is the default speed for the BC127.
  Serial.begin(9600);
  swPort.begin(9600);
  
  // print start up baud
  Serial.println("Running baud check...");
  String baudValue;
  jamModule.stdGetParam("BAUD", &baudValue);
  if (baudValue.startsWith("9600")) {
    Serial.println("Using expected baud: " + baudValue);
  }
  else {
    Serial.println("Using unexpected baud");
    Serial.println("Setup failed");
    return;    
  }
  
  if (_SS_MAX_RX_BUFF < 256) {
    Serial.println("Software serial buffer may be too small for media info");
  }
  
  // set permanent params
  jamModule.setClassicSink();
  jamModule.stdSetParam("ENABLE_HFP", "OFF");
  jamModule.stdSetParam("ENABLE_PBAP", "OFF");
  jamModule.stdSetParam("AUTOCONN", "1");
  jamModule.stdSetParam("NAME", "PurpletoothAutoStart");
  jamModule.stdSetParam("MM", "1 1 1");
  jamModule.writeConfig();
  jamModule.reset();

  setupSuccessful = true;
  Serial.println("Setup complete");
}

void loop() 
{  
  if (!setupSuccessful) {
    return;
  }
  
  // pop all the commands from the queue.
  if (!commandQueue.isEmpty()) {
    processCommand(commandQueue.pop());
  }
  
  BC127::opResult currentConnectionState = jamModule.connectionState();
  if (currentConnectionState == connectionState && currentConnectionState != BC127::CONNECT_ERROR) { 
    loopEndedHook();   
    return;
  }
  
  connectionState = currentConnectionState;
  
  switch (connectionState) {
    case BC127::CONNECT_ERROR:
      Serial.println("CONNECT_ERROR");
      Serial.println("Reseting...");
      jamModule.reset();
      break;
    
    case BC127::SUCCESS: {
      Serial.println("SUCCESS");
      
      break;
    }
    default:
      break;
  }
  
  loopEndedHook();
}

void loopEndedHook() {
  checkForSerialOutput();
}

void checkForSerialOutput() {
  String buffer;
  String EOL = String("\n\r");
  unsigned long loopStart = millis();
  
  while (loopStart + 3000 > millis())
  {
    // Grow the current buffered data, until we receive the EOL string.    
    if (swPort.available()) {
      byte byteRead = swPort.read();
//      Serial.write(byteRead);
      buffer.concat(char(byteRead));
    }

    if (buffer.endsWith(EOL))
    {
      buffer.trim();
      processSerialNotification(buffer);
      
      buffer = "";
    }    
  }
 
  if (buffer.length() > 2) {
    Serial.println("could not process notification: " + buffer);
  }
  
  if (swPort.overflow()) {
    Serial.println("OVERFLOW!!!");
  }
}

void processCommand(String command) {
  if (command.startsWith("LO_VOL")) {    
    jamModule.stdCmd("VOLUME A2DP=1");
    
    commandQueue.push(String("PLAY"));
  }
  else if (command.startsWith("PLAY")) {    
    jamModule.musicCommands(BC127::PLAY);
    
    for (int i = 2; i < DESIRED_VOLUME_LEVEL; i+= 1) {
      unsigned long loopStart = millis();  
      while (loopStart + 500 > millis()) {}
          
      String volumeCmd = "VOLUME A2DP=" + String(i);
      jamModule.stdCmd(volumeCmd);
    }
  }
}

void processSerialNotification(String notification) {
  if (notification.startsWith("OPEN_OK AVRCP")) {
    Serial.println("notification: " + notification);
    commandQueue.push(String("LO_VOL"));
  }
//  else if (notification.startsWith("AVRCP_MEDIA TITLE:")) {
//    Serial.println("track title: " + notification.substring(19));
//  }
//  else if (notification.startsWith("AVRCP_MEDIA ARTIST:")) { 
//    Serial.println("track artist: " + notification.substring(20));
//  }
//  else if (notification.startsWith("AVRCP_MEDIA ALBUM:")) {
//    Serial.println("track album: " + notification.substring(19));
//  }
  else {
    Serial.println("unknown notification: " + notification);
  }
}

void printParam(String param) {
  String value;
  jamModule.stdGetParam(param, &value);
  Serial.println(param + ":" + value);  
}
