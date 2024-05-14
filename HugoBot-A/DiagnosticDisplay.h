#ifndef DiagnosticDisplay_h
#define DiagnosticDisplay_h

#include <usb_serial.h>

class DiagnosticDisplay {


  private:
    usb_serial_class* _serial;
    int _maxLines;
    int _currentLine;
    // make an array of lines the max that can fit the screen
    // then redraw the screen on each log call.
    // maybe a stack with a length will do or always add to the end and let the stuff roll off the top.

  public:
    DiagnosticDisplay(usb_serial_class& serial);
    void log();

};


#endif