#include <WiFi.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <Adafruit_NeoPixel.h>

/*Put your SSID & Password*/
const char* ssid = "ESP32";  // Enter SSID here
const char* password = "NULL";  //Enter Password here

IPAddress local_IP(192, 168, 1, 50);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP Udp;
const IPAddress outIp(192, 168, 1, 255);
const unsigned int outPort = 8888;
const unsigned int localPort = 9999;
OSCErrorCode error;

WebServer server(80);

uint8_t LED1pin = 22;
bool LED1status = LOW;

uint8_t LED2pin = 5;
bool LED2status = LOW;

int clap101 = 0;
String score101;
int clap102 = 0;
String score102;
int clap103 = 0;
String score103;
int clap104 = 0;
String score104;

int ru = 0;
int BeatsPerMinute = 60; //60,000/BPM = ms //this is the intial start up value of BPM
int ms_of_beat = 60000 / BeatsPerMinute; //ms to turn on
int arr[] = {6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};
int brr[] = {6,6,6,6,1,6,4,6,3,2,6,6,5,6,6,6,3,6,6,2,6,1,6,6,6,6,6,4,6,5,6,6};
const int Metronome_LED = 22; //Pin LED is attached to
const int buttonPin = 36;
const int buttonPin1 = 39;
int buttonState = 0;
int buttonState1 = 0;
int LEDonRate = 70; //time LED is spent on for each flash
unsigned long last_beat = 0;
int current_time;
int metronome_led = 0;
unsigned long metronome_pulse;
unsigned long pulsed_button;
unsigned long time_difference;

int j = 0; 
int game = 0;
int type_game = 0;
int b = 1;
int button_pressed;
int i = random(0,32);

#define NUM_LEDS 19
Adafruit_NeoPixel pixels(NUM_LEDS, Metronome_LED, NEO_GRB + NEO_KHZ800);

// ******************** OSC ****************************************
//**************************************************************Send message OSC
void sendOSC( int val) {
  OSCMessage msg("/game/clap/101");
  msg.add(val);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();  // Mark the end of the OSC Packet
  msg.empty();   // Free space occupied by message

  //  digitalWrite(LEDpin, HIGH);
  //delay(500);
  //  digitalWrite(LEDpin, LOW);
  //delay(500);
}

void sendStartOSC( int val) {
  OSCMessage msg("/start/all");
  msg.add(val);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();  // Mark the end of the OSC Packet
  msg.empty();   // Free space occupied by message

  //  digitalWrite(LEDpin, HIGH);
  //delay(500);
  //  digitalWrite(LEDpin, LOW);
  //delay(500);
}

void sendPlayerOSC( int val) {
  OSCMessage msg("/player");
  msg.add(val);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();  // Mark the end of the OSC Packet
  msg.empty();   // Free space occupied by message

  //  digitalWrite(LEDpin, HIGH);
  //delay(500);
  //  digitalWrite(LEDpin, LOW);
  //delay(500);
}

void sendTypeGame( int val) {
  OSCMessage msg("/gametype");
  msg.add(val);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();  // Mark the end of the OSC Packet
  msg.empty();   // Free space occupied by message

  //  digitalWrite(LEDpin, HIGH);
  //delay(500);
  //  digitalWrite(LEDpin, LOW);
  //delay(500);
}
//**************************************************************Recieve message OSC
void RecieveMessageOSC() {
  // WiFiClient client=server.available(); //AP
  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      //msg.dispatch("/led", led);
      msg.dispatch("/tempo/ms", Receive_BPM);
      //msg.dispatch("/metro/on", Reset_BPM);
      msg.dispatch("/metro/on", gameStart);
      msg.dispatch("/gamescore/101", printMessage101);
      msg.dispatch("/gamescore/102", printMessage102);
      msg.dispatch("/gamescore/103", printMessage103);
      msg.dispatch("/gamescore/104", printMessage104);



    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}


String SendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Rhythm game</title>\n";
  ptr += "<link rel='stylesheet' href='https://fonts.googleapis.com/icon?family=Material+Icons'>";
  ptr +="<style>html { background: grey; font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {color: black;}\n";
  ptr +="#p1 {font-size: 40px;color: blue;margin-right: 450px;}\n";
  ptr +="#pa1 {font-size: 25px;color: black;margin-right: 450px;}\n";
  ptr +="#p2 {font-size: 40px;color: green;margin-left: 450px; margin-top: -120px;}\n";
  ptr +="#pa2 {font-size: 25px;color: black;margin-right: -450px;}\n";
  ptr +="#p3 {font-size: 40px;color: red;margin-right: 450px;}\n";
  ptr +="#pa3 {font-size: 25px;color: black;margin-right: 450px;}\n";
  ptr +="#p4 {font-size: 40px;color: orange;margin-left: 450px; margin-top: -120px;}\n";
  ptr +="#pa4 {font-size: 25px;color: black;margin-right: -450px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";


  
  ptr +="<h1 id='p1'> Player 1</h1>\n"; 
  {
    ptr +="<p id ='pa1' >Score: ";
    ptr +=score101;
    ptr +="</p> ";
  }

  
  ptr +="<h1 id='p2'> Player 2</h1>\n"; 
  {
    ptr +="<p id ='pa2'>Score: ";
    ptr +=score102;
    ptr +="</p> ";
  }

   ptr +="<h1 id='p3'> Player 3</h1>\n"; 
  {
    ptr +="<p id ='pa3'>Score: ";
    ptr +=score103;
    ptr +="</p> ";
  }

   ptr +="<h1 id='p4'> Player 4</h1>\n"; 
  {
    ptr +="<p id ='pa4'>Score: ";
    ptr +=score104;
    ptr +="</p> ";
  }



  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

//**************************************************************Get 101 score
void printMessage101(OSCMessage &msg) {
  clap101 = msg.getFloat(0);
  Serial.print("101: ");
  Serial.println(clap101);  
  score101 = String(clap101);
  server.send(200, "text/html", SendHTML());  
}

//**************************************************************Get 102 score
void printMessage102(OSCMessage &msg) {
  clap102 = msg.getFloat(0);
  Serial.print("102: ");
  Serial.println(clap102);  
  score102 = String(clap102);
  server.send(200, "text/html", SendHTML()); 
}

//**************************************************************Get 103 score
void printMessage103(OSCMessage &msg) {
  clap103 = msg.getFloat(0);
  Serial.print("103: ");
  Serial.println(clap103);  
  score103 = String(clap103);
  server.send(200, "text/html", SendHTML()); 
}
//**************************************************************Get 104 score
void printMessage104(OSCMessage &msg) {
  clap104 = msg.getFloat(0);
  Serial.print("104: ");
  Serial.println(clap104);  
  score104 = String(clap104);
  server.send(200, "text/html", SendHTML()); 
}
//**************************************************************Recieve BPM over OSC
void Receive_BPM(OSCMessage &msg) {
  ms_of_beat = msg.getFloat(0);
  Serial.println(msg.getFloat(0));

}

int randomPlayer() {
  int sol = 6;
  if (j < 4) {
    j++;
    return sol;
  }
  else {
    if ( i > 31 ) {
      i = 0;
    }
    if (type_game == 0) {
    sol = arr[i];
    i++;
    }
    else{
      sol = brr[i];
      i++;
    }
  }

   return sol;
}


//**************************************************************Refresh BPM over OSC
//void Reset_BPM(OSCMessage &msg) {
//
//  int last_bpm = ms_of_beat;
//  Serial.println(msg.getFloat(0));
//  if (msg.getFloat(0) == 0) {
//    ms_of_beat = 0;
//    //Metronome_Blink();
//      for (int i = 0; i < pixels.numPixels(); i++) {
//        pixels.setPixelColor(i, 0, 0, 0);
//      }
//    pixels.show();
//  }
//  else {
//    ms_of_beat = last_bpm;
//  }
//
//}

//**************************************************************Recieve BPM over OSC
void gameStart(OSCMessage &msg) {
  i = random(0,32);
  j = 0;
  game = msg.getFloat(0);
  Serial.println("Game:");
  Serial.println(msg.getFloat(0));


}
//**************************************************************Metronome Blink
void Metronome_Blink() {

//  if (ms_of_beat == 0) {
//    digitalWrite(Metronome_LED, LOW);
//    pixels.clear();   // Set all pixel colors to 'off'
//    metronome_led = 0;
//
//  }

    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW) {
      delay(300);
      sendStartOSC(0); //set OSC message to be sent
      digitalWrite(Metronome_LED, HIGH);
     }
     else {
    // turn LED off:
        digitalWrite(Metronome_LED, LOW);
    }

    current_time = millis();
    if (current_time - last_beat >=  ms_of_beat) {
      last_beat = current_time;
        uint16_t i;
        ru = randomPlayer();
        switch (ru) {
          case 1:
          for (i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, 0, 0, 255);
            pixels.show();
          }
           sendPlayerOSC(1);
            break;
          case 2:
           for (i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, 255, 0, 0);
             pixels.show();
           }
            sendPlayerOSC(2);
            break;
          case 3:
          for (i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, 0, 255, 0);
            pixels.show();
          }
           sendPlayerOSC(3);
            break;
          case 4:
          for (i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, 255, 255, 0);
            pixels.show();
          }
           sendPlayerOSC(4);
            break;
          case 5:
          for (i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, 200, 200, 200);
            pixels.show();
          }
           sendPlayerOSC(5);
            break;
          case 6:
          for (i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, 0, 150, 150); //GRB
            pixels.show();
          }
          sendPlayerOSC(6);
            break;
      }
           
       
        delay(ms_of_beat/3);
        
         for (i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, 0, 0, 0);
          }
        pixels.show();
      }

    
      
}




void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}




void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(buttonPin, INPUT);
   pinMode(buttonPin1, INPUT);
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid);
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);
  
  server.on("/", handle_OnConnect);
  /*
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  */
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  pixels.setBrightness(40);
  pixels.begin(); // INITIALIZE NeoPixel pixels object (REQUIRED
  pixels.show();
  pixels.clear();
}


void loop() {
  server.handleClient();
  RecieveMessageOSC();
  
  if (game == 1) {
    Metronome_Blink();
  }
  
  
 
  OSCMessage msg;
  
  buttonState = digitalRead(buttonPin);
    if (buttonState == LOW) {
      delay(500);
      sendStartOSC(1); //set OSC message to be sent
      i = random(0,32);
      j = 0;
      digitalWrite(Metronome_LED, HIGH);
     }
     else {
    // turn LED off:
        digitalWrite(Metronome_LED, LOW);
    }

  buttonState1 = digitalRead(buttonPin1);
    if (buttonState1 == LOW) {
      delay(500);
      if (type_game < 2) {
          type_game = type_game + 1;
           //set OSC message to be sent
      }
      else {
        type_game = 0;
      }
      sendTypeGame(type_game);
      digitalWrite(Metronome_LED, HIGH);
      }
     else {
    // turn LED off:
        digitalWrite(Metronome_LED, LOW);
    }
  
  
}
