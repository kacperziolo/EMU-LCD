#include "Arduino.h"
#include <CircularBuffer.h>
#include <Ticker.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

CircularBuffer<uint8_t, 5> bytes;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
SoftwareSerial mySerial(10,11); // RX, TX

bool lock = false;
const int buttonPin = 2;
int buttonState; 
int lastButtonState = LOW; 
int buttonPressCount = 0;
int printInterval = 100; //ms print interval
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers



void show_values();
Ticker diaplay_trigger(show_values, printInterval, 0, MILLIS);


enum ECUData_T
{
    ECU_CHANNEL = 0,
    ECU_IDCHAR,
    ECU_VALUEH,
    ECU_VALUEL,
    ECU_CHECKSUM,
    ECU_FRAME_LENGTH,
};

struct ECUData {
    uint8_t channel;
    uint8_t idChar;
    uint8_t valueH;
    uint8_t valueL;
    uint8_t cs;
};

struct ECUData actECUFrame;

uint16_t lastChValue[255];
//uint16_t lastChValue_max[255];
//uint16_t lastChValue_min[255];


void show_values()
{
    Serial.println("");
    for (int i = 0; i < 33; i++)
    {
        Serial.print(lastChValue[i]);
        Serial.print("|");
    }

//    Serial.print("Batt=");Serial.println(float (lastChValue[4])/37);
//    Serial.print("temp internal=");Serial.println(lastChValue[19]);
//    Serial.print("MAP=");Serial.println(lastChValue[1]);
//    Serial.print("AFR=");Serial.println(float(lastChValue[11])/10);
//    Serial.print("Baro=");Serial.println(lastChValue[13]);
//    
//    Serial.println("");

//    lcd.clear(); 
//    lcd.setCursor(0,1);
//    lcd.print(buttonPressCount);
    
    switch (buttonPressCount) {
      case 0:
    
    //lcd.clear(); 
    lcd.setCursor(0,0);
    lcd.print("MAP=");lcd.print(lastChValue[1]);lcd.print("kPa  ");
    
    lcd.setCursor(0,1);
    lcd.print("Batt=");lcd.print(float (lastChValue[4])/37);lcd.print("V   ");
    break;

      case 1:
    
    //lcd.clear(); 
    lcd.setCursor(0,0);
    lcd.print("AFR=");lcd.print(float(lastChValue[11])/10);lcd.print("  ");
    
    lcd.setCursor(0,1);
    lcd.print("CLT=");lcd.print(lastChValue[23]);lcd.print("\262C");lcd.print("    ");
    break;

          case 2:
    
    //lcd.clear(); 
    lcd.setCursor(0,0);
    lcd.print("IgnA=");lcd.print(lastChValue[5]);lcd.print("\262");lcd.print("   ");//lcd.print(" deg  ");
    
    lcd.setCursor(0,1);
    lcd.print("EGT=");lcd.print(lastChValue[7]);lcd.print("\262C");lcd.print("    ");
    break;

              case 3:
    
    //lcd.clear(); 
    lcd.setCursor(0,0);
    lcd.print("OilP=");lcd.print(float(lastChValue[20]/16));lcd.print("Bar");lcd.print("   ");//lcd.print(" deg  ");
    
    lcd.setCursor(0,1);
    lcd.print("OilT=");lcd.print(lastChValue[21]);lcd.print("\262C");lcd.print("    ");
    break;

              case 4:
    
    //lcd.clear(); 
    lcd.setCursor(0,0);
    lcd.print("Boost=");lcd.print(int(lastChValue[1]-lastChValue[13]));lcd.print("kPa");lcd.print("   ");//lcd.print(" deg  ");
    
    lcd.setCursor(0,1);
    lcd.print("IAT=");lcd.print(lastChValue[3]);lcd.print("\262C");lcd.print("    ");
    break;

                  case 5:
    
    //lcd.clear(); 
    lcd.setCursor(0,0);
    lcd.print("RPM=");lcd.print(lastChValue[0]);lcd.print("            ");//lcd.print(" deg  ");
    
    lcd.setCursor(0,1);
    lcd.print("Lambda=");lcd.print(float(lastChValue[26]/128));lcd.print("    ");
    break;

    default:
     lcd.clear(); 
    lcd.setCursor(0,0);
    lcd.print("pushes=");lcd.print(buttonPressCount);

         }
}

inline void parseECU()
{
    //actECUFrame.channel
    //actECUFrame.valueH
    //actECUFrame.valueL

    //Serial.print("channel="); Serial.println(actECUFrame.channel);
    //Serial.print("idChar="); Serial.println(actECUFrame.idChar);
    //Serial.print("valueH="); Serial.println(actECUFrame.valueH);
    //Serial.print("valueL="); Serial.println(actECUFrame.valueL);
    //Serial.print("cs="); Serial.println(actECUFrame.cs);

    uint16_t val = (uint16_t)(actECUFrame.valueH << 8) + actECUFrame.valueL;
    lastChValue[actECUFrame.channel - 1] = val;
//    if (lastChValue_min[actECUFrame.channel] > val)
//        lastChValue_min[actECUFrame.channel] = val;
//    if (lastChValue_max[actECUFrame.channel] < val)
//        lastChValue_max[actECUFrame.channel] = val;
}


void setup()
{
    pinMode(buttonPin, INPUT_PULLUP);
    Serial.begin(19200);
    Serial.println("RPM|MAP|TPS|IAT|Batt|IgnAngle|pulseWidth|Egt1|Egt2|knockLevel|dwellTime|wboAFR|gear|Baro|analogIn1|analogIn2|analogIn3|analogIn4|injDC|emuTemp|oilPressure|oilTemperature|fuelPressure|CLT|flexFuelEthanolContent|ffTemp|wboLambda|vssSpeed|deltaFPR|fuelLevel|tablesSet|lambdaTarget|scondarypulseWidth");
    mySerial.begin(19200);
    lcd.begin(16,2);  
  
    lcd.backlight(); // zalaczenie podwietlenia 
    lcd.setCursor(0,0); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)
    lcd.print("EMU LCD Display");
     delay(500);
     lcd.clear();


    for (int i = 0; i < 255; i++)
    {
        lastChValue[i] = 0;
//        lastChValue_min[i] = UINT16_MAX;
//        lastChValue_max[i] = 0;
    }
    diaplay_trigger.start();

    bytes.clear();
}

void serialEvent_my();
void loop(){
  
   int reading = digitalRead(buttonPin);
// If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

   if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        buttonPressCount++;
      }
    }
  }
    lastButtonState = reading;
   
    if(buttonPressCount > 5){
      buttonPressCount = 0;
  }
   
  

    while(bytes.size() >= ECU_FRAME_LENGTH)
    {
        if (bytes[1] == 0xA3)
        {
            actECUFrame.channel = bytes[ECU_CHANNEL];
            actECUFrame.idChar  = bytes[ECU_IDCHAR];
            actECUFrame.valueH  = bytes[ECU_VALUEH];
            actECUFrame.valueL  = bytes[ECU_VALUEL];
            actECUFrame.cs      = bytes[ECU_CHECKSUM];

      bytes.clear();

            uint8_t cs_calc = ((uint16_t)actECUFrame.channel + actECUFrame.idChar + actECUFrame.valueH + actECUFrame.valueL) % 256;
          //  if (cs_calc == actECUFrame.cs)
            { 
              // Serial.println(cs_calc);
            //Serial.println(actECUFrame.cs);
                //decode frame
                parseECU(); //work on global variable
            }
        }
        else
        {
            bytes.shift();//ignore byte
        }
    }
    diaplay_trigger.update();
    serialEvent_my();

}

void serialEvent_my() {
    int16_t inChar = mySerial.read();
    if (inChar >= 0)
    {
        bytes.push(inChar);
    }
}



