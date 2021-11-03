/*********
Features
1. Reading from DHT22 (Temperature, Humidity)
2. Display Temperature and Humidity on OLED Display 128x64
3. Millis Function for adjust message rate
4. LED Alarm for Abnormal Situation(Temperature)
5. WiFi Connection Function
6. MQTT Connection

*********/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Keypad.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "PubSubClient.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 27
#define DHTTYPE    DHT22
DHT dht(DHTPIN, DHTTYPE);
float temp = 0 ;
float humid = 0;

const int led_1 = 26;
int led_1_state = 0;

unsigned long message_rate = 1000;
unsigned long previous_dht_reading = 0;
unsigned long previous_message = 0;
unsigned long current_time = 0;
unsigned long blink_rate = 500;
unsigned long previous_ledtime = 0;
float alarm_temp = 32.0;
float alarm_humid = 60.00;

// Set to your own broker address and topic
const char* mqtt_server = "192.168.0.125";
const char* humidity_topic = "farm/humidity";
const char* temperature_topic = "farm/temperature";
const char* mqtt_username = "embedded";
const char* mqtt_password = "system";
const char* clientID = "client_livingroom";
const char* mqtt_port = "1883";
WiFiManager manager;  
WiFiClient espClient;
PubSubClient client(espClient);

float dht_22(float, float, int);
void oled_display(float, float);
int keypad_input();

void setup() {
  Serial.begin(115200);  
     
  bool success = manager.autoConnect("ESP32_AP","password");
  if(!success) {
      Serial.println("Failed to connect");
  } 
  else {
      Serial.println("Connected");
  } 

  client.setServer(mqtt_server,atoi(mqtt_port)); 

  dht.begin();
  pinMode(led_1, OUTPUT);
  digitalWrite(led_1, LOW);

  connect_MQTT();
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);}
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void loop() {
  current_time = millis();

  temp = dht_22(temp, &humid, message_rate);
  oled_display(temp, humid);
}

float dht_22(float temp,float *humid, int rate){
  if(temp > alarm_temp || *humid > alarm_humid){
    led_alarm();
    Serial.println(temp);
  }else{
   digitalWrite(led_1, LOW);
  }
  if(current_time - previous_dht_reading > rate){ 
  previous_dht_reading = current_time;
  temp = dht.readTemperature();
  *humid = dht.readHumidity();
  if (isnan(temp) || isnan(*humid)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  String hs="Hum: "+String((float)*humid)+" % ";
  String ts="Temp: "+String((float)temp)+" C ";

  if (client.publish(temperature_topic, String(temp).c_str())) {
    // Serial.println("Temperature sent!");
  }
  else {
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(temperature_topic, String(temp).c_str());
  }

  if (client.publish(humidity_topic, String(*humid).c_str())) {
    // Serial.println("Humidity sent!");
  }
  else {
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(humidity_topic, String(*humid).c_str());
  }
  client.disconnect();
  
  return temp;
  }
}

void oled_display(float temp,float humid){
  if(current_time - previous_message > message_rate){
  previous_message = current_time;
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(temp);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
  
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(humid);
  display.print(" %"); 
  
  display.display(); 

  if (isnan(temp) || isnan(humid)) {
    Serial.println("Failed to read from DHT");
  } else {
    Serial.print("Humidity: "); 
    Serial.print(humid);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(temp);
    Serial.println(" *C");
    }
  }
}

void led_alarm(){
    if(current_time - previous_ledtime > blink_rate){
      previous_ledtime = current_time;

      if(led_1_state == HIGH){
      led_1_state = LOW;
    }else{
      led_1_state = HIGH;
    }
    digitalWrite(led_1, led_1_state);
    Serial.println(led_1_state);
    }
}

void connect_MQTT(){
    if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}
