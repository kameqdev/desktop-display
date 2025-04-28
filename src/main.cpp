#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include "secrets.h"

#define ONBOARD_LED_PIN 2

// forward‑declare the task
void createWifiConnectionTask();
void wifiConnectionHandler(void *pvParameters);
void onWiFiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);

void createWifiConnectionTask() {
    digitalWrite(ONBOARD_LED_PIN, LOW);
    xTaskCreatePinnedToCore(wifiConnectionHandler, "wifiConnectionHandler", 10000, NULL, 1, NULL, 0);
}

void wifiConnectionHandler(void *pvParameters) {
    const char* ssid     = WIFI_SSID;
    const char* password = WIFI_PASSWORD;

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        digitalWrite(ONBOARD_LED_PIN, !digitalRead(ONBOARD_LED_PIN));
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    Serial.println();
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());

    digitalWrite(ONBOARD_LED_PIN, HIGH);
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Time synchronized");

    vTaskDelete(NULL);
}

void onWiFiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("WiFi disconnected, restarting connection task...");
    createWifiConnectionTask();
}

void setup() {
    Serial.begin(9600);
    pinMode(ONBOARD_LED_PIN, OUTPUT);
    digitalWrite(ONBOARD_LED_PIN, LOW);

    WiFi.onEvent(onWiFiDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    createWifiConnectionTask();
}

void loop() {}