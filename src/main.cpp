#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include "udp_utils.h"
#include "display_modes.h"
#include "config.h"

using namespace udpUtils;
using namespace displayModes;


byte displayState = 0;
void (*states[][2])() = { { digitalClockInit, digitalClock } };


void switchState() {
    displayState = (displayState + 1) % (sizeof(states) / sizeof(states[0]));

    display.fillScreen(TFT_BLACK);

    states[displayState][0]();
}


void setup() {
    Serial.begin(9600);

    // Connect to Wi-Fi
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println(" Connected!");

    // Start UDP communication
    udp.begin(5000); // Initialize UDP in setup
    Serial.println("UDP initialized.");

    // Initialize time
    configTime(GmtOffset_sec, DaylightOffset_sec, NtpServer);

    // Initialize TFT display
    display.init();
    display.setRotation(1);
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK); // adding a black background color erases previous text automatically
    display.setTextFont(4);
    
    // Initialize pins
    pinMode(MODE_BTN, INPUT_PULLUP);

    states[displayState][0]();
}

void loop() {
    static bool lastButtonState = HIGH;
    bool currentButtonState = digitalRead(MODE_BTN);

    if (lastButtonState == HIGH && currentButtonState == LOW)
        switchState();

    lastButtonState = currentButtonState;

    states[displayState][1]();

    
}