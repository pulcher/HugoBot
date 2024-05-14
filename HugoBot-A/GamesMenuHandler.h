#ifndef GAMES_MENU_H
#define GAMES_MENU_H

#include "hugobot-a-menu.h"
#include "Adafruit_ST7735.h"
#include "Adafruit_ST7735_Menu.h"
#include "Adafruit_seesaw.h"

class GamesMenuHandler {
  private:
    int EditGamesOption = 1;
    long Position = 0, oldPosition = 0;
    EditMenu* menu;
    Adafruit_ST7735* display;
    Adafruit_seesaw* ss;

  public:
    GamesMenuHandler(EditMenu& menu, Adafruit_ST7735& display, Adafruit_seesaw ss);
    void Handle();
};

#endif