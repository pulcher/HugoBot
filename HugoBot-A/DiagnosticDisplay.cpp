#include "Arduino.h"
#include "DiagnosticDisplay.h"

DiagnosticDisplay::DiagnosticDisplay() {
  _maxLines = 5;
}

void DiagnosticDisplay::log() {
  Serial.println("*************************************************************************");
}
