/*
AUTOMATION TURN ON/OFF SYSTEM FOLLOW TEMPERATURE FOR DAIKIN AIR CONDITIONING
Author: namdothanh (namdothanh87@gmail.com)
Date: 5/2017
Hardware: 
  + Arduino Uno Rev3  
  + DS18B20 sensor 
  + IR Transmit LED
Software: Arduino IDE
*/

#include <IRremote.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/* SYSTEM STATUS (FSM) */
#define STATUS_OFF              0
#define STATUS_ON               1
int status = STATUS_OFF;                  // Warning: Default status is OFF
const double TEMP_TURN_ON = 33.0f;
const double TEMP_TURN_OFF = 28.0f;

/* IR REMOTE SENDER */
const int PIN_SEND_IR = 3;      //Default                          
IRsend irsend;

/* DAIKIN CONDITION AIR - 64 bit*/
static const unsigned int DAIKIN_CONDITION_POWER_RAW[] = {
  9800, 9800, 9800, 9800, 
  4640, 2540, 
  390, 390,     //0
  390, 925,     //1
  390, 925,     //1
  390, 390,     //0
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 925,     //1
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 925,     //1
  390, 925,     //1
  390, 925,     //1
  390, 390,     //0
  390, 390,     //0
  390, 925,     //1
  390, 390,     //0
  390, 20200,
  4640};
  
/* DS18B20 Sensor */
#define NUMBER_MEASURE_TEMP     10
#define ONE_WIRE_BUS 2                    // Data wire is plugged into port 2 on the Arduino
OneWire oneWire(ONE_WIRE_BUS);            // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs) 
DallasTemperature sensors(&oneWire);      // Pass our oneWire reference to Dallas Temperature.
                                                        
void pressPowerKey() {
  irsend.sendRaw( DAIKIN_CONDITION_POWER_RAW, 
                  sizeof(DAIKIN_CONDITION_POWER_RAW)/sizeof(unsigned int), 
                  38 /* kHz */);
  delay(1000);                
}

double measureTemperature() {

  double temp = 0.0f;
  
  for(int i=0; i < NUMBER_MEASURE_TEMP; ++i) {
    sensors.requestTemperatures();            // Send the command to get temperatures
    temp += sensors.getTempCByIndex(0);
  }  
  temp /= NUMBER_MEASURE_TEMP;
  
  Serial.print("Temperature = ");
  Serial.print(temp);
  Serial.println(" (C)");
  
  //if met an error with ds18b20 sensor, alway turn off condition air
  //if(temp <= DEVICE_DISCONNECTED_C /* -127 */) {  
  if((temp < 15) || (temp > 45)) {  
    Serial.println("DS18B20 sensor is disconnected or error.");
    return (TEMP_TURN_OFF - 1);
  }
  return temp;  
}

void processSystem() {
  switch(status) {
    
    case STATUS_OFF:
      if(measureTemperature() > TEMP_TURN_ON) {
        pressPowerKey();
        status = STATUS_ON;
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("Action: Turn on");
      }
      break;
    
    case STATUS_ON:
      if(measureTemperature() < TEMP_TURN_OFF) {
        pressPowerKey();
        status = STATUS_OFF;
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("Action: Turn off");
      }
      break;
      
    default:
      break;
  };
  if(status == STATUS_ON) {
    Serial.println("Status: ON");    
  }
  else {
    Serial.println("Status: OFF");    
  }
  
}

void setup() {
  
  //start Serial port
  Serial.begin(115200);
  Serial.println("------------------------------------------------------\r\n"
                 "Automation Turn On/Off System for DAIKIN Air Conditioning");
  
  //start LED indicator
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  
  
  // Start up the DS18b20 & Onewire library
  sensors.begin();
  sensors.setResolution(12);
}

void loop() {
  //transmit uart
  Serial.println("......");
  
  //process system
  processSystem();
  
}
