#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <LiquidCrystal_I2C.h>

MAX30105 sensor;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Sesuaikan alamat I2C jika berbeda

#define BUZZER 25
#define LED 26

void setup() {
    Serial.begin(115200);

    // Inisialisasi Sensor MAX30102
    if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
        Serial.println("Sensor MAX30102 tidak terdeteksi!");
        while (1);
    }

    // Konfigurasi sensor
    sensor.setup(); 
    sensor.setPulseAmplitudeRed(0x0A);
    sensor.setPulseAmplitudeIR(0x0A);

    // Inisialisasi LCD
    lcd.begin();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Detak Jantung");

    // Inisialisasi Buzzer & LED
    pinMode(BUZZER, OUTPUT);
    pinMode(LED, OUTPUT);
}

void loop() {
    long irValue = sensor.getIR();
    long redValue = sensor.getRed();
    
    if (irValue > 50000) { // Jika sensor membaca detak jantung
        int bpm = (irValue % 100) + 60; // Dummy Data BPM (Harus pakai algoritma HRV asli)
        int spo2 = (redValue % 10) + 95; // Dummy Data SpO2

        // Tampilkan di LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BPM: ");
        lcd.print(bpm);
        
        lcd.setCursor(0, 1);
        lcd.print("SpO2: ");
        lcd.print(spo2);
        lcd.print("%");

        // Cek jika BPM terlalu rendah atau tinggi
        if (bpm < 50 || bpm > 120) {
            digitalWrite(BUZZER, HIGH);
            digitalWrite(LED, HIGH);
        } else {
            digitalWrite(BUZZER, LOW);
            digitalWrite(LED, LOW);
        }
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Letakkan Jari!");
        digitalWrite(BUZZER, LOW);
        digitalWrite(LED, LOW);
    }

    delay(1000);
}
