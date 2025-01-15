#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>

#define MODE_BTN 22

TFT_eSPI display = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

const char* ssid = "UPC4589383";
const char* password = "vydkj3Veeurz";

const char* serverIP = "83.168.105.162";
const int serverPort = 27016;

WiFiUDP udp;
char responseBuffer[1024];
bool challengeRequired = false;

// byte queryPacket[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x54, 'S', 'o', 'u', 'r', 'c', 'e', ' ', 'E', 'n', 'g', 'i', 'n', 'e', ' ', 'Q', 'u', 'e', 'r', 'y', 0x00 };
// byte challengePacket[1024];
// bool querySent = false;

byte playerQueryPacket[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0xFF }; // Initial player query packet
byte playerChallengePacket[9];
unsigned long lastPlayerQueryTime = 0;
unsigned long lastPlayerResponseTime = 0;
bool playerQuerySent = false;

short displayState = 0;


void switchState();
void drawServerInfo();
void drawSquare();
void sendPlayerQuery();
void checkForPlayerResponse();
void handlePlayerChallenge(char*);
void parsePlayerInfo(char*, int);

void (*states[])(void) = { drawServerInfo, drawSquare };

void setup() {
    Serial.begin(9600);

    // Connect to Wi-Fi
    Serial.print("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Connected!");

    // Start UDP communication
    udp.begin(5000);
    Serial.println("UDP initialized.");

    // Initialize TFT display
    display.init();
    display.setRotation(1);
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK); // adding a black background color erases previous text automatically
    display.setTextFont(4);
    
    // Initialize pins
    pinMode(MODE_BTN, INPUT_PULLUP);

    switchState();
}


void loop() {
    static bool lastButtonState = HIGH;
    bool currentButtonState = digitalRead(MODE_BTN);

    if (lastButtonState == HIGH && currentButtonState == LOW) {
        switchState();
    }

    lastButtonState = currentButtonState;

    states[displayState]();
}


void switchState() {
    display.fillScreen(TFT_BLACK);

    switch(displayState) {
        case 0:
            lastPlayerResponseTime = 0;
            lastPlayerQueryTime = 0;
            challengeRequired = false;
            playerQuerySent = false;
            break;
        case 1:
            break;
    }

    displayState = (displayState + 1) % 2;
}


void drawServerInfo() {
    if (millis() > lastPlayerResponseTime + 60000) {
        if (!playerQuerySent) {
            sendPlayerQuery();
            playerQuerySent = true;
        }
        checkForPlayerResponse();
    }
}


void drawSquare() {
    display.fillRect(display.width() / 2 - 50, display.height() / 2 - 50, 100, 100, TFT_CYAN);
}


void sendPlayerQuery() {
  if (challengeRequired) {
    Serial.println("Sending player query with challenge...");
    udp.beginPacket(serverIP, serverPort);
    udp.write(playerChallengePacket, sizeof(playerChallengePacket));
    udp.endPacket();
  } else {
    Serial.println("Sending initial player query...");
    udp.beginPacket(serverIP, serverPort);
    udp.write(playerQueryPacket, sizeof(playerQueryPacket));
    udp.endPacket();
  }
  lastPlayerQueryTime = millis();
}

void checkForPlayerResponse() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        Serial.println("Player response received!");
        int len = udp.read(responseBuffer, sizeof(responseBuffer));
        if (len > 0)
            responseBuffer[len] = '\0';

        if (responseBuffer[4] == 0x41) { // S2C_CHALLENGE
            Serial.println("Player challenge received!");
            handlePlayerChallenge(responseBuffer);
        } else if (responseBuffer[4] == 0x44) { // A2S_PLAYER
            Serial.println("Player info received!");
            parsePlayerInfo(responseBuffer, len);
            playerQuerySent = false;
            challengeRequired = false;
            lastPlayerResponseTime = millis();
        }
    } else {
        if (millis() - lastPlayerQueryTime > 3000) { // Timeout
            Serial.println("No player response received. Retrying...");
            playerQuerySent = false;
            challengeRequired = false;
        }
    }
}

void handlePlayerChallenge(char* buffer) {
    memcpy(playerChallengePacket, playerQueryPacket, 5);
    memcpy(playerChallengePacket + 5, buffer + 5, 4);

    challengeRequired = true;
    playerQuerySent = false;
}

byte lastNumPlayers = 0;
void parsePlayerInfo(char* buffer, int length) {
    int index = 5; // Start after the header

    // Number of players
    byte numPlayers = buffer[index++];
    Serial.print("Number of Players: ");
    Serial.println(numPlayers);

    display.setCursor(0, 0);
    display.setTextWrap(false);
    // display.fillScreen(TFT_BLACK);

    if (numPlayers == 0) {
        display.print("No players online");
        display.println("                                                                                      ");
        return;
    }

    for (byte i = 0; i < numPlayers; i++) {
        // Index
        byte playerIndex = buffer[index++];
        Serial.print("Player Index: ");
        Serial.println(playerIndex);

        // Player Name
        Serial.print("Player Name: ");
        char* playerName = buffer + index;
        while (buffer[index] != '\0' && index < length) {
            Serial.print(buffer[index]);
            index++;
        }
        Serial.println();
        index++;

        if (*playerName == '\0') return;

        // Player Score
        long playerScore = *(long*)&buffer[index];
        Serial.print("Player Score: ");
        Serial.println(playerScore);
        index += 4;

        // Player Duration
        float playerDuration = *(float*)&buffer[index];
        Serial.print("Player Duration: ");
        Serial.println(playerDuration);
        index += 4;

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%d | %s", i + 1, playerName);
        display.print(buffer);
        display.println("                                                                                      ");
    }

    for (byte i = 0; i < lastNumPlayers - numPlayers; i++)
        display.println("                                                                                      ");

    lastNumPlayers = numPlayers;
}
