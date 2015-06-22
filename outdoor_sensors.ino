#include <LowPower.h>
#include <stdlib.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#define SSID "MySSID"
#define PASS "MYWPAKEY"
#define IP "192.168.0.1"
String GET = "GET /temp.php?location=outside&temp=";
SoftwareSerial monitor(10, 11); // RX, TX
char inData[64];
char inChar=-1;

/* Program to read various inputs from Arduino in my shed!

   D8 = dallas 1-wire temperature sensor
   D10, D11  = serial connection to wireless unit
   A0      = analog in for soil moisture
   A1      = analog in for rain sensor
   A2      = analog in for light dependant resistor (10k resistor in parallel to 5v)
   D9      = digital in for high level water float
   D5      = digital in for low water level float
   
   Digital outputs
   D7      = power supply to soil moisture
   D6      = power supply to rain sensor
   
*/

void setup()
{
  monitor.begin(9600);
  Serial.begin(9600);
  pinMode(A0, INPUT); // Soil moisture
  pinMode(A1, INPUT); // Rain sensor
  pinMode(A2, INPUT); // LDR
  pinMode(7, OUTPUT); // power to soil moisture
  pinMode(6, OUTPUT); // power to rain sensor
  pinMode(9, INPUT); // high level water float
  pinMode(5, INPUT); // low level water float
  Serial.println("Starting up");
  sensors.begin();
  sendDebug("AT");
  delay(5000);
  if(monitor.find("OK")){
    Serial.println("RECEIVED: OK");
    connectWiFi();
  } else {
     Serial.println("ERROR no OK found");
  };
}

void loop(){
  
  // Temperature for 1-wire
  Serial.println("Request temperatures");
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  char buffer[10];
  String temp_out = dtostrf(tempC, 4, 1, buffer);
  Serial.print("Temperature: ");
  Serial.println(temp_out);
  
  // Soil moisture
  Serial.println("Request soil moisture");
  int s1 = analogRead(A0);
  Serial.print("Moisture: ");
  Serial.println(s1);
  String s1_out = String(s1);
  if(s1 >= 1000) {
    Serial.println("Sensor not in the ground or error!");
  } else {
    if(s1 < 1000 && s1 >= 600) { 
       Serial.println("Soil is DRY"); 
    };
    if(s1 < 600 && s1 >= 370) {
       Serial.println("Soil is HUMID"); 
    };
    if(s1 < 370) {
       Serial.println("Sensor in WATER");
    };
  };

  // Rain Sensor
  Serial.println("Request rain sensor");
  int s2 = analogRead(A1);
  Serial.print("Rain: ");
  Serial.println(s2);
  String s2_out = String(s2);
  if (s2 < 50) {
      Serial.println("Rain flooded");
  };
  if (s2 > 150 && s2 < 500) {
      Serial.println("Raining");
  };
  
  // LDR
  Serial.println("Request Light levels");
  int s3 = analogRead(A2);
  Serial.print("Light: ");
  Serial.println(s3);
  String s3_out = String(s3);
  if (s3 > 300) {
      Serial.println("Light bright");
  };
  if (s3 > 50 && s3 < 300) {
      Serial.println("Light normal");
  };
  if (s3 < 50) {
      Serial.println("Light dark");
  };

  // high level water float
  Serial.println("Request water level highlevel");
  int s4 = digitalRead(9);
  String s4_out = String(s4);
  Serial.print("High level float: ");
  Serial.println(s4);

  // low level water float
  Serial.println("Request water level lowlevel");
  int s5 = digitalRead(5);
  String s5_out = String(s5);
  Serial.print("Low level float: ");
  Serial.println(s5);
  
  updateTemp(temp_out,s1_out,s2_out,s3_out,s4_out,s5_out);
  delay(5000);
  getSignalStrength();
  // Sleep for 8 s with ADC module and BOD module off
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  // delay(60000);
}

void updateTemp(String tenmpF,String as1,String as2,String as3,String as4,String as5){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  sendDebug(cmd);
  delay(2000);
  if(monitor.find("Error")){
    Serial.print("RECEIVED: Error");
    return;
  }
  cmd = GET;
  cmd += tenmpF;
  cmd += "&soil=";
  cmd += as1;
  cmd += "&rain=";
  cmd += as2;
  cmd += "&light=";
  cmd += as3;
  cmd += "&highlevel=";
  cmd += as4;
  cmd += "&lowlevel=";
  cmd += as5;
  cmd += "\r\n";
  monitor.print("AT+CIPSEND=");
  monitor.println(cmd.length());
  if(monitor.find(">")){
    Serial.print(">");
    Serial.print(cmd);
    monitor.print(cmd);
  }else{
    sendDebug("AT+CIPCLOSE");
  }
  if(monitor.find("OK")){
    Serial.println("RECEIVED: OK");
  }else{
    Serial.println("RECEIVED: Error");
  }
}
void sendDebug(String cmd){
  Serial.print("SEND: ");
  Serial.println(cmd);
  monitor.println(cmd);
} 
 
boolean connectWiFi(){
  monitor.println("AT+CWMODE=1");
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  sendDebug(cmd);
  delay(5000);
  if(monitor.find("OK")){
    Serial.println("RECEIVED: OK");
    return true;
  }else{
    Serial.println("RECEIVED: Error");
    return false;
  }
}

void getSignalStrength() {
  monitor.println("AT+CWLAP");
  delay(3000);
  byte numBytesAvailable= monitor.available();
  if (numBytesAvailable > 0){
        // store everything into "inData"
        int i;
        for (i=0;i<numBytesAvailable;i++){
            inChar= monitor.read();
            inData[i] = inChar;
        }

        inData[i] = '\0';


        Serial.print("Arduino Received: ");
        Serial.println(inData);
    }
  numBytesAvailable= monitor.available();
  if (numBytesAvailable > 0){
        // store everything into "inData"
        int i;
        for (i=0;i<numBytesAvailable;i++){
            inChar= monitor.read();
            inData[i] = inChar;
        }

        inData[i] = '\0';


        Serial.print("Arduino Received: ");
        Serial.println(inData);
    }
}

