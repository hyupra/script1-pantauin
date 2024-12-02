#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inisialisasi LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Konfigurasi WiFi
#define SSID "iPhone"
#define SSID_PASSWORD "pengenkayamalaskerja"

// Konfigurasi MQTT
#define DHTTYPE DHT22
#define DHTPIN D7
#define relayPin D3
#define flamePin D6
#define mqPin D0
#define buzzer D5

DHT dht(DHTPIN, DHTTYPE);

// Firebase Firestore configuration
const String FIREBASE_PROJECT_ID = "firedetector-167d8"; // Ganti dengan Project ID Firebase Anda
const String FIREBASE_API_KEY = "AIzaSyBeJx5AYpr56gL4qBqic9jNDgzFnTNKSxc"; // Ganti dengan API Key Anda

WiFiClientSecure client; // Untuk koneksi HTTPS

float hum;  // Menyimpan nilai kelembaban
float temp; // Menyimpan nilai suhu
int flameValue = 0;
int mqValue = 0;
int mqThreshold = 400; // Batasan nilai untuk deteksi gas
String kondisiMq = "";
String kondisiApi = "";

void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(flamePin, INPUT);
  pinMode(mqPin, INPUT);

  lcd.begin();
  lcd.clear();

  // Menghubungkan ke jaringan WiFi
  WiFi.begin(SSID, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Menonaktifkan pemeriksaan sertifikat SSL (hanya untuk pengujian)
  client.setInsecure();
  dht.begin();

  // Menampilkan pesan awal di LCD
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  lcd.setCursor(0, 1);
  lcd.print("PANTAUIN");
  delay(2000);
}

void loop() {
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  flameValue = digitalRead(flamePin);
  mqValue = analogRead(mqPin);

  // Menentukan kondisi MQ
  if (mqValue >= mqThreshold) {
    kondisiMq = "Asap Buruk";
  } else {
    kondisiMq = "Asap Baik";
  }

  // Menentukan kondisi Flame
  if (flameValue == LOW) {
    kondisiApi = "Api Terdeteksi";
    digitalWrite(buzzer, HIGH);
    digitalWrite(relayPin, LOW);
  } else {
    kondisiApi = "Api Tidak Terdeteksi";
    digitalWrite(buzzer, LOW);
    digitalWrite(relayPin, HIGH);
  }

  // Menampilkan data pada LCD secara bergantian
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(hum);
  lcd.print("%");
  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MQ: ");
  lcd.print(kondisiMq);
  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Flame: ");
  lcd.print(kondisiApi);
  delay(1500);

  // Mengirim data ke Firebase Firestore
  sendDataToFirestore();
}

void sendDataToFirestore() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // URL untuk Firestore Sub-collection
    String firestoreUrl = "https://firestore.googleapis.com/v1/projects/" + FIREBASE_PROJECT_ID + "/databases/(default)/documents/Fire/id001/DataAlat";

    // Mempersiapkan JSON data sesuai dengan struktur Firestore
    StaticJsonDocument<300> doc;
    doc["fields"]["Humidity"]["doubleValue"] = hum;
    doc["fields"]["Temperature"]["doubleValue"] = temp;
    doc["fields"]["MQValue"]["stringValue"] = kondisiMq;
    doc["fields"]["FlameDetected"]["stringValue"] = kondisiApi;

    // Serialisasi data JSON
    String jsonData;
    serializeJson(doc, jsonData);

    Serial.println("Sending JSON data to Firestore:");
    Serial.println(jsonData);

    // Konfigurasi permintaan HTTP POST
    http.begin(client, firestoreUrl + "?key=" + FIREBASE_API_KEY);
    http.addHeader("Content-Type", "application/json");

    // Kirim data ke Firestore
    int httpResponseCode = http.POST(jsonData);

    // Cek hasil pengiriman data
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      Serial.println("Error: Failed to send HTTP request.");
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
