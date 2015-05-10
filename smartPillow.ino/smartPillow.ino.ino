#include <Wire.h>
#include <Wire.h>
#include <ADXL345.h>
#include "pedometer.h"
#include <SPI.h>
#include <WiFi.h>
#include <TimerOne.h>

#define MINUTES_TO_ALARM  (60) //If you don't walk enough in MINUTES_TO_ALARM minutes, the buzzer beeps 0.5s.
unsigned long curtime = 0;
unsigned long cursteps = 0;

int isWalking = 0;        // if walking,isWalking=1.
int ipprinted = 0;        // if printing local ip, ipprinted = 1.

Pedometer pedometer;
int stepIndex = 0;

char ssid[] = "insertssidhere";           // your network SSID (name) 
char pass[] = "insertpasswordhere";       // your network password
int keyIndex = 0;                // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(88);
int serverconnected = 0;


// initialize Xadow – Breakout Board
void buzzer_init(){
   pinMode(13,OUTPUT);
   pinMode(11,OUTPUT);
}

// Xadow – Buzzer beeps for t_ms 
void buzzer_on(int t_ms){
  unsigned long cur = millis(); 
  while((millis()-cur) < t_ms){
    digitalWrite(13,HIGH);        //now the breakout board
    digitalWrite(11,HIGH);        // when pin13 and pin11 are HIGH, buzzer is on.
    delayMicroseconds(150);
    digitalWrite(13,LOW);
    digitalWrite(11,LOW);         // when pin13 and pin11 are LOW, buzzer is on.
    delayMicroseconds(150);
  }
}

void setup() {
  Serial.begin(9600);           // set baudrate = 9600bps
  // put your setup code here, to run once:
  buzzer_init();
  pedometer.init();
  Wire.begin(); 
  SeeedOled.init();                  //initialze SEEED OLED display
  SeeedOled.clearDisplay();          //clear the screen and set start position to top left corner
  SeeedOled.setNormalDisplay();      //Set display to normal mode (i.e non-inverse mode)
  SeeedOled.setPageMode();           //Set addressing mode to Page Mode  
  WiFi_Init();

  SeeedOled.drawBitmap(pmlogo,384);
  printxybmp(3,1,5,24,WalkMan[stepIndex]);
  printnum(4,6,"0");
  
  curtime = millis();              // get the current time
  cursteps = pedometer.stepCount;  // get the current steps
  
  Timer1.initialize(200000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
  Timer1.attachInterrupt( TimerISR ); // attach the service routine here
}

void loop() {
  // update pedometer steps
  StepsUpdate();
  
  // if wifi connected and not walking, print local ip.
  PrintLocalIP();
  
  // if wifi disconnected,reconnect. 
  if(WiFi.RSSI()==0){
     status = WL_IDLE_STATUS;
  }
  
  //If you don't walk enough in MINUTES_TO_ALARM minutes, the buzzer beeps 0.5s.
  if((millis()-curtime > MINUTES_TO_ALARM * 60000) && (pedometer.stepCount - cursteps < MINUTES_TO_ALARM*60/2)){
     buzzer_on(500);
     curtime = millis();
     cursteps = pedometer.stepCount;
  }
}

// update steps
void StepsUpdate(){
  static unsigned int stepCount = pedometer.stepCount;
  pedometer.stepCalc();
  if(pedometer.stepCount > stepCount && !ipprinted) {
    stepIndex++;
    if(stepIndex>5) stepIndex = 0;
    printxybmp(3,1,5,24,WalkMan[stepIndex]);
    char number[10];
    sprintf(number,"%d",pedometer.stepCount);
    printnum(4,6,number); 
    stepCount = pedometer.stepCount;
  }
}

// print local ip if not walking
void PrintLocalIP(){
  static unsigned int stepCount = pedometer.stepCount;
  static unsigned long curt = millis();
  
  if(status == WL_CONNECTED && serverconnected){
    if(!isWalking){
      if(!ipprinted){
        SeeedOled.setTextXY(3,0);
        SeeedOled.drawBitmap(blank,640);
      }
      SeeedOled.setTextXY(4,0);
      SeeedOled.putString("   Connected.   ");  
      SeeedOled.setTextXY(6,0);
      IPAddress ip = WiFi.localIP();
      char local_ip[16];
      sprintf(local_ip,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
      local_ip[15]= '\0';
      SeeedOled.putString(local_ip);
      ipprinted = 1;
    }
    else {
      if(ipprinted){
        SeeedOled.setTextXY(3,0);
        SeeedOled.drawBitmap(blank,640);
        ipprinted = 0;
      }
    }
  }
}


//initialize wifi connection 
void WiFi_Init(){
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 

  String fv = WiFi.firmwareVersion();
  if( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");
}

// print the WiFi connection status using serialport
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// send html to client
void sendhtml(){
   // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 1");         // refresh the page automatically every 1 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<title>Edison Pedometer</title>"); 
          client.println("<font face=\"Microsoft YaHei\" color=\"#0071c5\"/>");
          client.println("<h1 align=\"center\">Pedometer</h1>");
          client.print("<br /><br /><br /><br />");
          client.print("<h2 align=\"center\"><big>");
          client.print(pedometer.stepCount);
          client.println("  steps</big></h2>");       
          client.println("</html>");
           break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

// timer1 interrupt handle
void TimerISR()
{
  static unsigned long cur = millis();
  
  static unsigned long cur1 = millis();
  static unsigned int steps = pedometer.stepCount;
  
  if(status != WL_CONNECTED){
    if((millis()-cur) > 5000){
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
      status = WiFi.begin(ssid,pass);
      cur = millis();
    }
  } else {
    if(!serverconnected){
      server.begin();
      // you're connected now, so print out the status:
      printWifiStatus();
      serverconnected = 1;
    } else {
      sendhtml();
    }
  }
  
  if(millis()-cur1>5000){
    if(pedometer.stepCount == steps)
      isWalking = 0;
    else
      isWalking = 1;
    
    cur1 = millis();
    steps = pedometer.stepCount;
  }    
}
