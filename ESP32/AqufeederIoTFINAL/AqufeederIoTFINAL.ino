#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h> 

// Wifi
const char* ssid = "PujaKokomi"; 
const char* password = "KokomiWangy"; 

// YRL Server
const char* serverUrl = "http://34.101.35.237:5000/";

// Define pin
const int trigPin = 5;
const int echoPin = 18;
const int servoPin = 13;
const int buzzerPin = 25;

// Kalibrasi tempat penyimpanan
const float tankHeight = 20.0; // tinggi wadah
const float minDistance = 4.0; // jarak makanan ke wadah kalau penuh
const float maxDistance = tankHeight - minDistance; // jarak makanan ke wadah kalau kosong

// Variabel
Servo feederServo;
float foodPercentage = 0;
bool buzzerOn = false;
bool feedOn = false;
unsigned long lastFeedTime = 0;
const long feedDuration = 100; // waktu pemberian pakan 

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi pin
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  feederServo.attach(servoPin);
  
  // Close gate initially
  closeGate();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
    return;
  }
  
  readSensor();
  sendSensorData();
  getControlCommands();

  if (feedOn) {
    feedFish();
  }

  digitalWrite(buzzerPin, buzzerOn ? HIGH : LOW);
  
  delay(2000);
}

void reconnectWiFi() {
  Serial.println("WiFi disconnected. Reconnecting...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  delay(5000);
}

void readSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  
  if (distance <= minDistance) {
    foodPercentage = 100.0;
  } else if (distance >= maxDistance) {
    foodPercentage = 0.0;
  } else {
    foodPercentage = 100.0 * (1.0 - (distance - minDistance) / (maxDistance - minDistance));
  }

  Serial.print("Food Percentage: ");
  Serial.println(foodPercentage);
}

void sendSensorData() {
  WiFiClient client;

  HTTPClient http;
  http.begin(client, String(serverUrl) + "/update_sensor");
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(128);
  doc["percentage"] = foodPercentage;
  String jsonString;
  serializeJson(doc, jsonString);

  int httpCode = http.POST(jsonString);
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Sensor data sent successfully");
  } else {
    Serial.printf("Error sending sensor data: %d - %s\n", httpCode, http.errorToString(httpCode).c_str());
  }

  http.end();
}

void getControlCommands() {
  WiFiClient client;

  HTTPClient http;
  http.begin(client, String(serverUrl) + "/get_control");

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      buzzerOn = doc["buzzer"] == "on";
      feedOn = doc["feed"] == "on";

      Serial.print("Buzzer: ");
      Serial.println(buzzerOn ? "ON" : "OFF");
      Serial.print("Feed: ");
      Serial.println(feedOn ? "ON" : "OFF");
    } else {
      Serial.println("JSON parse error");
    }
  } else {
    Serial.printf("Error getting control commands: %d - %s\n", httpCode, http.errorToString(httpCode).c_str());
  }

  http.end();
}

void feedFish() {
  Serial.println("Feeding fish...");
  openGate();
  delay(feedDuration);
  closeGate();
  feedOn = false;
}

void openGate() {
  feederServo.write(90);
  Serial.println("Gate opened");
}

void closeGate() {
  feederServo.write(0);
  Serial.println("Gate closed");
}
