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
#include "Adafruit_BNO08x.h"
#include "Colors.h"
#include "Adafruit_seesaw.h"
#include <SparkFun_VL53L5CX_Library.h>
#include "Optical_Flow_Sensor.h"
#include <SD.h>
#include <SPI.h>

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
#define TFT_DC1 8
#define TFT_CS 10
#define TFT_CS1 0
#define TFT_RST 9
#define TFT_RST1 9
#define TFT_MOSI 11  // Data out
#define TFT_MOSO 12
#define TFT_SCLK 13  // Clock out

// BNO08x defines
#define BNO08X_RESET -1

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

// Lidar Sensor stuff
int imageResolution = 0; //Used to pretty print output
int imageWidth = 0; //Used to pretty print output

// create some selectable menu sub-items, these are lists inside a menu item
const char *ReadoutItems[] = { "Abs.", "Deg F", "Deg C" };
const char *RefreshItems[] = { "Off", "1 s", "2 s", "10 s",
                               "30 s", "1 m", "5 m" };
const char *PrecisionItems[] = { "10", "0", "0.0", "0.00", "0.000" };
const char *TuneItems[] = { "Slope", "Offset", "Linear", "2nd order ", "3rd order", "Log" };
const char *OffOnItems[] = { "Off", "On" };

const char *DataRateItems[] = { "300b", "1.2kb", "2.4kb", "4.8kb", "9.6kb", "19.2kb", "56kb" };

// Adafruit_ST7735 Display = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_ST7735 Display = Adafruit_ST7735(&SPI1, TFT_CS1, TFT_DC1, TFT_RST1);

SparkFun_VL53L5CX centerLidar;
VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM

Adafruit_BNO08x bno08x(BNO08X_RESET);
Optical_Flow_Sensor flow(10, PAA5100);

// SD card stuff
const int chipSelect = BUILTIN_SDCARD;

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

void Setup8x8Sensor() {
    Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
      // No SD card, so don't do anything more - stay stuck here
    }
  }
  Serial.println("card initialized.");

  Serial.println("SparkFun VL53L5CX Driver");

  Serial.println("Initializing sensor board. This can take up to 10s. Please wait.");
  if (centerLidar.begin() == false)
  {
    Serial.println(F("centerLidar Sensor not found - check your wiring. Freezing"));
    while (1) ;
  }

  centerLidar.setResolution(8 * 8);

  imageResolution = centerLidar.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution); //Calculate printing width

  bool response = centerLidar.setRangingFrequency(10);
  if (response == true)
  {
    int frequency = centerLidar.getRangingFrequency();
    if (frequency > 0)
    {
      Serial.print("Ranging frequency set to ");
      Serial.print(frequency);
      Serial.println(" Hz.");
    }
    else
      Serial.println(F("Error recovering ranging frequency."));
  }
  else
  {
    Serial.println(F("Cannot set ranging frequency requested. Freezing..."));
    while (1) ;
  }

  centerLidar.startRanging();
}

void SetupImuSensor() {
 if (!bno08x.begin_I2C(0x4a, &Wire1, 0)) {
    // if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte
    // UART buffer! if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");

  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }

  setReports();

  Serial.println("Reading events");
  delay(100);
}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
  // if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
  //   Serial.println("Could not enable accelerometer");
  // }
  // if (!bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED)) {
  //   Serial.println("Could not enable gyroscope");
  // }
  if (!bno08x.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED)) {
    Serial.println("Could not enable magnetic field calibrated");
  }
}

void SetupOpticalFlowSensor()
{
    if (!flow.begin()) {
    Serial.println("Initialization of the flow sensor failed");
    while(1) { }
  }
}

void printActivity(uint8_t activity_id) {
  switch (activity_id) {
  case PAC_UNKNOWN:
    Serial.print("Unknown");
    break;
  case PAC_IN_VEHICLE:
    Serial.print("In Vehicle");
    break;
  case PAC_ON_BICYCLE:
    Serial.print("On Bicycle");
    break;
  case PAC_ON_FOOT:
    Serial.print("On Foot");
    break;
  case PAC_STILL:
    Serial.print("Still");
    break;
  case PAC_TILTING:
    Serial.print("Tilting");
    break;
  case PAC_WALKING:
    Serial.print("Walking");
    break;
  case PAC_RUNNING:
    Serial.print("Running");
    break;
  case PAC_ON_STAIRS:
    Serial.print("On Stairs");
    break;
  default:
    Serial.print("NOT LISTED");
  }
  Serial.print(" (");
  Serial.print(activity_id);
  Serial.print(")");
}


void setup() {

  Serial.begin(115200);

  Wire.begin(); //This resets I2C bus to 100kHz
  Wire.setClock(1000000); //Sensor has max I2C freq of 1MHz

  // fire up the VL53L5CX 8x8 Time of Flight (ToF) Array Sensor
  Setup8x8Sensor();

  // fire up the BNO085 IMU
  SetupImuSensor();

  // fire up the PAA5100JE-O
  SetupOpticalFlowSensor();

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

void DisplayLidar() {
  //Poll sensor for new data
  if (centerLidar.isDataReady() == true)
  {
    // File dataFile = SD.open("lidar.txt", FILE_WRITE);
    
    // dataFile.println("**********************************************************************");
    Serial.println("**********************************************************************");

    if (centerLidar.getRangingData(&measurementData)) //Read distance data into array
    {
      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for (int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth)
      {
        for (int x = imageWidth - 1 ; x >= 0 ; x--)
        {
          Serial.print("\t");
          // dataFile.print("\t");
          Serial.print(measurementData.distance_mm[x + y]);
          // dataFile.print(measurementData.distance_mm[x + y]);
        }
        Serial.println();
        // dataFile.println();
      }
      Serial.println();
      // dataFile.println();
    }

    // dataFile.close();
  }
}

void DisplayImu () {
  sh2_SensorValue_t sensorValue;
  
    if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  switch (sensorValue.sensorId) {

  case SH2_ACCELEROMETER:
    Serial.print("Accelerometer - x: ");
    Serial.print(sensorValue.un.accelerometer.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.accelerometer.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.accelerometer.z);
    break;
  case SH2_GYROSCOPE_CALIBRATED:
    Serial.print("Gyro - x: ");
    Serial.print(sensorValue.un.gyroscope.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.gyroscope.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.gyroscope.z);
    break;
  case SH2_MAGNETIC_FIELD_CALIBRATED:
    Serial.print("Magnetic Field - x: ");
    Serial.print(sensorValue.un.magneticField.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.magneticField.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.magneticField.z);
    break;
  case SH2_LINEAR_ACCELERATION:
    Serial.print("Linear Acceration - x: ");
    Serial.print(sensorValue.un.linearAcceleration.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.linearAcceleration.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.linearAcceleration.z);
    break;
  case SH2_GRAVITY:
    Serial.print("Gravity - x: ");
    Serial.print(sensorValue.un.gravity.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.gravity.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.gravity.z);
    break;
  case SH2_ROTATION_VECTOR:
    Serial.print("Rotation Vector - r: ");
    Serial.print(sensorValue.un.rotationVector.real);
    Serial.print(" i: ");
    Serial.print(sensorValue.un.rotationVector.i);
    Serial.print(" j: ");
    Serial.print(sensorValue.un.rotationVector.j);
    Serial.print(" k: ");
    Serial.println(sensorValue.un.rotationVector.k);
    break;
  case SH2_GEOMAGNETIC_ROTATION_VECTOR:
    Serial.print("Geo-Magnetic Rotation Vector - r: ");
    Serial.print(sensorValue.un.geoMagRotationVector.real);
    Serial.print(" i: ");
    Serial.print(sensorValue.un.geoMagRotationVector.i);
    Serial.print(" j: ");
    Serial.print(sensorValue.un.geoMagRotationVector.j);
    Serial.print(" k: ");
    Serial.println(sensorValue.un.geoMagRotationVector.k);
    break;

  case SH2_GAME_ROTATION_VECTOR:
    Serial.print("Game Rotation Vector - r: ");
    Serial.print(sensorValue.un.gameRotationVector.real);
    Serial.print(" i: ");
    Serial.print(sensorValue.un.gameRotationVector.i);
    Serial.print(" j: ");
    Serial.print(sensorValue.un.gameRotationVector.j);
    Serial.print(" k: ");
    Serial.println(sensorValue.un.gameRotationVector.k);
    break;

  case SH2_STEP_COUNTER:
    Serial.print("Step Counter - steps: ");
    Serial.print(sensorValue.un.stepCounter.steps);
    Serial.print(" latency: ");
    Serial.println(sensorValue.un.stepCounter.latency);
    break;

  case SH2_STABILITY_CLASSIFIER: {
    Serial.print("Stability Classification: ");
    sh2_StabilityClassifier_t stability = sensorValue.un.stabilityClassifier;
    switch (stability.classification) {
    case STABILITY_CLASSIFIER_UNKNOWN:
      Serial.println("Unknown");
      break;
    case STABILITY_CLASSIFIER_ON_TABLE:
      Serial.println("On Table");
      break;
    case STABILITY_CLASSIFIER_STATIONARY:
      Serial.println("Stationary");
      break;
    case STABILITY_CLASSIFIER_STABLE:
      Serial.println("Stable");
      break;
    case STABILITY_CLASSIFIER_MOTION:
      Serial.println("In Motion");
      break;
    }
    break;
  }

  case SH2_RAW_ACCELEROMETER:
    Serial.print("Raw Accelerometer - x: ");
    Serial.print(sensorValue.un.rawAccelerometer.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.rawAccelerometer.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.rawAccelerometer.z);
    break;
  case SH2_RAW_GYROSCOPE:
    Serial.print("Raw Gyro - x: ");
    Serial.print(sensorValue.un.rawGyroscope.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.rawGyroscope.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.rawGyroscope.z);
    break;
  case SH2_RAW_MAGNETOMETER:
    Serial.print("Raw Magnetic Field - x: ");
    Serial.print(sensorValue.un.rawMagnetometer.x);
    Serial.print(" y: ");
    Serial.print(sensorValue.un.rawMagnetometer.y);
    Serial.print(" z: ");
    Serial.println(sensorValue.un.rawMagnetometer.z);
    break;

  case SH2_SHAKE_DETECTOR: {
    Serial.print("Shake Detector - shake detected on axis: ");
    sh2_ShakeDetector_t detection = sensorValue.un.shakeDetector;
    switch (detection.shake) {
    case SHAKE_X:
      Serial.println("X");
      break;
    case SHAKE_Y:
      Serial.println("Y");
      break;
    case SHAKE_Z:
      Serial.println("Z");
      break;
    default:
      Serial.println("None");
      break;
    }
  }

  case SH2_PERSONAL_ACTIVITY_CLASSIFIER: {

    sh2_PersonalActivityClassifier_t activity =
        sensorValue.un.personalActivityClassifier;
    Serial.print("Activity classification - Most likely: ");
    printActivity(activity.mostLikelyState);
    Serial.println("");

    Serial.println("Confidences:");
    // if PAC_OPTION_COUNT is ever > 10, we'll need to
    // care about page
    for (uint8_t i = 0; i < PAC_OPTION_COUNT; i++) {
      Serial.print("\t");
      printActivity(i);
      Serial.print(": ");
      Serial.println(activity.confidence[i]);
    }
  }
  }
}

void DisplayOpticalFlow () {
  int16_t deltaX, deltaY;

  // Get motion count since last call
  flow.readMotionCount(&deltaX, &deltaY);

  Serial.print("X: ");
  Serial.print(deltaX);
  Serial.print(", Y: ");
  Serial.print(deltaY);
  Serial.print("\n");
}

void loop() {
  
  DisplayLidar();

  // DisplayImu();

  //DisplayOpticalFlow();

  delay(5);
}
