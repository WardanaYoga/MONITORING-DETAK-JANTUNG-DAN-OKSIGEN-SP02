#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 sensor;

#define MAX_HISTORY 10  // Simpan 10 RR-Interval terakhir

long rrIntervals[MAX_HISTORY];  // Array untuk menyimpan RR-Interval
int rrIndex = 0;
unsigned long lastBeatTime = 0;

void setup() {
    Serial.begin(115200);
    if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
        Serial.println("Sensor MAX30102 tidak terdeteksi!");
        while (1);
    }

    // Konfigurasi Sensor MAX30102
    sensor.setup();
    sensor.setPulseAmplitudeRed(0x0A);
    sensor.setPulseAmplitudeIR(0x0A);

    Serial.println("HRV Monitoring Started...");
}

void loop() {
    long irValue = sensor.getIR();

    if (checkForBeat(irValue)) {  // Deteksi puncak detak jantung
        unsigned long now = millis();
        if (lastBeatTime > 0) {
            long rrInterval = now - lastBeatTime;
            Serial.print("RR-Interval: ");
            Serial.print(rrInterval);
            Serial.println(" ms");

            // Simpan RR-Interval ke dalam array
            rrIntervals[rrIndex] = rrInterval;
            rrIndex = (rrIndex + 1) % MAX_HISTORY;

            // Hitung SDNN dan RMSSD
            if (rrIndex == 0) {
                float sdnn = calculateSDNN(rrIntervals, MAX_HISTORY);
                float rmssd = calculateRMSSD(rrIntervals, MAX_HISTORY);
                Serial.print("SDNN: ");
                Serial.println(sdnn);
                Serial.print("RMSSD: ");
                Serial.println(rmssd);
            }
        }
        lastBeatTime = now;
    }

    delay(10);
}

// Fungsi menghitung SDNN (Standar Deviasi dari RR-Interval)
float calculateSDNN(long *rr, int size) {
    float mean = 0, sum = 0;
    for (int i = 0; i < size; i++) mean += rr[i];
    mean /= size;

    for (int i = 0; i < size; i++) sum += pow(rr[i] - mean, 2);
    return sqrt(sum / size);
}

// Fungsi menghitung RMSSD (Root Mean Square of Successive Differences)
float calculateRMSSD(long *rr, int size) {
    float sum = 0;
    for (int i = 1; i < size; i++) {
        sum += pow(rr[i] - rr[i - 1], 2);
    }
    return sqrt(sum / (size - 1));
}
