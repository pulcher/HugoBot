#include "Arduino.h"
#include "DiagnosticDisplay.h"

DiagnosticDisplay::DiagnosticDisplay(usb_serial_class& serial, Adafruit_ST7735& display) 
  : _serial(&serial), _display(&display) {}

void DiagnosticDisplay::log(String message) {
  _serial->print(message);
}

void DiagnosticDisplay::logln(String message) {
  _serial->println(message);
}
