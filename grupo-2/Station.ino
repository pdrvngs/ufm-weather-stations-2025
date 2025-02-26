//MAC Address del esp32 "88:13:bf:6f:d1:e4"
const String MacAddress = "MAC";

#include <cmath>

//BMP280, atmospheric sensor
#include <BMP280_DEV.h>

float temperature, pressure, altitude;
BMP280_DEV bmp280;

// Photoresistor, light sensor
const int photoPin1 = 32;
const int photoPin2 = 35;

bool once = true;

// DHT11, Humidity and Temperature Sensor
#include "DHT.h"
#define DHTPIN 19
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Anemometer, Air Speed Sensor
#define PI 3.1416

const int motorPin = 33;  // GPIO33 (D33) en ESP32

//Wifi
#include <WiFi.h>
//Time
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "Philip";
const char* pass = "simedas100pesos";

//const char* ssid = "Ama-gi";
//const char* pass = NULL;

// Initialize NTPClient
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600, 60000); // UTC-6 (Guatemala timezone)

const bool verbose = true;
const bool wifi = true;

//Firebase (database)
#include<Firebase_ESP_Client.h>
#include<addons/TokenHelper.h>
//#include<addons/RTDHelper.h>

#define API_KEY "API"
//"AIzaSyD1r37EmwHCUYTmzbr-TZDqc0tOyia9vzI"

#define USER_EMAIL "philipfalla@ufm.edu"
#define USER_PASSWORD "philip1234"

#define DATABASE_URL "https://weatherstation-23ec0-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  bmp280.begin(BMP280_I2C_ALT_ADDR);
  bmp280.setTimeStandby(TIME_STANDBY_500MS);
  dht.begin();

  if(wifi){  // Ensure WiFi is enabled
    WiFi.begin(ssid, pass);
    
    // Wait up to 10 seconds for WiFi connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 100) {  
      delay(100);
      if(verbose){
        Serial.print(".");
      }
      attempts++;
    }

    if(WiFi.status() == WL_CONNECTED) {
      if(verbose){
        Serial.println("\n✅ Connected to WiFi");
      }

      //get time
      timeClient.begin();
      timeClient.update();
      
      // Firebase Initialization
      config.api_key = API_KEY;
      config.database_url = DATABASE_URL;
      auth.user.email = USER_EMAIL;
      auth.user.password = USER_PASSWORD;
      
      Firebase.reconnectWiFi(true);
      fbdo.setResponseSize(4096);
      config.token_status_callback = tokenStatusCallback;
      config.max_token_generation_retry = 5;

      Firebase.begin(&config, &auth);
    } else {
      if(verbose){
        Serial.println("\n❌ WiFi Connection Failed! Restarting...");
      }
      ESP.restart();
    }
  }
  if(verbose){
    Serial.println("✅ Setup complete. Sensors ready.");
  }
}

void sendToFirebase(String t, float temp, float humid, float pres, float alt, float wind, float light) {
  if (Firebase.ready()) {
      FirebaseJson Time;
      FirebaseJson Data;
      /*
      (Not used anymore)
      FirebaseJson DHT;
      FirebaseJson BMP;
      FirebaseJson Nemo;
      FirebaseJson Light;*/

      // Populate DHT11 JSON (Temperature & Humidity)
      Data.set("Temperature", temp);
      Data.set("Humidity", humid);

      // Populate BMP280 JSON (Pressure & Altitude)
      Data.set("Pressure", pres);
      Data.set("Altitude", alt);

      // Populate Anemometer JSON (Wind Speed)
      Data.set("Wind Speed", wind);

      // Populate Light Sensor JSON (Light Level)
      Data.set("Light Level", light);

      /*
      // Main Data JSON containing all sensor readings (not used anymore)
      Data.set("DHT11", DHT);
      Data.set("BMP280", BMP);
      Data.set("Anemometer", Nemo);
      Data.set("Light Sensor", Light);
      */

      // Assign Data JSON inside Time JSON using timestamp as key
      Time.set(t, Data);

      // Print JSON before sending
      if(verbose){
        Serial.print("📄 JSON Data: ");
        String jsonStr;
        Time.toString(jsonStr, true);  // Convert JSON to a string
        Serial.println(jsonStr);
      }

      // Send JSON to Firebase
      if (Firebase.RTDB.setJSON(&fbdo, "/LosMejores", &Time)) { 
        if(verbose){
          Serial.println("✅ Data sent successfully!");
        }
      } else {
        if(verbose){
          Serial.print("❌ Error sending data. HTTP Code: ");
          Serial.print(fbdo.httpCode());
          Serial.print(" | Reason: ");
          Serial.println(fbdo.errorReason());
        }
      }
  } else {
    if(verbose){
      Serial.println("❌ Firebase not ready.");
    }
  }
}

//Reset whole database
void resetFirebase() {
  Firebase.RTDB.deleteNode(&fbdo, "/");
}

//Print DHT
float readH() {
  float h = dht.readHumidity();

  if (isnan(h)) {
    if (verbose) {
      Serial.println(F("Failed to read humidity from DHT sensor!"));
    }
    return NULL;
  }
  if (verbose) {
    Serial.print(F("Humedad: "));
    Serial.print(h);
    Serial.print(F(" %"));
  }
  return h;
  //OLD: return String(h) + "% ";
}

float readT() {
  float t = dht.readTemperature();

  if (isnan(t)) {
    if (verbose) {
      Serial.println(F("Failed to read temperature from DHT sensor!"));
    }
    return NULL;
  }
  if (verbose) {
    Serial.print(F("Temperatura: "));
    Serial.print(t);
    Serial.println(F("°C "));
  }
  return t;
  //OLD: return String(t) + "°C\n";
}

//Print BMP280
float readAlt() {
  float a = altitude;
  
  if (verbose) {
    Serial.print(F("Altitúd: "));
    Serial.print(a);
    Serial.println(F("m"));
  }
  return a;
  //OLD: return String(a) + " m\n";
}

float readPr() {
  float p = pressure;
  
  if (verbose) {
    Serial.print(F("Presión: "));
    Serial.print(p);
    Serial.print(F("hPa   "));
  }
  return p;
  //OLD: return String(p) + " hPa";
}


//Print Luz
float readLS() {

  int LL1 = analogRead(photoPin1);
  if(verbose){
    Serial.print(LL1);
  }

  int LL2 = analogRead(photoPin2);
  if(verbose){
  Serial.print(LL2);
  }

  int lightLevel = max(LL1, LL2);

  if(verbose){  
    Serial.print(lightLevel);
  }

  if (verbose) {
    Serial.print(F("LDR -> Nivel de luz: "));
    Serial.print(lightLevel);
    Serial.println(F(" lux"));
  }
  return float(lightLevel);
  //OLD: return String(lightLevel) + " lux\n";
}

//Print Nemomtr
float readNemo() {
  float radius = 61.5;  // Radio en milímetros
  float Kv = 1333.3;    // Kv en RPM/V

  int lecture = analogRead(motorPin);
  float voltage = lecture * (3.3 / 4095.0);  // Ajustado para el rango 0-4095 del ESP32

  // Convertir el radio de mm a metros
  float radiusInMeters = radius / 1000.0;

  // Calcular la velocidad tangencial en m/s
  float velocity = radiusInMeters * ((Kv * voltage * (2 * PI)) / 60.0);

  // Imprimir resultados por el puerto serie
  if (verbose) {
    Serial.print("Voltaje: ");
    Serial.print(voltage, 2);  // Mostrar con 2 decimales
    Serial.print(" V  |  Velocidad Aire: ");
    Serial.print(velocity, 2);  // Mostrar con 2 decimales
    Serial.println(" m/s");
  }
  return roundU(velocity);
}

//round nearest 2 decimales
float roundU(float x){
  return round(x * 100.0) / 100.0;
}

// Función para todo al mismo tiempo
bool ready() {
  bmp280.startNormalConversion();
  while (!bmp280.getMeasurements(temperature, pressure, altitude)) {
    delay(10);  // Small delay to prevent locking up the ESP32
    return false;
  }
  return true;
}

String getFormattedTime() {
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);
    
    char timeString[30];
    snprintf(timeString, sizeof(timeString), "%04d-%02d-%02d %02d:%02d:%02d",
             ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
             ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    return String(timeString);
}

bool checkWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true; // WiFi ya está conectado
  }

  if (verbose){
    Serial.println("⚠️ WiFi desconectado. Intentando reconectar...");
  }

  WiFi.disconnect();
  WiFi.reconnect();

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    if(verbose){
      Serial.print(".");
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    if(verbose){
      Serial.println("\n✅ WiFi reconectado.");
    }
    return true;
  } else {
    if(verbose){
      Serial.println("\n❌ No se pudo reconectar a WiFi.");
    }
    return false;
  }
}

void loop() {
  if (!checkWiFi()) {
    if(verbose){
      Serial.println("❌ Esperando conexión a WiFi antes de continuar...");
    }
    delay(5000);  // Esperar antes de volver a intentarlo
    return;
  }

  timeClient.update();
  String TT;
  
  if(ready()){
    TT = getFormattedTime();
    float Humid = readH();
    float Temp = readT();
    float Alt = readAlt();
    float Pr = readPr();
    float LS = readLS();
    float Nemo = readNemo();
    sendToFirebase(TT, Temp, Humid, Pr, Alt, Nemo, LS); 
  }

  delay(2000);  // Wait 2 seconds before next reading
}