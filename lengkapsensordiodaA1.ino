#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_INA219.h>
#include <Adafruit_SHT31.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h> //Nama library: Firebase Arduino Client Library for ESP8266 and ESP32
#include <WiFi.h>
#include "time.h"

// Konfigurasi pin dan sensor
#define BMP_SCK  13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS   10
#define anemometerPin 13

// Inisialisasi sensor dan LCD
Adafruit_BMP280 bmp;
Adafruit_INA219 ina219(0x45);
Adafruit_SHT31 sht31 = Adafruit_SHT31();
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//Nama Wifi
#define WIFI_SSID "SSID WIFI"
#define WIFI_PASSWORD "PASSWORD WIFI" 

// Insert Firebase project API Key
#define API_KEY "AIzaSyDCxtH5fU2Mi3oeITc_K0yHyS0N030A1RY"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "anginmelayu@gmail.com"
#define USER_PASSWORD "anginmelayu"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://angin-melayu-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes. 
String voltPath = "/tegangan";
String suhuPath = "/suhu";
String humPath = "/kelembaban";
String pressPath = "/tekanan";
String tinggiPath = "/ketinggian";
String windPath = "/windspeed";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;//Millis untuk mengirim ke firebase
unsigned long timerDelay = 180000; //Mengirim ke Firebase tiap 3 menit. bisa diganti.

// Variabel untuk anemometer
volatile int rotationCount = 0;
float windSpeed = 0.0;
unsigned long lastTime = 0;
const float conversionFactor = 2.4; // Sesuaikan sesuai spesifikasi anemometer

// Variabel tampilan LCD
int displayState = 0;

void IRAM_ATTR countRotation() {
  rotationCount++;
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);
  Serial.println(F("Inisialisasi sensor dan anemometer"));

  // Inisialisasi LCD
  lcd.begin(20, 4);
  lcd.init();
  lcd.backlight();

  // Inisialisasi sensor INA219
  if (!ina219.begin()) {
    Serial.println("Tidak dapat mendeteksi sensor INA219, periksa koneksi!");
    lcd.setCursor(0, 0);
    lcd.print("INA219 tidak ditemukan!");
    while (1) delay(1);
  }
  Serial.println("Sensor INA219 ditemukan!");

  // Inisialisasi sensor SHT31
  if (!sht31.begin(0x44)) {
    Serial.println("Sensor SHT31 tidak ditemukan!");
    lcd.setCursor(0, 1);
    lcd.print("SHT31 tidak ditemukan!");
    while (1) delay(1);
  }

  // Inisialisasi sensor BMP280
  if (!bmp.begin(0x76)) {
    Serial.println(F("BMP280 tidak ditemukan!"));
    lcd.setCursor(0, 2);
    lcd.print("BMP280 tidak ditemukan!");
    while (1) delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

  // Inisialisasi anemometer
  pinMode(anemometerPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(anemometerPin), countRotation, FALLING);

  // WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");

  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/dataSensor/" + uid;
}

void loop() {
  // Baca data dari sensor INA219
  float busVoltage = ina219.getBusVoltage_V();

  // Baca data dari sensor SHT31
  float suhuSHT31 = sht31.readTemperature();
  float kelembaban = sht31.readHumidity();

  // Baca data dari sensor BMP280
  float tekanan = bmp.readPressure();
  float ketinggian = bmp.readAltitude(1013.25);

  // Hitung kecepatan angin
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 1000) {
    detachInterrupt(digitalPinToInterrupt(anemometerPin));
    windSpeed = (rotationCount / 1.0) * conversionFactor;
    rotationCount = 0;
    lastTime = currentTime;
    attachInterrupt(digitalPinToInterrupt(anemometerPin), countRotation, FALLING);
  }

  // Tampilan bergantian pada LCD
  lcd.clear();
  if (displayState == 0) {
    // Tampilkan data tegangan dari INA219
    lcd.setCursor(0, 0);
    lcd.print("Tegangan Baterai:");
    lcd.setCursor(0, 1);
    lcd.print(busVoltage);
    lcd.print(" V");

    Serial.println("=== Data INA219 ===");
    Serial.print("Tegangan Baterai: ");
    Serial.print(busVoltage);
    Serial.println(" V");

  } else if (displayState == 1) {
    // Tampilkan data suhu dan kelembaban dari SHT31
    lcd.setCursor(0, 0);
    lcd.print("Suhu(SHT31): ");
    lcd.print(suhuSHT31);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Kelembaban: ");
    lcd.print(kelembaban);
    lcd.print(" %");

    Serial.println("=== Data SHT31 ===");
    Serial.print("Suhu: ");
    Serial.print(suhuSHT31);
    Serial.println(" C");
    Serial.print("Kelembaban: ");
    Serial.print(kelembaban);
    Serial.println(" %");

  } else if (displayState == 2) {
    // Tampilkan data tekanan dan ketinggian dari BMP280
    lcd.setCursor(0, 0);
    lcd.print("Tekanan: ");
    lcd.print(tekanan / 100); // dalam hPa
    lcd.print(" hPa");

    lcd.setCursor(0, 1);
    lcd.print("Ketinggian: ");
    lcd.print(ketinggian);
    lcd.print(" m");

    Serial.println("=== Data BMP280 ===");
    Serial.print("Tekanan: ");
    Serial.print(tekanan);
    Serial.println(" Pa");
    Serial.print("Ketinggian: ");
    Serial.print(ketinggian);
    Serial.println(" m");

  } else if (displayState == 3) {
    // Tampilkan data kecepatan angin dari anemometer
    lcd.setCursor(0, 0);
    lcd.print("Kecepatan Angin: ");
    lcd.setCursor(0, 1);
    lcd.print(windSpeed);
    lcd.print(" m/s");

    Serial.println("=== Data Anemometer ===");
    Serial.print("Kecepatan Angin: ");
    Serial.print(windSpeed);
    Serial.println(" m/s");
  }

  // Perbarui displayState untuk bergantian menampilkan data
  displayState = (displayState + 1) % 4;

  // Mnengirim data ke firebase database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    json.set(voltPath.c_str(), busVoltage);
    json.set(suhuPath.c_str(), suhuSHT31);
    json.set(humPath.c_str(), kelembaban);
    json.set(pressPath.c_str(), tekanan);
    json.set(tinggiPath.c_str(), ketinggian);
    json.set(windPath.c_str(), windSpeed);
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
  delay(2000); // Tampilkan setiap data selama 2 detik
}
