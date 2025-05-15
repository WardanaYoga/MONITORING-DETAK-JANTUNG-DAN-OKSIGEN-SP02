#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

uint32_t irBuffer[100];  // IR LED sensor data
uint32_t redBuffer[100]; // Red LED sensor data

int32_t bufferLength; 
int32_t spo2; 
int8_t validSPO2; 
int32_t heartRate; 
int8_t validHeartRate;

void setup()
{
  Serial.begin(115200);
  delay(1000); // beri waktu serial terbuka

  // Inisialisasi I2C pada pin default ESP32 (GPIO21 = SDA, GPIO22 = SCL)
  Wire.begin(21, 22); // SDA, SCL

  // Inisialisasi sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 tidak ditemukan. Cek wiring dan power.");
    while (1);
  }

  Serial.println("Pasang jari pada sensor dan tekan tombol apapun di Serial Monitor...");
  //while (Serial.available() == 0); // tunggu tombol ditekan
  //Serial.read();

  // Konfigurasi sensor
  byte ledBrightness = 60;
  byte sampleAverage = 4;
  byte ledMode = 2; // Red + IR
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop()
{
  bufferLength = 100; // simpan 100 sampel (4 detik @ 25Hz)

  // Baca 100 sampel awal
  for (byte i = 0; i < bufferLength; i++) {
    while (particleSensor.available() == false)
      particleSensor.check();

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();

    Serial.print("red=");
    Serial.print(redBuffer[i]);
    Serial.print(", ir=");
    Serial.println(irBuffer[i]);
  }

  // Hitung HR dan SpO2
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  while (1) {
    // Geser buffer
    for (byte i = 25; i < 100; i++) {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    // Tambah 25 data baru
    for (byte i = 75; i < 100; i++) {
      while (particleSensor.available() == false)
        particleSensor.check();

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();

      Serial.print("red=");
      Serial.print(redBuffer[i]);
      Serial.print(", ir=");
      Serial.print(irBuffer[i]);
      Serial.print(", HR=");
      Serial.print(heartRate);
      Serial.print(", HRvalid=");
      Serial.print(validHeartRate);
      Serial.print(", SpO2=");
      Serial.print(spo2);
      Serial.print(", SpO2valid=");
      Serial.println(validSPO2);
    }

    // Hitung ulang
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}
