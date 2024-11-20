/*
 ******************************************************************************
 * Sketch  WiFi Client and Server
 * Author  Zhentao Lin @ Freenove (http://www.freenove.com)
 * Date    2020/7/11
 ******************************************************************************
 * Brief
 *   This sketch is used to control a 2D ellipse through communicate to an 
 *   ESP32 board or other micro controller.
 *   It will automatically detect and connect to a device (serial port) which 
 *   use the same trans format.
 ******************************************************************************
 * Copyright
 *   Copyright © Freenove (http://www.freenove.com)
 * License
 *   Creative Commons Attribution ShareAlike 3.0 
 *   (http://creativecommons.org/licenses/by-sa/3.0/legalcode)
 ******************************************************************************
*/
import controlP5.*;
import processing.net.*;

ControlP5 cp5;
//Println console;
Server myServer;
Client myClient;
int dataIn=0;
int radioFlag1=0;
int radioFlag2=0;
int send_flag=1;

void setup() {
    size(600,400);
    smooth();
    noStroke();
    cp5 = new ControlP5(this);
    cp5.addTab("default")
       .activateEvent(true)
       .setHeight(30)
       .setWidth(100)
       .setLabel("TCP Server")
       .setId(1)
       ;
    cp5.getTab("TCP Client")
       .activateEvent(true)  
       .setHeight(30)
       .setWidth(100)
       .setId(2)  
       ;
 
    cp5.addTextfield("Local IP")
       .setPosition(20,40)                               
       .setSize(160,40)
       .setFont(createFont("微软雅黑 Light",20))
       .setColor(color(255,255,0))
       .setColorLabel(color(0))
       .setColorBackground(color(100,255))
       .moveTo("default")
       ;
    cp5.addTextfield("Local PORT")
       .setPosition(20,120)                            
       .setSize(160,40)
       .setFont(createFont("微软雅黑 Light",20))
       .setColor(color(255,255,0))
       .setColorLabel(color(0))
       .setColorBackground(color(100,255))
       .setText("8888")
       .moveTo("default")
       ;
    cp5.addRadioButton("connect1")
       .setPosition(20,200)                    
       .setSize(100,25)
       .addItem("Listening", 1)
       .setColorLabel(color(0))
       .moveTo("default")
       ;

    cp5.addTextfield("Remote IP")
       .setPosition(20,40)                               
       .setSize(160,40)
       .setFont(createFont("微软雅黑 Light",20))
       .setColor(color(255,255,0))
       .setColorLabel(color(0))
       .setColorBackground(color(100,255))
       .setText("192.168.1.xxx")
       .moveTo("TCP Client")
       ;
    cp5.addTextfield("Remote PORT")
       .setPosition(20,120)                            
       .setSize(160,40)
       .setFont(createFont("微软雅黑 Light",20))
       .setColor(color(255,255,0))
       .setColorLabel(color(0))
       .setColorBackground(color(100,255))
       .setText("80")
       .moveTo("TCP Client")
       ;
    cp5.addRadioButton("connect2")
       .setPosition(20,200)                  
       .setSize(100,25)
       .addItem("connect Server",1)
       .setColorLabel(color(0))
       .moveTo("TCP Client")
       ;

    cp5.addTextarea("recvData")
        .setPosition(200,40)
        .setSize(350,150)
        .setFont(createFont("微软雅黑 Light",20))
        .setColor(color(255,255,0))
        .setColorBackground(color(100,255))
        .scroll(15)
        .setColorForeground(color(0,100))
        .moveTo("global")
        ;
     cp5.addTextfield("sendData")
        .setPosition(200,230)                            
        .setSize(350,80)
        .setFont(createFont("微软雅黑 Light",20))
        .setColor(color(255,255,0))
        .setLabel("")
        .setFocus(true)
        .setColorBackground(color(100,255))
        .setColorForeground(color(0,100))
        .moveTo("global")
       ;
     cp5.addButton("clearRecv")
        .setValue(0)
        .setPosition(200,200)
        .setSize(100,20)
        .setColorLabel(color(255,100))
        .moveTo("global")
        ;
     cp5.addButton("clearSend")
        .setValue(0)
        .setPosition(200,320)
        .setSize(100,20)
        .setColorLabel(color(255,100))
        .moveTo("global")
        ;
     cp5.addButton("Send")
        .setValue(0)
        .setPosition(450,320)
        .setSize(100,20)
        .plugTo(this,"send_Textfield_SendData")
        .setColorLabel(color(255,100))
        .moveTo("global")
        ;
     cp5.get(Textfield.class,"Local IP").setText(Server.ip());
}

void draw(){
  background(220);
  if(radioFlag1==1){
    Client thisClient = myServer.available();
    if (thisClient !=null) {
      String whatClientSaid = thisClient.readString();
      if (whatClientSaid != null) {
        cp5.get(Textarea.class,"recvData").append(thisClient.ip()+":  ");
        cp5.get(Textarea.class,"recvData").append(whatClientSaid+"\n");
      } 
    }
    
  }
 // myServer.active()
  if(radioFlag2==1){
    if (myClient.available() > 0) {
      String whatClientSaid = myClient.readString();
      if (whatClientSaid != null) {
        cp5.get(Textarea.class,"recvData").append(cp5.get(Textfield.class,"Remote IP").getText()+":  ");
        cp5.get(Textarea.class,"recvData").append(whatClientSaid+"\n");
      } 
    } 
  }
}

void controlEvent(ControlEvent theControlEvent) {
  if (theControlEvent.isTab()) {
    if(theControlEvent.getTab().getId()==1){
      cp5.get(Textarea.class,"recvData").clear();
      cp5.get(Textfield.class,"sendData").clear();
      send_flag=1;
    }
    else if(theControlEvent.getTab().getId()==2){
      cp5.get(Textarea.class,"recvData").clear();
      cp5.get(Textfield.class,"sendData").clear();
      send_flag=2;
    }
  }
}

void connect1(int a) {
  println("a radio Button event: "+a);
  radioFlag1=a;
  if(radioFlag1==1){
    String port_buffer = cp5.get(Textfield.class,"Local PORT").getText();
    int port = int(port_buffer);
    myServer = new Server(this,port); 
    println(Server.ip());
    cp5.get(Textfield.class,"Local IP").setText(Server.ip());
    println(port);
  }
  else 
    myServer.stop();
}
void connect2(int a) {
  println("a radio Button event: "+a);
  radioFlag2=a;
  if(radioFlag2==1){
    String port_buffer = cp5.get(Textfield.class,"Remote PORT").getText();
    String IP_buffer = cp5.get(Textfield.class,"Remote IP").getText();
    int port = int(port_buffer);
    if(IP_buffer.compareTo("192.168.1.xxx")==0){
      println("connect error!");
    }
    else{
      myClient = new Client(this,IP_buffer,port); 
      println("connect success!");
    }
      
  }
  else 
    myClient.stop();
}
void clearRecv(){
  cp5.get(Textarea.class,"recvData").clear();
}
void clearSend(){
  cp5.get(Textfield.class,"sendData").clear();
}
void send_Textfield_SendData(){
  if(send_flag==1){
    myServer.write(cp5.get(Textfield.class,"sendData").getText());
    cp5.get(Textfield.class,"sendData").clear();
  }
  else if(send_flag==2){
    myClient.write(cp5.get(Textfield.class,"sendData").getText());
    cp5.get(Textfield.class,"sendData").clear();
  }
}