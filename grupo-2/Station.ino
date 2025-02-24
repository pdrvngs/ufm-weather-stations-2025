//BMP280, atmospheric sensor
#include <BMP280_DEV.h>

float temperature, pressure, altitude;
BMP280_DEV bmp280;

// Photoresistor, light sensor
const int photoPin1 = 32;
const int photoPin2 = 35;

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

const char* ssid = "Philip";
const char* pass = "simedas100pesos";

const bool verbose = true;
const bool wifi = false;

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
  Serial.println("🔄 Setup is running...");
  
  bmp280.begin(BMP280_I2C_ALT_ADDR);
  bmp280.setTimeStandby(TIME_STANDBY_500MS);
  dht.begin();

  if(wifi){  // Ensure WiFi is enabled
    WiFi.begin(ssid, pass);
    
    // Wait up to 10 seconds for WiFi connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 100) {  
      delay(100);
      Serial.print(".");
      attempts++;
    }

    if(WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ Connected to WiFi");
      
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
      Serial.println("\n❌ WiFi Connection Failed! Restarting...");
      ESP.restart();
    }
  }

  Serial.println("✅ Setup complete. Sensors ready.");
}

/*Send to Firebase
void sendToFirebase(String data) {
  FirebaseJson json;

      // Parse the data string and structure it into JSON
  json.set("BMP280/Presion", pressure);
  json.set("BMP280/Altitud", altitude);
  json.set("LDR/NivelDeLuz", max(analogRead(photoPin1), max(analogRead(photoPin2), analogRead(photoPin3))));
  json.set("DHT11/Humedad", dht.readHumidity());
  json.set("DHT11/Temperatura", dht.readTemperature());

  float radius = 61.5 / 1000.0;  // Convert mm to meters
  float Kv = 1333.3;
  int lecture = analogRead(motorPin);
  float voltage = lecture * (3.3 / 4095.0);
  float velocity = radius * ((Kv * voltage * (2 * PI)) / 60.0);

  json.set("Anemometer/Voltaje", voltage);
  json.set("Anemometer/VelocidadAire", velocity);

      // Send JSON to Firebase
  if (Firebase.RTDB.setJSON(&fbdo, "/weatherData", json)) {
    Serial.println("✅ Data sent successfully!");
  } else {
  Serial.println("❌ Error sending data: " + fbdo.errorReason());
  }
}*/

void sendToFirebase(String data) {
    if (Firebase.ready()) {
        FirebaseJson json;
        json.set("rawData", data);

        // Print JSON data before sending
        Serial.print("📄 JSON Data: ");
        String jsonStr;
        json.toString(jsonStr, true); // Convert JSON to a string
        Serial.println(jsonStr);

        // Send JSON to Firebase
        if (Firebase.RTDB.setJSON(&fbdo, "/weatherData", &json)) {
            Serial.println("✅ Data sent successfully!");
        } else {
            Serial.print("❌ Error sending data. HTTP Code: ");
            Serial.print(fbdo.httpCode());
            Serial.print(" | Reason: ");
            Serial.println(fbdo.errorReason());
        }
    } else {
        Serial.println("❌ Firebase not ready.");
    }
}




//Print DHT
String readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    if (verbose) {
      Serial.println(F("Failed to read from DHT sensor!"));
    }
    return "Failed to read from DHT sensor!\n";
  }
  if (verbose) {
    Serial.print(F("Humedad: "));
    Serial.print(h);
    Serial.print(F("% Temperatura: "));
    Serial.print(t);
    Serial.println(F("°C "));
  }
  return "Humedad: " + String(h) + "%  Temperatura: " + String(t) + "°C\n";
}

//Print BMP280
String readBMP() {
  if (verbose) {
    Serial.print(F("Presión: "));
    Serial.print(pressure);
    Serial.print(F("hPa   "));

    Serial.print(F("Altitúd: "));
    Serial.print(altitude);
    Serial.println(F("m"));
  }
  return "Presión: " + String(pressure) + " hPa   Altitud: " + String(altitude) + " m\n";
}

//Print Luz
String readLS() {
  int LL1 = analogRead(photoPin1);
  Serial.print(LL1);
  int LL2 = analogRead(photoPin2);
  Serial.print(LL2);

  int lightLevel = max(LL1, LL2);
  Serial.print(lightLevel);

  if (verbose) {
    Serial.print(F("LDR -> Nivel de luz: "));
    Serial.print(lightLevel);
    Serial.println(F(" lux"));
  }
  return "LDR -> Nivel de luz: " + String(lightLevel) + " lux\n";
}

//Print Nemomtr
String readNemo() {
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
  return "Voltaje: " + String(voltage, 2) + " V  |  Velocidad Aire: " + String(velocity, 2) + " m/s\n";
}

// Función para todo al mismo tiempo
String readEVRTNG() {
  bmp280.startNormalConversion();
  while (!bmp280.getMeasurements(temperature, pressure, altitude)) {
    delay(10);  // Small delay to prevent locking up the ESP32
  }
  return readBMP() + readLS() + readDHT() + readNemo();
}

void loop() {
  //Serial.println("🔄 Loop is running...");
  //String str = readEVRTNG();  // Read all sensor data

  Serial.print("Firebase Ready: ");
  Serial.println(Firebase.ready() ? "✅ Yes" : "❌ No");

  String str = "1... 2... 3... probando.";

  if (verbose) {
    Serial.println(str);
  }
  
  sendToFirebase(str);  // Send data as JSON to Firebase

  delay(1000);  // Wait 2 seconds before next reading
}