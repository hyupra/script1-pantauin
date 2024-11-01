#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define DHTPIN D5       // Pin DHT22
#define DHTTYPE DHT22   // Tipe DHT
#define MQ2PIN A0       // Pin MQ2
#define FLAME_PIN D1    // Pin Flame Sensor

DHT dht(DHTPIN, DHTTYPE);

// Tentukan ambang batas untuk deteksi gas
const int thresholdGas = 300; // Sesuaikan nilai ini berdasarkan kalibrasi

// WiFi Credentials
const char* SSID = "iPhone";  // Ganti dengan SSID WiFi Anda
const char* SSID_PASSWORD = "pengenkayamalaskerja";  // Ganti dengan password WiFi Anda

// Firebase Firestore configuration
const String FIREBASE_PROJECT_ID = "firedetector-167d8"; // Ganti dengan Project ID Firebase Anda
const String FIREBASE_API_KEY = "AIzaSyBeJx5AYpr56gL4qBqic9jNDgzFnTNKSxc"; // Ganti dengan API Key Anda

WiFiClientSecure client;  // Menggunakan WiFiClientSecure untuk HTTPS

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  // Setup Flame Sensor Pin
  pinMode(FLAME_PIN, INPUT);

  // Menghubungkan ke WiFi
  WiFi.begin(SSID, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Menonaktifkan pemeriksaan sertifikat SSL (hanya untuk pengujian)
  client.setInsecure();
}

void loop() {
  // Read DHT22 Sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Validasi apakah data sensor terbaca dengan benar
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return; // Jika gagal, keluar dari loop untuk mencegah pengiriman data yang tidak valid
  }

  // Read MQ2 Gas Sensor
  int mq2Value = analogRead(MQ2PIN);
  
  // Read Flame Sensor
  int flameDetected = digitalRead(FLAME_PIN) == LOW; // Sesuaikan jika logika HIGH digunakan saat api terdeteksi
  
  // Deteksi Kebakaran Berdasarkan Kondisi Sensor
  if (mq2Value > thresholdGas || flameDetected) {
    Serial.println("Peringatan! Kebakaran terdeteksi.");
  }

  // Tampilkan Data pada Serial Monitor
  Serial.print("Kelembaban: ");
  Serial.print(h);
  Serial.print(" %  ");
  Serial.print("Suhu: ");
  Serial.print(t);
  Serial.print(" C  ");
  Serial.print("Gas Level: ");
  Serial.print(mq2Value);
  Serial.print(" Flame Detected: ");
  Serial.println(flameDetected);

  // Kirim data ke Firestore
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Membangun URL API Firestore
    String firestoreUrl = "https://firestore.googleapis.com/v1/projects/" + FIREBASE_PROJECT_ID + "/databases/(default)/documents/api"; // Ganti "fire" dengan nama koleksi Anda

    // Menyusun data JSON untuk Firestore
    StaticJsonDocument<300> doc;
    doc["fields"]["Temp"]["doubleValue"] = t;
    doc["fields"]["Hum"]["doubleValue"] = h;
    doc["fields"]["GasLevel"]["integerValue"] = mq2Value;
    doc["fields"]["FlameDetected"]["booleanValue"] = flameDetected;

    String jsonData;
    serializeJson(doc, jsonData);

    Serial.println("Sending JSON data: ");
    Serial.println(jsonData); // Debug: Tampilkan data JSON

    http.begin(client, firestoreUrl + "?key=" + FIREBASE_API_KEY);  // Memulai request dengan URL dan API Key
    http.addHeader("Content-Type", "application/json");

    // Mengirim permintaan POST ke Firestore
    int httpResponseCode = http.POST(jsonData); // Firestore menggunakan POST untuk menambahkan dokumen

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode); // Menampilkan kode error untuk diagnosis
      Serial.println("Error: Failed to send HTTP request.");
    }

    http.end(); // Akhiri permintaan HTTP
  } else { 
    Serial.println("WiFi Disconnected");
  }

  delay(5000); // Delay 5 detik sebelum membaca sensor lagi
}
