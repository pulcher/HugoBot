#include "GamesMenuHandler.h"
#include "arduino.h"

GamesMenuHandler::GamesMenuHandler(EditMenu& menu, Adafruit_ST7735& display, Adafruit_seesaw& ss) 
  : menu(&menu), display(&display), encoder(&ss) {}

void GamesMenuHandler::Handle() {
  Serial.println("Games menu handling....");

  display->fillScreen(MENU_BACKGROUND);
  menu->draw();

  while (EditGamesOption != 0) {

    // standard encoder read
    Position = encoder->getEncoderPosition();
    delay(DEBOUNCE);
    
    if ((Position - oldPosition) > 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = encoder->getEncoderPosition();
        delay(DEBOUNCE);
      }

      menu->MoveUp();
    }

    if ((Position - oldPosition) < 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = encoder->getEncoderPosition();
        delay(DEBOUNCE);
      }

      menu->MoveDown();
    }

    if (!encoder->digitalRead(SS_SWITCH_SELECT)) {
      while (!encoder->digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      EditGamesOption = menu->selectRow();
    }
  }

}
