#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 sensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Ubah alamat I2C jika perlu

#define BUZZER 5
#define LED_PIN 2
#define MAX_HISTORY 10  // Menyimpan 10 RR-Interval terakhir

long rrIntervals[MAX_HISTORY];  
int rrIndex = 0;
unsigned long lastBeatTime = 0;
float spo2 = 0.0, bpm = 0.0;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    
    pinMode(BUZZER, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    // Inisialisasi LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");

    // Inisialisasi MAX30102
    if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
        Serial.println("Sensor MAX30102 tidak terdeteksi!");
        lcd.clear();
        lcd.print("Sensor Error!");
        while (1);
    }

    sensor.setup();
    sensor.setPulseAmplitudeRed(0x0A);
    sensor.setPulseAmplitudeIR(0x0A);

    lcd.clear();
    lcd.print("HRV & SpO2 Ready");
    delay(2000);
}

void loop() {
    long irValue = sensor.getIR();
    long redValue = sensor.getRed();

    if (checkForBeat(irValue)) {  // Deteksi detak jantung
        unsigned long now = millis();
        if (lastBeatTime > 0) {
            long rrInterval = now - lastBeatTime;
            bpm = 60000.0 / rrInterval;  // Hitung BPM
            rrIntervals[rrIndex] = rrInterval;
            rrIndex = (rrIndex + 1) % MAX_HISTORY;

            if (rrIndex == 0) {
                float sdnn = calculateSDNN(rrIntervals, MAX_HISTORY);
                float rmssd = calculateRMSSD(rrIntervals, MAX_HISTORY);
                Serial.print("SDNN: "); Serial.println(sdnn);
                Serial.print("RMSSD: "); Serial.println(rmssd);
            }
        }
        lastBeatTime = now;
    }

    // Hitung SpO2 (dummy perhitungan)
    spo2 = calculateSpO2(irValue, redValue);

    // Tampilan di LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BPM: "); lcd.print(bpm);
    lcd.setCursor(0, 1);
    lcd.print("SpO2: "); lcd.print(spo2); lcd.print("%");

    // Alarm jika BPM rendah atau tinggi
    if (bpm < 50 || bpm > 120) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }

    // Buzzer jika SpO2 di bawah 90%
    if (spo2 < 90) {
        digitalWrite(BUZZER, HIGH);
        delay(500);
        digitalWrite(BUZZER, LOW);
    }

    delay(1000);
}

// Fungsi perhitungan SpO2 (dummy, butuh algoritma lebih akurat)
float calculateSpO2(long ir, long red) {
    if (red == 0) return 0;  // Hindari pembagian dengan nol
    return 100.0 - ((float)red / ir * 25.0);  // Estimasi sederhana
}

// Fungsi menghitung SDNN (Standar Deviasi RR-Interval)
float calculateSDNN(long *rr, int size) {
    float mean = 0, sum = 0;
    for (int i = 0; i < size; i++) mean += rr[i];
    mean /= size;

    for (int i = 0; i < size; i++) sum += pow(rr[i] - mean, 2);
    return sqrt(sum / size);
}

// Fungsi menghitung RMSSD (Variasi RR-Interval)
float calculateRMSSD(long *rr, int size) {
    float sum = 0;
    for (int i = 1; i < size; i++) {
        sum += pow(rr[i] - rr[i - 1], 2);
    }
    return sqrt(sum / (size - 1));
}
