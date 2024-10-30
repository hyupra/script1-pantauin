#include <ESP8266WiFi.h>
#include <DHT.h>

#define DHTPIN D5       // Pin DHT22
#define DHTTYPE DHT22   // Tipe DHT
#define MQ2PIN A0       // Pin MQ2
#define FLAME_PIN D1    // Pin Flame Sensor

DHT dht(DHTPIN, DHTTYPE);

// Tentukan ambang batas untuk deteksi gas
const int thresholdGas = 300; // Sesuaikan nilai ini berdasarkan kalibrasi

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  // Setup Flame Sensor Pin
  pinMode(FLAME_PIN, INPUT);
}

void loop() {
  // Read DHT22 Sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
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

  delay(2000);  // Periksa setiap 2 detik
}
