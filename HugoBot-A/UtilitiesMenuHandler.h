#ifndef UTILITIES_MENU_H
#define UTILITIES_MENU_H

#include "hugobot-a-menu.h"
#include "Adafruit_ST7735.h"
#include "Adafruit_ST7735_Menu.h"
#include "Adafruit_seesaw.h"

class UtilitiesMenuHandler {
  private:
    int EditInformationOptions = 1;
    long Position = 0, oldPosition = 0;
    EditMenu* menu;
    Adafruit_ST7735* display;
    Adafruit_seesaw* encoder;

  public:
    UtilitiesMenuHandler(EditMenu& menu, Adafruit_ST7735& display, Adafruit_seesaw& ss);
    void Handle();
};

#endif
