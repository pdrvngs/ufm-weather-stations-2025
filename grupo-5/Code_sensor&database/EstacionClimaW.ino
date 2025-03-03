#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <AHTxx.h>
#include <FirebaseESP32.h>
#include <time.h>  // Para obtener la fecha y hora

// Credenciales Wi-Fi
#define WIFI_SSID "WiFi gratis pero solo amigos"
#define WIFI_PASSWORD "123propanotriol"

// Firebase (Usando autenticación con email y password)
#define FIREBASE_HOST "https://weatherstation-23ec0-default-rtdb.firebaseio.com/"
#define FIREBASE_API_KEY "AIzaSyD1r37EmwHCUYTmzbr-TZDqc0tOyia9vzI"
#define USER_EMAIL "Juandacg@ufm.edu"
#define USER_PASSWORD "juan1234"

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

Adafruit_BMP280 bmp;
AHTxx aht10(AHTXX_ADDRESS_X38, AHT1x_SENSOR);

// Pines
#define LDR_PIN 33
#define BARO_PIN 34
#define ANEMO_PIN 32

// Variables del anemómetro
volatile int conteoPulsos = 0;
unsigned long tiempoInicio = 0;
const float factorConversion = 2.4;

void IRAM_ATTR contarPulsos() {
    conteoPulsos++;
}

// Función para obtener la fecha y hora actual en formato "YYYY-MM-DD_HH:MM:SS"
String obtenerFechaHora() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Error al obtener la hora");
        return "0000-00-00_00:00:00";
    }
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d_%02d:%02d:%02d", 
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, 
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return String(buffer);
}

void setup() {
    Serial.begin(115200);
    
    // Conectar WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConectado a WiFi!");

    // Sincronizar hora con un servidor NTP
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    // Configurar Firebase
    config.api_key = FIREBASE_API_KEY;
    config.database_url = FIREBASE_HOST;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    
    Serial.println("Autenticando en Firebase...");
    while (!Firebase.ready()) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nAutenticado en Firebase!");

    Wire.begin(21, 22);

    if (!bmp.begin(0x76)) {
        Serial.println("Error: No se detecta el BMP280.");
        while (1);
    }
    if (!aht10.begin()) {
        Serial.println("Error: No se detecta el AHT10.");
        while (1);
    }

    pinMode(ANEMO_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ANEMO_PIN), contarPulsos, FALLING);
    tiempoInicio = millis();
}

void loop() {
    float temperatura = aht10.readTemperature();
    float humedad = aht10.readHumidity();
    float presion = bmp.readPressure() / 100.0;
    int luz = analogRead(LDR_PIN);
    
    float tiempoTranscurrido = (millis() - tiempoInicio) / 1000.0;
    float velocidadViento = (conteoPulsos / tiempoTranscurrido) * factorConversion;

    conteoPulsos = 0;
    tiempoInicio = millis();

    String timestamp = obtenerFechaHora();

    Serial.println("=================================");
    Serial.println("Grupo: Brazos_al_pastor");
    Serial.print("Fecha y Hora: "); Serial.println(timestamp);
    Serial.print("Temperatura: "); Serial.print(temperatura); Serial.println(" °C");
    Serial.print("Humedad: "); Serial.print(humedad); Serial.println(" %");
    Serial.print("Presión: "); Serial.print(presion); Serial.println(" hPa");
    Serial.print("Luz: "); Serial.println(luz);
    Serial.print("Velocidad del viento: "); Serial.print(velocidadViento); Serial.println(" m/s");
    Serial.println("=================================\n");

    FirebaseJson json;
    json.set("temperatura", temperatura);
    json.set("humedad", humedad);
    json.set("presion", presion);
    json.set("luz", luz);
    json.set("velocidad_viento", velocidadViento);

    String path = "/Brazos_al_pastor/" + timestamp;
    if (Firebase.pushJSON(firebaseData, path, json)) {
        Serial.println("Datos enviados a Firebase correctamente!");
    } else {
        Serial.println("Error al enviar datos a Firebase.");
        Serial.println(firebaseData.errorReason());
    }

    delay(2000);
}

