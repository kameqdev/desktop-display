#include "display_modes.h"
#include "time.h"
#include "Free_Fonts.h"
#include "server_config.h"
#include "udp_utils.h"

using namespace udpUtils;

namespace displayModes {
    TFT_eSPI display = TFT_eSPI(); // Define the display variable

    long long lastDrawTime;

    void digitalClockInit() {
        lastDrawTime = -1000;
        display.setTextDatum(TC_DATUM);
        display.setTextPadding(display.width() / 2);
        display.setTextFont(2);
    }

    void digitalClock() {
        if (millis() < lastDrawTime + 1000) return;
        lastDrawTime = millis();

        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            // display.println("Failed to obtain time");
            return;
        }

        // Adjust for GMT offset
        timeinfo.tm_hour += 1;

        // Adjust for daylight saving time
        if (timeinfo.tm_isdst > 0) {
            timeinfo.tm_hour -= 1;
        }

        char timeStringBuffer[9];
        strftime(timeStringBuffer, sizeof(timeStringBuffer), "%H:%M:%S", &timeinfo);

        char dateStringBuffer[21];
        strftime(dateStringBuffer, sizeof(dateStringBuffer), "%d %B %Y", &timeinfo);

        char dayStringBuffer[13];
        strftime(dayStringBuffer, sizeof(dayStringBuffer), "%A", &timeinfo);
        
        display.setTextFont(7);
        display.setTextSize(2);
        display.drawString(timeStringBuffer, display.width() / 2, display.height() / 2 - 64);

        display.setTextFont(4);
        display.setTextSize(1);
        display.drawString(dateStringBuffer, display.width() / 2, display.height() / 2 + 48);
        display.drawString(dayStringBuffer, display.width() / 2, display.height() / 2 + 76);
    }

    void gmodServerDataInit() {
        lastDrawTime = -60000;
        querySent = false;
        display.setTextPadding(0);

        display.setTextDatum(TC_DATUM);
        display.setFreeFont(FMB18);
        display.drawString("FableZone DarkRP", display.width() / 2, 8);

        display.drawLine(16, 50, display.width() - 16, 50, TFT_LIGHTGREY);

        display.setTextDatum(TR_DATUM);
        display.setFreeFont(FMB9);
        display.drawString("Map", display.width() - 24, 72);
        display.drawString("Server IP", display.width() - 24, 132);
        display.drawString("Server Port", display.width() - 24, 192);

        display.setFreeFont(FM9);
        display.drawString(SERVER_IP, display.width() - 24, 152);
        display.drawString(std::to_string(SERVER_PORT).c_str(), display.width() - 24, 212);
        

        bool info = false;
        while(!info) {
            if (!querySent)
                sendQuery(A2S_INFO_QUERY, A2S_INFO_QUERY_SIZE);
            else
                info = checkForInfoResponse();
        };
    }

    void gmodServerData() {
        if (millis() < lastDrawTime + 60000) return;

        if (!querySent) {
            sendQuery(A2S_PLAYER_QUERY, A2S_PLAYER_QUERY_SIZE);
        } else {
            bool state = checkForPlayerResponse();
            if (state) lastDrawTime = millis();
        }
    }

}