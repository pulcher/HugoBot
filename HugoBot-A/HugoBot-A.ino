/*

  example in how to implement the menu library for and ESP32 and Adafruit libraries. This is an example that
  uses both menu types
  1) a simple selection menu (ItemMenu) where the user can scroll up/down and select
  an item and some action be performed such as drawing a screen or activating a sub menu
  2) an menu with in-line where the user can scroll up/down the list and select an item
  however the difference is that move/up down on a selected item will scroll the values of that
  menu item and NOT scroll the list, selecing a selected item will restor control to moving up/down
  3) a EditMenu menu with in-line an no icons (down and dirty)

  highlights
  1. menu items exceeds screen size but library handles wrapping
  2. each item selection has a range, increment and decimal readout
  3. items can also be read from a char array


  ESP32     display
  3v3         VCC
  GND         GND
  5           TFT_CS
  25          TFT_RESET
  2           TFT_DC
  23          MOSI
  18          SCK
  3v3         LED
  19          MISO
  3v3         BL
  
  ESP32         Encoder
  32            select button 1
  GND           select button 2
  27            encoder direction 1
  33            encoder direction 2
  GND           encoder dir ground
  
*/

// required libraries
#include "Adafruit_GFX.h"
#include "Adafruit_ST7735.h"
#include "Adafruit_ST7735_Menu.h"
#include "Colors.h"
#include "Adafruit_seesaw.h"
#include <ST7735_t3.h>

// found in \Arduino\libraries\Adafruit-GFX-Library-master
#include "fonts\FreeSans9pt7b.h"
#include "fonts\FreeSans12pt7b.h"
#include "fonts\FreeSans18pt7b.h"
#include "fonts\FreeSansBold9pt7b.h"
#include "fonts\FreeSansBold12pt7b.h"
#include "fonts\FreeSansOblique9pt7b.h"

#define DATA_COLUMN 85

#define SEESAW_ADDR      0x49
#define SS_SWITCH_SELECT 1
#define SS_SWITCH_UP     2
#define SS_SWITCH_LEFT   3
#define SS_SWITCH_DOWN   4
#define SS_SWITCH_RIGHT  5

// esp32 pinouts
#define TFT_DC 8
#define TFT_CS 10
#define TFT_RST 9
#define TFT_MOSI 11  // Data out
#define TFT_MOSO 12
#define TFT_SCLK 13  // Clock out

// easy way to include fonts but change globally
#define FONT_SMALL FreeSans9pt7b      // font for menus
#define FONT_EDITTITLE FreeSans9pt7b  // font for menus
#define FONT_ITEM FreeSans9pt7b       // font for menus
#define FONT_TITLE FreeSans9pt7b      // font for all headings
#define FONT_SMALL_OB FreeSansOblique9pt7b

#define DEBOUNCE 100

// set default colors
#define TITLE_TEXT C_WHITE
#define TITLE_BACK C_DKBLUE

#define MENU_TEXT C_WHITE
#define MENU_BACKGROUND C_BLACK

// cursor dancing
#define MENU_SELECTTEXT C_WHITE
#define MENU_SELECT C_BLUE
#define MENU_SELECTBORDER C_DKBLUE

// if row selected
#define MENU_HIGHLIGHTTEXT C_WHITE
#define MENU_HIGHLIGHT C_RED
#define MENU_HIGHBORDER C_DKRED

#define MENU_DISABLE C_GREY

int MenuOption = 0;
int AllowColorMenu = 0;

// must have variables for each menu item
// best to have these global so you can use them in processing functions
int MenuOption1 = 0, MenuOption2 = 0, MenuOption3 = 0, MenuOption4 = 0, MenuOption5 = 0;
int InformationOptions1 = 0, InformationOptions2 = 0, InformationOptions3 = 0, InformationOptions4 = 0, InformationOptions5 = 0;
int GamesOptions1 = 0, GamesOptions2 = 0, GamesOptions3 = 0;
int CalibrationOptions1 = 0, CalibrationOptions2 = 0, CalibrationOptions3 = 0, CalibrationOptions4 = 0, CalibrationOptions5 = 0;
int UtilitiesOptions1 = 0, UtilitiesOptions2 = 0, UtilitiesOptions3 = 0;
int SettingsOptions1 = 0, SettingsOptions2 = 0, SettingsOptions3 = 0, SettingsOptions4 = 0;

int OptionOption1 = 0, OptionOption2 = 0, OptionOption3 = 0;
int WirelessOption1 = 0, WirelessOption2 = 0, WirelessOption3 = 0, WirelessOption4 = 0;
int ServoMenu1 = 0, ServoMenu2 = 0, ServoMenu3 = 0, ServoMenu4 = 0, ServoMenu5 = 0, ServoMenu6 = 0;

float Temp1Adj = 0.2, Temp2Adj = -.3, AlarmVal = 1;

// encoder stuff
long Position = 0, oldPosition = 0;

// create some selectable menu sub-items, these are lists inside a menu item
const char *ReadoutItems[] = { "Abs.", "Deg F", "Deg C" };
const char *RefreshItems[] = { "Off", "1 s", "2 s", "10 s",
                               "30 s", "1 m", "5 m" };
const char *PrecisionItems[] = { "10", "0", "0.0", "0.00", "0.000" };
const char *TuneItems[] = { "Slope", "Offset", "Linear", "2nd order ", "3rd order", "Log" };
const char *OffOnItems[] = { "Off", "On" };

const char *DataRateItems[] = { "300b", "1.2kb", "2.4kb", "4.8kb", "9.6kb", "19.2kb", "56kb" };

Adafruit_ST7735 Display = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
// Adafruit_ST7735 Display = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
// ST7735_t3 Display = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// fire up the seesaw interface.
Adafruit_seesaw ss(&Wire1);

// required, you must create either an Item menu (no inline editing) or an EditMenu (allows inline editing)
//ClassName YourMenuName(&DisplayObject, True=Touch input, False(Default)=mechanical input);
ItemMenu MainMenu(&Display);
EditMenu InformationMenu(&Display);
EditMenu GamesMenu(&Display);
EditMenu CalibrationMenu(&Display);
EditMenu UtilitiesMenu(&Display);
EditMenu SettingsMenu(&Display);

// Menu functions

void ProcessInformationMenu() {
  int EditInformationOptions = 1;

  Display.fillScreen(MENU_BACKGROUND);
  InformationMenu.draw();

  while (EditInformationOptions != 0) {

    // standard encoder read
    Position = ss.getEncoderPosition();
    delay(DEBOUNCE);
    
    if ((Position - oldPosition) > 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();
        delay(DEBOUNCE);
      }

      InformationMenu.MoveUp();
    }

    if ((Position - oldPosition) < 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();\
        delay(DEBOUNCE);
      }

      InformationMenu.MoveDown();
    }

    if (!ss.digitalRead(SS_SWITCH_SELECT)) {
      while (!ss.digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      EditInformationOptions = InformationMenu.selectRow();
    }
  }
}

void ProcessGamesMenu() {
  int EditGamesOption = 1;

  Display.fillScreen(MENU_BACKGROUND);
  GamesMenu.draw();

  while (EditGamesOption != 0) {

    // standard encoder read
    Position = ss.getEncoderPosition();
    delay(DEBOUNCE);
    
    if ((Position - oldPosition) > 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();
        delay(DEBOUNCE);
      }

      GamesMenu.MoveUp();
    }

    if ((Position - oldPosition) < 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();\
        delay(DEBOUNCE);
      }

      GamesMenu.MoveDown();
    }

    if (!ss.digitalRead(SS_SWITCH_SELECT)) {
      while (!ss.digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      EditGamesOption = GamesMenu.selectRow();
    }
  }
}

void ProcessCalibrationMenu() {
  int EditInformationOptions = 1;

  Display.fillScreen(MENU_BACKGROUND);
  CalibrationMenu.draw();

  while (EditInformationOptions != 0) {

    // standard encoder read
    Position = ss.getEncoderPosition();
    delay(DEBOUNCE);
    
    if ((Position - oldPosition) > 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();
        delay(DEBOUNCE);
      }

      CalibrationMenu.MoveUp();
    }

    if ((Position - oldPosition) < 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();\
        delay(DEBOUNCE);
      }

      CalibrationMenu.MoveDown();
    }

    if (!ss.digitalRead(SS_SWITCH_SELECT)) {
      while (!ss.digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      EditInformationOptions = CalibrationMenu.selectRow();
    }
  }
}

void ProcessUtilitiesMenu() {

  int EditInformationOptions = 1;

  Display.fillScreen(MENU_BACKGROUND);
  UtilitiesMenu.draw();

  while (EditInformationOptions != 0) {

    // standard encoder read
    Position = ss.getEncoderPosition();
    delay(DEBOUNCE);
    
    if ((Position - oldPosition) > 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();
        delay(DEBOUNCE);
      }

      UtilitiesMenu.MoveUp();
    }

    if ((Position - oldPosition) < 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();\
        delay(DEBOUNCE);
      }

      UtilitiesMenu.MoveDown();
    }

    if (!ss.digitalRead(SS_SWITCH_SELECT)) {
      while (!ss.digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      EditInformationOptions = UtilitiesMenu.selectRow();
    }
  }
}

void ProcessSettingsMenu() {
  int EditInformationOptions = 1;

  Display.fillScreen(MENU_BACKGROUND);
  SettingsMenu.draw();

  while (EditInformationOptions != 0) {

    // standard encoder read
    Position = ss.getEncoderPosition();
    delay(DEBOUNCE);
    
    if ((Position - oldPosition) > 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();
        delay(DEBOUNCE);
      }

      SettingsMenu.MoveUp();
    }

    if ((Position - oldPosition) < 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();\
        delay(DEBOUNCE);
      }

      SettingsMenu.MoveDown();
    }

    if (!ss.digitalRead(SS_SWITCH_SELECT)) {
      while (!ss.digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      EditInformationOptions = SettingsMenu.selectRow();
    }
  }
}
void ProcessMainMenu() {

  // set an inital flag that will be used to store what menu item the user exited on
  int MainMenuOption = 1;

  Serial.println("in processing menu... set background");
  Display.fillScreen(MENU_BACKGROUND);
  MainMenu.draw();

  // run the processing loop until user move selector to title bar (which becomes exit, i.e. 0 return val)
  // and selectes it
  // note menu code can return - 1 for errors so run unitl non zero
  while (MainMenuOption != 0) {

    //getEncoder(&MainMenu); 

    Position = ss.getEncoderPosition();
    delay(DEBOUNCE);
    // Serial.print("Encoder position: ");
    // Serial.println(Position);
    if ((Position - oldPosition) > 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();
        delay(DEBOUNCE);
      }
      
      MainMenu.MoveUp();
    }

    if ((Position - oldPosition) < 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();
        delay(DEBOUNCE);
      }

      MainMenu.MoveDown();
    }

    // but wait...the user pressed the button on the encoder
    if (!ss.digitalRead(SS_SWITCH_SELECT)) {

      // debounce the button press
      while (!ss.digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      // get the row the selector is on
      MainMenuOption = MainMenu.selectRow();

      // here is where you process accordingly
      // remember on pressing button on title bar 0 is returned and will exit the loop
      if (MainMenuOption == MenuOption1) {
        ProcessInformationMenu();

        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption2) {
        ProcessGamesMenu();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption3) {
        ProcessCalibrationMenu();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption4) {
        ProcessUtilitiesMenu();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption5) {
        ProcessSettingsMenu();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }      
    }
  }
}

void setup() {

  Serial.begin(115200);

  // fire up the display
  Display.initR(INITR_GREENTAB);
  Display.setRotation(1);

  // Initialize the SeeSaw
  if(!ss.begin(SEESAW_ADDR)) {
    Serial.println(F("Unable to find SeeSaw"));
    while(1);
  }

  Serial.println("Found Product 5740");

  ss.pinMode(SS_SWITCH_UP, INPUT_PULLUP);
  ss.pinMode(SS_SWITCH_DOWN, INPUT_PULLUP);
  ss.pinMode(SS_SWITCH_LEFT, INPUT_PULLUP);
  ss.pinMode(SS_SWITCH_RIGHT, INPUT_PULLUP);
  ss.pinMode(SS_SWITCH_SELECT, INPUT_PULLUP);

  // get starting position
  Position = ss.getEncoderPosition();

  Serial.println("Turning on interrupts");
  ss.enableEncoderInterrupt();
  Serial.println("Turning on GPIO interrupts");
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH_UP, 1);

  Serial.println("initializing main menu");
  MainMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, 20, 5, "Main", FONT_SMALL, FONT_TITLE);

  MenuOption1 = MainMenu.addNI("Information");
  MenuOption2 = MainMenu.addNI("Games");
  MenuOption3 = MainMenu.addNI("Calibration");
  MenuOption4 = MainMenu.addNI("Utilities");
  MenuOption5 = MainMenu.addNI("Settings");

  MainMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  MainMenu.setTitleBarSize(0, 0, 160, 20);
  MainMenu.setTitleTextMargins(10, 15);
  MainMenu.setMenuBarMargins(0, 160, 2, 1);
  MainMenu.setItemTextMargins(10, 15, 5);
  MainMenu.setItemColors(C_GREY, MENU_SELECTBORDER);

  int addNI(const char *ItemText, float Data, float LowLimit, float HighLimit,
            float Increment, byte DecimalPlaces = 0, const char **ItemMenuText = NULL);

  InformationMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                DATA_COLUMN, 20, 5, "Information", FONT_SMALL, FONT_SMALL);

  InformationOptions1 = InformationMenu.addNI("F-Lidar", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  InformationOptions2 = InformationMenu.addNI("Compass", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);  
  InformationOptions3 = InformationMenu.addNI("Location", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);              
  InformationOptions4 = InformationMenu.addNI("IMU", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);              
  InformationOptions5 = InformationMenu.addNI("Battery", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);              

  InformationMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  InformationMenu.setTitleBarSize(0, 0, 160, 20);
  InformationMenu.setTitleTextMargins(5, 15);
  InformationMenu.setMenuBarMargins(0, 160, 2, 1);
  InformationMenu.setItemTextMargins(5, 15, 0);
  InformationMenu.setItemColors(C_GREY, MENU_SELECTBORDER, MENU_HIGHBORDER);            

  GamesMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                DATA_COLUMN, 20, 5, "Games", FONT_SMALL, FONT_SMALL);

  GamesOptions1 = GamesMenu.addNI("Six", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  GamesOptions2 = GamesMenu.addNI("4", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);  
  GamesOptions3 = GamesMenu.addNI("Quick", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);              

  GamesMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  GamesMenu.setTitleBarSize(0, 0, 160, 20);
  GamesMenu.setTitleTextMargins(5, 15);
  GamesMenu.setMenuBarMargins(0, 160, 2, 1);
  GamesMenu.setItemTextMargins(5, 15, 0);
  GamesMenu.setItemColors(C_GREY, MENU_SELECTBORDER, MENU_HIGHBORDER);

  CalibrationMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                DATA_COLUMN, 20, 5, "Calibration", FONT_SMALL, FONT_SMALL);

  CalibrationOptions1 = CalibrationMenu.addNI("X factor", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  CalibrationOptions2 = CalibrationMenu.addNI("Y factor", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);  
  CalibrationOptions3 = CalibrationMenu.addNI("IMU", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);              
  CalibrationOptions4 = CalibrationMenu.addNI("Gripper", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);              
  CalibrationOptions5 = CalibrationMenu.addNI("Lidar", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);              

  CalibrationMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  CalibrationMenu.setTitleBarSize(0, 0, 160, 20);
  CalibrationMenu.setTitleTextMargins(5, 15);
  CalibrationMenu.setMenuBarMargins(0, 160, 2, 1);
  CalibrationMenu.setItemTextMargins(5, 15, 0);
  CalibrationMenu.setItemColors(C_GREY, MENU_SELECTBORDER, MENU_HIGHBORDER);

  UtilitiesMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                DATA_COLUMN, 20, 5, "Utilities", FONT_SMALL, FONT_SMALL);

  UtilitiesOptions1 = UtilitiesMenu.addNI("Load Conf", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  UtilitiesOptions2 = UtilitiesMenu.addNI("Save Conf", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);  
  UtilitiesOptions3 = UtilitiesMenu.addNI("Reset", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);                        

  UtilitiesMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  UtilitiesMenu.setTitleBarSize(0, 0, 160, 20);
  UtilitiesMenu.setTitleTextMargins(5, 15);
  UtilitiesMenu.setMenuBarMargins(0, 160, 2, 1);
  UtilitiesMenu.setItemTextMargins(5, 15, 0);
  UtilitiesMenu.setItemColors(C_GREY, MENU_SELECTBORDER, MENU_HIGHBORDER);

  SettingsMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                DATA_COLUMN, 20, 5, "Settings", FONT_SMALL, FONT_SMALL);

  SettingsOptions1 = SettingsMenu.addNI("X factor", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  SettingsOptions2 = SettingsMenu.addNI("Y factor", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);  
  SettingsOptions3 = SettingsMenu.addNI("IMU-N", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);              
  SettingsOptions4 = SettingsMenu.addNI("Gripper", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);                          

  SettingsMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  SettingsMenu.setTitleBarSize(0, 0, 160, 20);
  SettingsMenu.setTitleTextMargins(5, 15);
  SettingsMenu.setMenuBarMargins(0, 160, 2, 1);
  SettingsMenu.setItemTextMargins(5, 15, 0);
  SettingsMenu.setItemColors(C_GREY, MENU_SELECTBORDER, MENU_HIGHBORDER);

  Serial.println("ProcessingMainMenu");
  ProcessMainMenu();

  // menu code done, now proceed to your code
  Display.fillScreen(MENU_BACKGROUND);
}

void loop() {
}
