#include "UtilitiesMenuHandler.h"
#include "arduino.h"

UtilitiesMenuHandler::UtilitiesMenuHandler(EditMenu& menu, Adafruit_ST7735& display, Adafruit_seesaw& ss)
  : menu(&menu), display(&display), encoder(&ss) {}

void UtilitiesMenuHandler::Handle() {
  Serial.println("Utilities menu handling....");

  int EditInformationOptions = 1;

  display->fillScreen(MENU_BACKGROUND);
  menu->draw();

  while (EditInformationOptions != 0) {

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
        Position = encoder->getEncoderPosition();\
        delay(DEBOUNCE);
      }

      menu->MoveDown();
    }

    if (!encoder->digitalRead(SS_SWITCH_SELECT)) {
      while (!encoder->digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      EditInformationOptions = menu->selectRow();
    }
  }
}
