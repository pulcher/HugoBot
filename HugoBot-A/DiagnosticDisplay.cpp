#include "Arduino.h"
#include "DiagnosticDisplay.h"

DiagnosticDisplay::DiagnosticDisplay(usb_serial_class& serial) 
  : _serial(&serial) {}

void DiagnosticDisplay::log() {
  _serial->println("*************************************************************************");
}
