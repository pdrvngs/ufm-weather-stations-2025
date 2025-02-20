#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; 

void setup() {
  Serial.begin(9600);
  if (!bmp.begin(0x76)) {  // Cambio a 0x76 el escáner lo detectó ahí
    Serial.println("Error al detectar el sensor BMP280");
    while (1);
  }
  Serial.println("BMP280 detectado correctamente");
}

void loop() {
  Serial.print("Temperatura: ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("Presión: ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println(" hPa");

  delay(10000);
}

