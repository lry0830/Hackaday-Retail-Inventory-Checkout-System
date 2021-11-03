/** Features
 *  1. LCS begin
 *  2. Show Name and Price
 *  3. WIFI
 *  4. MQTT
 *  5. Price Changing
 *  6. Product Name Changing
 */

#include <LiquidCrystal_I2C.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "PubSubClient.h"

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// Set to your own broker address and topic
const char* mqtt_server = "192.168.0.125";
const char* price_topic = "farm/price_1";
const char* product_topic = "farm/product_1";
const char* mqtt_username = "embedded";
const char* mqtt_password = "system";
const char* clientID = "client_livingroom";
const char* mqtt_port = "1883";
WiFiManager manager;  
WiFiClient espClient;
PubSubClient client(espClient);

// Default Text
String productName = "Orange";
String productPrice = "1.00";

void setup(){
  Serial.begin(115200);  
     
  bool success = manager.autoConnect("ESP32_AP","password");
  if(!success) {
      Serial.println("Failed to connect");
  } 
  else {
      Serial.println("Connected");
  }

  client.setServer(mqtt_server,atoi(mqtt_port)); 
  connect_MQTT();
  client.setCallback(callback);
  
  lcd.begin();                     
  lcd.backlight();
  lcd.clear();
}

void loop(){
  printScreen();
  client.loop();
}

void connect_MQTT(){
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  if(client.subscribe(price_topic)){
    Serial.println("Connected to Topic");
  }
  if(client.subscribe(product_topic)){
    Serial.println("Connected to Topic");
  }
}

void printScreen(){
  lcd.setCursor(0, 0);
  lcd.print(productName);
  lcd.setCursor(0,1);
  lcd.print("RM");
  lcd.setCursor(2,1);
  lcd.print(productPrice);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String receivedMessage;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    receivedMessage += (char)message[i];
  }
  Serial.println();
  
  if (String(topic) == price_topic) {
    Serial.print("Changing output to ");
    Serial.print(receivedMessage);
    updatePrice(receivedMessage);
  }

  if (String(topic) == product_topic) {
    Serial.print("Changing output to ");
    Serial.print(receivedMessage);
    updateProductName(receivedMessage);
  }
}

void updatePrice(String price){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(productName);
  lcd.setCursor(0,1);
  lcd.print("RM");
  lcd.setCursor(2,1);
  lcd.print(price);
  productPrice = price;
}

void updateProductName(String productname){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(productname);
  lcd.setCursor(0,1);
  lcd.print("RM");
  lcd.setCursor(2,1);
  lcd.print(productPrice);
  productName = productname;
}
