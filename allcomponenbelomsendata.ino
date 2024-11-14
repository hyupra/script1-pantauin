#include <PubSubClient.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SSID "Arduino uno"
#define SSID_PASSWORD "@Aa112233"

// Konfigurasi MQTT
#define DHTTYPE DHT22
#define DHTPIN D7
#define relayPin D3
#define flamePin D6
#define mqPin D0
#define buzzer D5

DHT dht(DHTPIN, DHTTYPE);

String kondisi = "";
float hum;  // Menyimpan nilai kelembaban
float temp; // Menyimpan nilai suhu
int flameValue = 0;
int mqValue = 0;
int mqThreshold = 400; // Batasan nilai untuk deteksi gas

WiFiClient espClient;
PubSubClient client(espClient);

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

  dht.begin();
}

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  flameValue = digitalRead(flamePin);
  mqValue = analogRead(mqPin); // Menggunakan analogRead untuk nilai sensor MQ

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
  Serial.print("MQ Sensor Value: ");
  Serial.println(mqValue);

  // Kondisi jika flame sensor mendeteksi api
 if (mqValue >= mqThreshold) { // Jika MQ2 mendeteksi asap buruk
    Serial.println("Kondisi Asap Buruk"); // Menampilkan kondisi asap buruk
    kondisi = "Kondisi Asap Buruk";
    } else  { // Jika MQ2 mendeteksi asap baik
    Serial.println("Kondisi Asap Baik"); // Menampilkan kondisi asap baik
    kondisi = "Kondisi Asap Baik";
} 

  if (flameValue == LOW) { // LOW berarti api terdeteksi oleh flame sensor
    Serial.println("Api terdeteksi oleh Flame Sensor");
    kondisi = "Api Terdeteksi";
    digitalWrite(buzzer, HIGH);    // Buzzer aktif
    digitalWrite(relayPin, LOW);   // Relay aktif
    delay(3000);
} 
else { // Kondisi aman
    Serial.println("Aman terkendali");
    kondisi = "Aman Terkendali";
    digitalWrite(buzzer, LOW);     // Buzzer mati
    digitalWrite(relayPin, HIGH);  // Relay non-aktif
    delay(3000);
}

  // script i2c
  lcd.setCursor(4,0);
  lcd.print("Welcome");
  lcd.setCursor(0,1);
  lcd.print("WAHYU CABUL");

  delay(2000); // Delay selama 2 detik sebelum membaca data sensor lagi
}
