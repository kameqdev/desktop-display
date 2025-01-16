#pragma once

#include <WiFiUdp.h>


namespace udpUtils {
    extern WiFiUDP udp;
    extern bool querySent;

    extern byte A2S_INFO_QUERY[];
    extern byte A2S_PLAYER_QUERY[];
    extern byte A2S_INFO_QUERY_SIZE;
    extern byte A2S_PLAYER_QUERY_SIZE;
    
    extern const byte S2C_CHALLENGE_RES_HEADER;
    extern const byte A2S_PLAYER_RES_HEADER;
    extern const byte A2S_INFO_RES_HEADER;

    void sendQuery(byte query[], size_t querySize);
    bool checkForInfoResponse();
    bool checkForPlayerResponse();
    void handleInfoChallenge(byte challenge[4]);
    void handlePlayerChallenge(byte challenge[4]);
    void parseServerInfo(char* buffer, int length);
    void parsePlayerInfo(char* buffer, int length);
}