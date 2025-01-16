#pragma once

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

namespace displayModes {
    extern TFT_eSPI display;
    
    void digitalClockInit();
    void digitalClock();


    void gmodServerDataInit();
    void gmodServerData();
}