#include "udp_utils.h"
#include "server_config.h"
#include "display_modes.h"
#include "Free_Fonts.h"

using namespace displayModes;

namespace udpUtils {
    long long lastQueryTime = INT_MIN;

    WiFiUDP udp;

    bool querySent = false;

    byte A2S_INFO_QUERY[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x54, 0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E, 0x65, 0x20, 0x51, 0x75, 0x65, 0x72, 0x79, 0x00 };
    byte A2S_PLAYER_QUERY[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0xFF };
    byte A2S_INFO_QUERY_SIZE = sizeof(A2S_INFO_QUERY);
    byte A2S_PLAYER_QUERY_SIZE = sizeof(A2S_PLAYER_QUERY);
    
    const byte S2C_CHALLENGE_RES_HEADER = 0x41;
    const byte A2S_PLAYER_RES_HEADER  = 0x44;
    const byte A2S_INFO_RES_HEADER  = 0x49;


    void sendQuery(byte query[], size_t querySize) {
        Serial.println("Sending query...");
        udp.beginPacket(SERVER_IP, SERVER_PORT);
        udp.write(query, querySize);
        udp.endPacket();
        lastQueryTime = millis();
        querySent = true;
    }

    bool checkForInfoResponse() {
        int packetSize = udp.parsePacket();
        if (packetSize) {
            char responseBuffer[1024];
            int len = udp.read(responseBuffer, sizeof(responseBuffer));

            if (len > 0)
                responseBuffer[len] = '\0';

            if (responseBuffer[4] == S2C_CHALLENGE_RES_HEADER) {
                byte challengePacket[4];
                memcpy(challengePacket, responseBuffer + 5, 4);
                handleInfoChallenge(challengePacket);
                return false;
            }

            parseServerInfo(responseBuffer, len);
            querySent = false;
            return true;
        } else if (millis() - lastQueryTime > 5000) { // Timeout
            Serial.println("No query response received. Retrying...");
            querySent = false;
        }
        return false;
    }

    bool checkForPlayerResponse() {
        int packetSize = udp.parsePacket();
        if (packetSize) {
            char responseBuffer[1024];
            int len = udp.read(responseBuffer, sizeof(responseBuffer));

            if (len > 0)
                responseBuffer[len] = '\0';

            if (responseBuffer[4] == S2C_CHALLENGE_RES_HEADER) {
                byte challengePacket[4];
                memcpy(challengePacket, responseBuffer + 5, 4);
                handlePlayerChallenge(challengePacket);
                return false;
            }
            parsePlayerInfo(responseBuffer, len);
            querySent = false;
            return true;
        } else if (millis() - lastQueryTime > 3000) { // Timeout
            Serial.println("No query response received. Retrying...");
            querySent = false;
        }
        return false;
    }

    void handleInfoChallenge(byte challenge[4]) {
        byte query[sizeof(A2S_INFO_QUERY) + 4];
        memcpy(query, A2S_INFO_QUERY, sizeof(A2S_INFO_QUERY));
        memcpy(query + sizeof(A2S_INFO_QUERY), challenge, 4);
        sendQuery(query, sizeof(query));

        lastQueryTime = millis();
    }

    void handlePlayerChallenge(byte challenge[4]) {
        byte query[sizeof(A2S_PLAYER_QUERY)];
        memcpy(query, A2S_PLAYER_QUERY, sizeof(A2S_PLAYER_QUERY));
        memcpy(query + sizeof(A2S_PLAYER_QUERY) - 4, challenge, 4);
        sendQuery(query, sizeof(query));

        lastQueryTime = millis();
    }

    void parseServerInfo(char* buffer, int length) {
        int index = 6; // Start after the header

        display.setTextSize(1);

        // Server Name
        // display.setTextDatum(TC_DATUM);
        // display.setFreeFont(FMB12);
        char smallBuffer[128];
        // strncpy(smallBuffer, buffer + index, 128);
        // smallBuffer[127] = '\0';
        // display.drawString(smallBuffer, display.width() / 2, 8);

        index += strlen(buffer + index) + 1;

        // Map Name
        display.setTextDatum(TR_DATUM);
        display.setFreeFont(FM9);
        strncpy(smallBuffer, buffer + index, 128);
        smallBuffer[127] = '\0';
        display.drawString(smallBuffer, display.width() - 24, 92);

        index += strlen(buffer + index) + 1;

        // Folder
        index += strlen(buffer + index) + 1;

        // Game
        index += strlen(buffer + index) + 1;

        // ID
        index += 2;

        // Players
        display.setTextDatum(TL_DATUM);
        display.setFreeFont(FMB9);
        strncpy(smallBuffer, buffer + index, 128);
        int textWidth = display.drawString("Players: ", 24, 72);

        display.setFreeFont(FM9);
        byte players = buffer[index++];
        byte maxPlayers = buffer[index++];
        std::string playersStr = std::to_string(players) + "/" + std::to_string(maxPlayers);
        display.drawString(playersStr.c_str(), 24 + textWidth, 72);
    }

    void parsePlayerInfo(char* buffer, int length) {
        int index = 5; // Start after the header

        // Number of players
        byte numPlayers = buffer[index++];

        display.setTextWrap(false);
        display.setTextDatum(TL_DATUM);
        display.setTextFont(2);

        if (numPlayers == 0) return;

        for (byte i = 0; i < numPlayers; i++) {
            // Index
            index++;

            // Player Name
            char* playerName = buffer + index;
            index += strlen(buffer + index) + 1;

            // Player Score
            index += 4;

            // Player Duration
            index += 4;

            if (*playerName == '\0') return;

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%2d. %s", i + 1, playerName);
            display.drawString(buffer, 24, 100 + i * 20);
        }
    }
}