//------
//  Theia Thermometer - Simple Smart Thermometer 
//-----
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_MLX90614.h>
#include "Ailerons_Regular12pt7b.h"
#include "CeraPro_Medium8pt7b.h"

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//for feather32u4 
#define RFM95_CS       8
#define RFM95_RST      4
#define RFM95_INT      7

#define TFT_CS         11
#define TFT_RST        10 
#define TFT_DC         12


// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// For 1.44" TFT with ST7735:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//colors
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF



//LED and laser variables
int laser = 9; // choose the pin for the laser
int blueLED = A0; //red
int greenLED = A1; //green
int redLED = A2; //blue
int pin_switch = 6;   // choose the input pin (for a pushbutton)
float sensorValue = 0; // temp value
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 1000;  //the value is a number of milliseconds
long time = 0;         // the last time the recording was toggled
long debounce = 200;   // the debounce time, increase if the output flickers
int calibrationVal = 4.2; // this value helps calibrate the temperature sensor if needed.  It should be modified until Object C of boiling water is 100
boolean oldSwitchState = LOW;
boolean newSwitchState = LOW;
boolean Status = LOW;
int unitID = 1; //give THIS thermometer a number, so that it can be identified by the TheiaHub.  This is the unitID variable on the TheiaHub.py software.

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(redLED, red_light_value);
  analogWrite(greenLED, green_light_value);
  analogWrite(blueLED, blue_light_value);
}

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
  mlx.begin();  //start the MLX sensor
  startMillis = millis();  //initial start 

  int screenWidth = tft.width();
  int screenHeight = tft.height();
  pinMode(pin_switch, INPUT); //input button
  pinMode(laser, OUTPUT);  // declare laser as output
  pinMode(redLED, OUTPUT);  // red
  pinMode(greenLED, OUTPUT);  // green
  pinMode(blueLED, OUTPUT);  // blue

  Serial.begin(115200);

//  delay(100);

  Serial.println("Karakoram Theia Thermometer Bootup");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

//   Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  tft.fillScreen(BLACK);
  RGB_color(255, 255, 255); //start the LED on WHITE
  logo();
  delay(3000);
  tft.fillScreen(BLACK);
  tft.drawRect(10, 10, 108, 110, WHITE);
  tft.setTextColor(WHITE);
  tft.setFont(&CeraPro_Medium8pt7b);
  tft.setCursor((screenWidth/2)-20, 50);
  tft.println("Ready"); 
  tft.setCursor((screenWidth/2)-6, 70);
  tft.println("to");
  tft.setCursor((screenWidth/2)-17, 90); 
  tft.println("Scan"); 
}

void loop() {

  buttonEffects();

}


void middlepeak() {
  uint16_t color = WHITE;
  int startX = 34; // starting X
  int startY = 81; // starting y
  int triangleWidth = 48;
  int triangleHeight = 41;

  int x0 = startX;
  int y0 = startY;
  int x1 = x0+(triangleWidth/2);
  int y1 = y0-triangleHeight;
  int x2 = x0+triangleWidth;
  int y2 = y0;
    tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void leftpeak() {
  uint16_t color = WHITE;

  int startX = 22; // starting X
  int startY = 81; // starting y

  int x01 = startX;
  int y01 = startY;
  int x11 = x01+14;
  int y11 = y01-23;
  int x21 = x01+6;
  int y21 = y01;
  int x02 = x21;
  int y02 = y21;
  int x12 = x11;
  int y12 = y11;
  int x22 = x11+6;
  int y22 = y11;
  
  tft.fillTriangle(x01, y01, x11, y11, x21, y21, color);
  tft.fillTriangle(x02, y02, x12, y12, x22, y22, color);
}

void rightpeak() {
  uint16_t color = WHITE;

  int startX = 88; // starting X
  int startY = 81; // starting y
  
  int x01 = startX;
  int y01 = startY;
  int x11 = x01-22;
  int y11 = y01-39;
  int x21 = x11+9;
  int y21 = y11-17;
  int x02 = x01;
  int y02 = y01;
  int x12 = x21;
  int y12 = y21;
  int x22 = x01+20;
  int y22 = y01;
  tft.fillTriangle(x01, y01, x11, y11, x21, y21, color);
  tft.fillTriangle(x02, y02, x12, y12, x22, y22, color);
}

void logo() {
  leftpeak();
  middlepeak();
  rightpeak();
  tft.setCursor(2, 96);
  tft.setTextColor(WHITE);
  tft.setFont(&Ailerons_Regular12pt7b);
  tft.setTextSize(1);
  tft.println("Karakoram");
}

void temperature() {
  int screenWidth = tft.width();
  int screenHeight = tft.height();
  tft.setTextColor(WHITE);
  tft.setFont(&CeraPro_Medium8pt7b);
  tft.setCursor((screenWidth/2)-20, 50);
  tft.println("Ready"); 
  tft.setCursor((screenWidth/2)-6, 70);
  tft.println("to");
  tft.setCursor((screenWidth/2)-17, 90); 
  tft.println("Scan"); 
}

void buttonEffects() {
  int screenWidth = tft.width();
  int screenHeight = tft.height();
    newSwitchState = digitalRead(pin_switch);
  
    if ( newSwitchState != oldSwitchState && millis() - time > debounce ) 
    {
       if ( newSwitchState == HIGH )
       {
           if ( Status == LOW ) { 
              tft.fillScreen(BLACK);
              tft.drawRect(10, 10, 108, 110, WHITE);
              Serial.println("Laser On");  
              Status = HIGH; 
              digitalWrite(laser, Status); //turn laser on
              tft.setTextColor(WHITE);
              tft.setFont(&CeraPro_Medium8pt7b);
              tft.setCursor((screenWidth/2)-20, 60);
              tft.println("LASER"); 
              tft.setCursor((screenWidth/2)-10, 80);
              tft.println("ON");
              RGB_color(0, 0, 0);
            }
           else                    { 
              Serial.println("Laser OFF");  
              tft.fillScreen(BLACK); 
              Serial.println("Transmitting..."); // Send a message to rf95_server
              Status = LOW;  
              digitalWrite(laser, Status); //turn laser off
              float sensorValue = (mlx.readObjectTempC() + calibrationVal);
              int roudedValue = round(sensorValue);

              char buf[80] = {0}; //80 byte message
              sprintf(buf, "%d|%d", unitID, roudedValue); //can add in new variables here, but don't forget to add another|%d
              Serial.print("Sending "); Serial.println(buf);
              delay(10);
              rf95.send((uint8_t *)buf, 80); // send integer measured value as C-string, six byte packet
              delay(200);
              
              if (sensorValue > 32){
                tft.setTextColor(RED);
                tft.setFont(&CeraPro_Medium8pt7b);
                tft.drawRect(10, 10, 108, 110, RED);
                tft.setCursor((screenWidth/2)-25, 30);
                tft.println("Temp: "); 
                tft.setFont(NULL);
                tft.setTextSize(3);
                tft.setCursor(20, 50);
                tft.println(sensorValue);
                tft.setTextSize(1);
                tft.setFont(&CeraPro_Medium8pt7b);
                tft.setCursor(22, 100);
                tft.print("Degrees C");
                RGB_color(255, 0, 0);
              } else {
                tft.setTextColor(GREEN);
                tft.setFont(&CeraPro_Medium8pt7b);
                tft.drawRect(10, 10, 108, 110, GREEN);
                tft.setCursor((screenWidth/2)-25, 30);
                tft.println("Temp: "); 
                tft.setFont(NULL);
                tft.setTextSize(3);
                tft.setCursor(20, 50);
                tft.println(sensorValue);
                tft.setTextSize(1);
                tft.setFont(&CeraPro_Medium8pt7b);
                tft.setCursor(22, 100);
                tft.print("Degrees C");
                RGB_color(0, 255, 0);
              }
           }
       }
       oldSwitchState = newSwitchState;
    }   
};
