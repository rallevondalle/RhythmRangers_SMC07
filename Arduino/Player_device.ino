// Wifi UDP library from espressif ESP32 github
//OSC protocol comunication for wifi. OSC from CNMAT
#include <Adafruit_NeoPixel.h>
#include <MPU9250_asukiaaa.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>


#ifdef _ESP32_HAL_I2C_H_
#define SDA_PIN 19
#define SCL_PIN 18
#endif

#ifdef __AVR__
#include <avr/power.h>
#endif

//----------------------------------------------------------------

const int PositiveButton1 = 36;
const int NegativeButton1 = 34;
const int InputButton = 39;
const int BuzzerPin = 19; //Dont delete, dont know the speaker pin

float BeatsPerMinute = 60; //60,000/BPM = ms //this is the intial start up value of BPM
float ms_of_beat = 60000 / BeatsPerMinute; //ms to turn on

const int Metronome_LED = 22; //Pin LED is attached to
int LEDonRate = 70; //time LED is spent on for each flash

unsigned long metronome_pulse;
unsigned long pulsed_button;
unsigned long time_difference;

float accuracy = 0.3;
int tempo_threshold = ms_of_beat * accuracy;

unsigned long last_beat = 0;
int current_time;

float score = 0;
String all_score_string;
float score_array[32];
int score_index = 0;
float total_score = 0;

int metronome_led = 0;

float aX, aY, aZ, aSQRT, gX, gY, gZ, mDirection, mX, mY, mZ;

//Variables for clap detection
const int numReadings = 10;      // the number of samples for the avarage
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
float total = 0;                // the running total
float average = 0;              // the average
int moment_clap = 0;            //Saves the insntant in miliseconds that a clap happened
int now;

float game1_start = 0;

#define NUM_LEDS 19
Adafruit_NeoPixel pixels(NUM_LEDS, Metronome_LED, NEO_GRB + NEO_KHZ800);
MPU9250_asukiaaa mySensor(MPU9250_ADDRESS_AD0_HIGH);

//-----------------Wifi Set up -----------------------------------

char ssid[] = "ESP32";          // your network SSID (name)
char pass[] = "NULL";
WiFiUDP Udp;
const IPAddress outIp(192, 168, 1, 255);
const unsigned int outPort = 8888;
const unsigned int outPortSTRING = 8765 ;
const unsigned int localPort = 9999;
OSCErrorCode error;

IPAddress local_IP(192, 168, 1, 102);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);


//**************************************************************Absolute Value function. Default from arduino does not work
int absolute (int i)
{
  if (i < 0)
    return -i;
  else
    return i;
}

//**************************************************************Metronome Blink
void Metronome_Blink() {

  current_time = millis();
  //Serial.print(ms_of_beat);

  if (ms_of_beat == 0) {
    digitalWrite(Metronome_LED, LOW);
    pixels.clear();   // Set all pixel colors to 'off'
    metronome_led = 0;

  }

  if (current_time - last_beat >=  ms_of_beat) {
    last_beat = current_time;
    metronome_pulse = millis();
  }
}

//**************************************************************score computing
float score_computing(){
  
  //pulsed_button = millis();
  pulsed_button = now;
  time_difference = absolute(pulsed_button - metronome_pulse);
  if (time_difference > ms_of_beat / 2) {
    time_difference = time_difference - ms_of_beat;
  }
    float numerator, denominator;
  numerator = absolute(time_difference);
  denominator = ms_of_beat / 2;
  score = 1 - (numerator / denominator);
  return score;
  
  }

//**************************************************************score lights GRB
void score_lights(float score1){
  
  if(score>=0.1){
    uint16_t i;
      for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, 0, 200, 0);
      }
    pixels.show();     
    }

     if(score>=0.2 && score<0.3){
    uint16_t i;
      for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, 25, 175, 0);
      }
    pixels.show();     
    }

     if(score>=0.3 && score<0.4){
    uint16_t i;
      for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, 50, 150, 0);
      }
    pixels.show();     
    }

     if(score>=0.4 && score<0.5){
    uint16_t i;
      for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, 75, 125, 0);
      }
    pixels.show();     
    }

     if(score>=0.5 && score<0.6){
    uint16_t i;
      for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, 100, 100, 0);
      }
    pixels.show();     
    }
  
  if(score>=0.6 && score<0.7){
    uint16_t i;
      for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, 125, 75, 0);
      }
    pixels.show();     
    }

    if(score>=0.7 && score<0.8){
    uint16_t i;
      for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, 150, 50, 0);
      }
    pixels.show();     
    }
      if(score>=0.8 && score<0.9){
  uint16_t i;
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, 175, 25, 0);
    }
  pixels.show();     
  }
      if(score>=0.9){
  uint16_t i;
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, 200, 0, 0);
    }
  pixels.show();     
  }
  
  /*
  //Cool lights, too much delay
  if(score<0.9){
    uint16_t i, j;
    int score255 = score*255;
    for (j = 255; j > 0; j--) {
      for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, j*score, (1/(score+1))*j, 0);
      }
    pixels.show();
    delay(2);
      
    }
  }
  else if(score>=0.9){
    uint16_t i, j;
    for (j = 255; j > 0; j--) {
      for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, j*score, 0, 0);
      }
    pixels.show();
    delay(2);
      }
    }
*/

}

//**************************************************************Tapping interrupt function
float IRAM_ATTR Tapping() {

  pulsed_button = millis();
  time_difference = pulsed_button - metronome_pulse;

  if (time_difference > ms_of_beat / 2) {
    time_difference = time_difference - ms_of_beat;
  }
//  if (time_difference < tempo_threshold / 2) {
//    Serial.printf("Nice tap, you are on tempo! %d ms late\n", time_difference);
//    for (int j = 13; j < 19; j++) {
//      pixels.setPixelColor(j, pixels.Color(150, 0, 0));
//      pixels.show();   // Send the updated pixel colors to the hardware.
//      //pixels.clear();
//    }
//  }
//  else if (time_difference > -tempo_threshold / 2) {
//    Serial.printf("Nice tap, you are on tempo! %d ms early\n", absolute(time_difference));
//    for (int j = 13; j < 19; j++) {
//      pixels.setPixelColor(j, pixels.Color(150, 0, 0));
//      pixels.show();   // Send the updated pixel colors to the hardware.
//      //pixels.clear();
//    }
//  }
//  else {
//    Serial.printf("%d ms -- Out of tempo\n", absolute(time_difference));
//    for (int j = 13; j < 19; j++) {
//      pixels.setPixelColor(j, pixels.Color(0, 150, 0));
//      pixels.show();   // Send the updated pixel colors to the hardware.
//      //pixels.clear();
//    }
//  }
   
}
//
////**************************************************************BPM Increase
//void BPM_Plus_One() {
//  BeatsPerMinute = BeatsPerMinute + 10;
//  ms_of_beat = 60000 / BeatsPerMinute;
//  Serial.printf("Tempo increased to %d BPM\n", BeatsPerMinute);
//}
//
////**************************************************************BPM Decrease
//void BPM_Minus_One() {
//  BeatsPerMinute = BeatsPerMinute - 10;
//  ms_of_beat = 60000 / BeatsPerMinute;
//  Serial.printf("Tempo decreased to %d BPM\n", BeatsPerMinute);
//}

//**************************************************************Clap detection
bool clap_detection() {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = aSQRT;
  // add the reading to the total:M
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
  //Serial.println(average);
  
//  int anti_double_claps = millis();
//  if (anti_double_claps - moment_clap > 100) {      
    if ( aSQRT - average > 4.5 ) {     
      now = millis();
//      moment_clap = anti_double_claps;
      return true;
//      delay(100);
    }   
    else {return false;}
//  }
}
//
//current_time = millis();
//  if (current_time - last_beat >=  ms_of_beat) {
//    last_beat = current_time;
//    metronome_pulse = millis();
//  }

// ******************** OSC ****************************************
//**************************************************************Send message OSC
void sendOSC( int val) {
  OSCMessage msg("/game/clap/102");
  msg.add(val);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();  // Mark the end of the OSC Packet
  msg.empty();   // Free space occupied by message
}
//**************************************************************Send message OSC drift
void sendOSC_drift( int val) {
  OSCMessage msg("/game/clap/102/drift");
  msg.add(val);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();  // Mark the end of the OSC Packet
  msg.empty();   // Free space occupied by message
}

//**************************************************************Send message OSC score string
void sendOSC_all_score_string( String val) {
  OSCMessage msg("/scorestring/102");
  msg.add(val);
  Udp.beginPacket(outIp, outPortSTRING);
  msg.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();  // Mark the end of the OSC Packet
  msg.empty();   // Free space occupied by message
}
//**************************************************************Send message OSC drift
void sendOSC_score( int val) {
  OSCMessage msg("/game/clap/102/score");
  msg.add(val);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();  // Mark the end of the OSC Packet
  msg.empty();   // Free space occupied by message
}
//**************************************************************Send message OSC drift
void sendOSC_index( int val) {
  OSCMessage msg("/game/clap/102/index");
  msg.add(val);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // Send the bytes to the SLIP stream
  Udp.endPacket();  // Mark the end of the OSC Packet
  msg.empty();   // Free space occupied by message
}
//**************************************************************Recieve BPM over OSC
void Receive_BPM(OSCMessage &msg) {
  ms_of_beat = msg.getFloat(0);
  Serial.println(msg.getFloat(0));

}
//**************************************************************Refresh BPM over OSC
void Reset_BPM(OSCMessage &msg) {

  int last_bpm = ms_of_beat;
  Serial.println(msg.getFloat(0));
  game1_start = msg.getFloat(0);
  if (msg.getFloat(0) == 0) {
    ms_of_beat = 0;
    Metronome_Blink();
  }
  else {
    ms_of_beat = last_bpm;
  }

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
      msg.dispatch("/metro/on", Reset_BPM);
      msg.dispatch("/game", metronome_game);
      


    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }

}
//**************************************************************game1
void metronome_game(OSCMessage &msg){  
  game1_start = msg.getFloat(0); 
  Serial.println(msg.getFloat(0));    
}

//**************************************************************Setup
void setup() {
  Serial.begin(115200);

  pinMode(Metronome_LED, OUTPUT);
  //pinMode(BuzzerPin, OUTPUT);

  //pinMode(PositiveButton1, INPUT);
  //pinMode(NegativeButton1, INPUT);
  //pinMode(InputButton, INPUT);

  //attachInterrupt(digitalPinToInterrupt(PositiveButton1), BPM_Plus_One, FALLING);
  //attachInterrupt(digitalPinToInterrupt(NegativeButton1), BPM_Minus_One, FALLING);
  //attachInterrupt(digitalPinToInterrupt(InputButton), Tapping, FALLING);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif

#ifdef _ESP32_HAL_I2C_H_ // For ESP32
  Wire.begin(SDA_PIN, SCL_PIN);
  mySensor.setWire(&Wire);
#endif

  mySensor.beginAccel();
  mySensor.beginGyro();
  mySensor.beginMag();

  pixels.setBrightness(40);
  pixels.begin(); // INITIALIZE NeoPixel pixels object (REQUIRED
  pixels.show();
  pixels.clear();

  for (int i = 0; i < pixels.numPixels(); i++) {
  pixels.setPixelColor(i, 0, 0, 0); //GRB
  pixels.show();
}
for (int i = 0; i < pixels.numPixels();  i++) {
  pixels.setPixelColor(i, 255, 0, 0); //GRB
  delay(10*i);
  pixels.show();
}
  for (int i = 0; i < pixels.numPixels(); i++) {
  pixels.setPixelColor(i, 0, 0, 0); //GRB
  pixels.show();
}
}

//**************************************************************Main Loop
void loop() {

  RecieveMessageOSC();
  //if(game1_start == 0){}
  if(game1_start == 1){ 
    int game1 = 1;
    int beats_per_game = 32;
    while(game1==1){   
      tempo_threshold = ms_of_beat * accuracy;
      
      if (mySensor.accelUpdate() == 0 && mySensor.gyroUpdate() == 0) {
        gX = mySensor.gyroX();
        gY = mySensor.gyroY();
        gZ = mySensor.gyroZ();
        
        aX = mySensor.accelX();
        aY = mySensor.accelY();
        aZ = mySensor.accelZ();
        aSQRT = mySensor.accelSqrt();
      }
      
      Metronome_Blink();
      bool clap;
      clap = clap_detection();
      delay(1);
              
      int gate = 0;
      
  if (clap) {
      gate = 1;
      int anti_double_claps = millis();
    if (anti_double_claps - moment_clap > 100) {      
      moment_clap = anti_double_claps;

      score = score_computing();
      score_lights(score);

      int int_score = score*100;

      String score_string = String(int_score);
      char coma = ',';
      score_string.concat(coma);     
      all_score_string.concat(score_string);
      
      score_array[score_index] = score;
      total_score = total_score + score_array[score_index];
      Serial.print("Clap score: ");
      Serial.println(score_array[score_index]*100); 

      sendOSC_index(score_index);
      sendOSC_score(int_score);           
      
      score_index++;       
    }   
  } 
      RecieveMessageOSC();
      
      if (score_index == beats_per_game || game1_start == 0) {
        
        if (score_index == 0){score_index=1;}
        int avarage_score = (total_score / score_index)*100;
        
        float sum = 0;
        for(int i=0; i<score_index; i++){
          sum = sq((score_array[i]*100)-(avarage_score))+sum;          
        }
        float variance = sqrt(sum/score_index);
        Serial.print("Variance: ");
        Serial.println(variance);
        Serial.print("Average score: ");
        Serial.println(avarage_score); 
        
        Serial.print("All score string: ");
        Serial.println(all_score_string);

        sendOSC_all_score_string(all_score_string);
        sendOSC_drift(avarage_score);
        all_score_string = "";
        score_index = 0;
        total_score = 0;
        game1 = 0;
        game1_start = 0;
        for (int i = 0; i < pixels.numPixels(); i++) {
          pixels.setPixelColor(i, 0, 0, 0); //GRB
          pixels.show();
        }
        if(avarage_score>50){
          int pixels_score = ((avarage_score-50)*pixels.numPixels())/50;
          for (int i = 0; i < pixels_score; i++) {
            pixels.setPixelColor(i, 255, 0, 0); //GRB
            delay(10*i);
            pixels.show();
          }
        }   
      }
    }
  }
}
