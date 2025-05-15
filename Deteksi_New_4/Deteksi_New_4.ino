#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <LiquidCrystal_I2C.h>

MAX30105 particleSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define MAX_BRIGHTNESS 255
#define tombolBaca 4 // Pin tombol
#define ledIndikator 15

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
uint16_t irBuffer[100];
uint16_t redBuffer[100];
#else
uint32_t irBuffer[100];
uint32_t redBuffer[100];
#endif

int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

byte pulseLED = 13;
byte readLED = 14;

void setup()
{
  Serial.begin(115200);

  pinMode(pulseLED, OUTPUT);
  pinMode(readLED, OUTPUT);
  pinMode(ledIndikator, OUTPUT);
  pinMode(tombolBaca, INPUT_PULLUP); // Tombol aktif LOW
  Wire.begin(21, 22); // ESP32 default: SDA=21, SCL=22

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println(F("Sensor tidak ditemukan. Cek kabel dan power."));
    while (1);
  }

  byte ledBrightness = 60;
  byte sampleAverage = 4;
  byte ledMode = 2;
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  Serial.println("Silakan tekan tombol untuk mulai pembacaan HR & SpO2...");
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Siap digunakan");
  lcd.setCursor(0, 1);
  lcd.print("Tekan tombol...");

}

void loop()
{
  if (digitalRead(tombolBaca) == LOW)
  {
    Serial.println("Pembacaan dimulai...");
    
    lcd.clear();
    lcd.print("Membaca data...");
    digitalWrite(ledIndikator, HIGH);

    delay(500); // debounce
    ambilDataHRSpO2();

    Serial.println("Selesai. Silakan ganti orang dan tekan tombol lagi.\n");
    delay(2000); // jeda sebelum kembali ke loop
  }
}

void ambilDataHRSpO2()
{ 
  bufferLength = 100;

  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (!particleSensor.available())
      particleSensor.check();

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer,
                                         &spo2, &validSPO2, &heartRate, &validHeartRate);

  // Tampilkan hasil
  Serial.print("HR     : ");
  Serial.print(heartRate);
  Serial.print(" bpm (Valid: ");
  Serial.print(validHeartRate);
  Serial.println(")");

  Serial.print("SpO2   : ");
  Serial.print(spo2);
  Serial.print(" % (Valid: ");
  Serial.print(validSPO2);
  Serial.println(")");

  lcd.clear();
  if(validHeartRate && validSPO2)
  {
    lcd.setCursor(0, 0);
    lcd.print("HR:");
    lcd.print(heartRate);
    lcd.print("bpm");

    lcd.setCursor(0, 1);
    lcd.print("Sp02:");
    lcd.print(spo2);
    lcd.print("%");
  }
  else
  {
    lcd.print("Data tidak valid");
  }
}
