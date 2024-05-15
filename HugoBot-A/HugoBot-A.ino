/*
  HugoBot
*/

// required libraries
#include "Adafruit_GFX.h"
#include "Adafruit_ST7735.h"
#include "Adafruit_ST7735_Menu.h"
#include "Adafruit_BNO08x.h"
#include "Adafruit_seesaw.h"
#include <SparkFun_VL53L5CX_Library.h>
#include "Optical_Flow_Sensor.h"
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "directions.h"
#include "DiagnosticDisplay.h"
#include "GamesMenuHandler.h"
#include "InformationMenuHandler.h"
#include "CalibrationMenuHandler.h"
#include "UtilitiesMenuHandler.h"
#include "SettingsMenuHandler.h"

#include "hugobot-a-menu.h"

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

DiagnosticDisplay diag = DiagnosticDisplay(Serial, Display);

SparkFun_VL53L5CX centerLidar;
VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM

Adafruit_BNO08x bno08x(BNO08X_RESET);
Optical_Flow_Sensor flow(10, PAA5100);

// SD card stuff
const int chipSelect = BUILTIN_SDCARD;

// fire up the seesaw interface.
Adafruit_seesaw ss(&Wire);

// required, you must create either an Item menu (no inline editing) or an EditMenu (allows inline editing)
//ClassName YourMenuName(&DisplayObject, True=Touch input, False(Default)=mechanical input);
ItemMenu MainMenu(&Display);
EditMenu InformationMenu(&Display);
EditMenu GamesMenu(&Display);
EditMenu CalibrationMenu(&Display);
EditMenu UtilitiesMenu(&Display);
EditMenu SettingsMenu(&Display);

GamesMenuHandler gamesMenuHandler = GamesMenuHandler(GamesMenu, Display, ss);
InformationMenuHandler informationMenuHandler = InformationMenuHandler(InformationMenu, Display, ss);
CalibrationMenuHandler calibrationMenuHandler = CalibrationMenuHandler(CalibrationMenu, Display, ss);
UtilitiesMenuHandler utilitiesMenuHandler = UtilitiesMenuHandler(UtilitiesMenu, Display, ss);
SettingsMenuHandler settingsMenuHandler = SettingsMenuHandler(SettingsMenu, Display, ss);

// Menu functions

void ProcessMainMenu() {

  // set an inital flag that will be used to store what menu item the user exited on
  int MainMenuOption = 1;

  diag.logln("in processing menu... set background");
  Display.fillScreen(MENU_BACKGROUND);
  MainMenu.draw();

  // run the processing loop until user move selector to title bar (which becomes exit, i.e. 0 return val)
  // and selectes it
  // note menu code can return - 1 for errors so run unitl non zero
  while (MainMenuOption != 0) {

    //getEncoder(&MainMenu); 

    Position = ss.getEncoderPosition();
    delay(DEBOUNCE);
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
        informationMenuHandler.Handle();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption2) {
        gamesMenuHandler.Handle();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption3) {
        calibrationMenuHandler.Handle();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption4) {
        utilitiesMenuHandler.Handle();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption5) {
        settingsMenuHandler.Handle();
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
    // while (1) {
    //   // No SD card, so don't do anything more - stay stuck here
    // }
  }
  Serial.println("card initialized.");

  Serial.println("SparkFun VL53L5CX Driver");

  Serial.println("Initializing sensor board. This can take up to 10s. Please wait.");
  if (centerLidar.begin() == false)
  {
    Serial.println(F("centerLidar Sensor not found - check your wiring. Freezing"));
   // while (1) ;
  } else {
                
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
//    while (1) ;
  }

  centerLidar.startRanging();
  }
}

void SetupImuSensor() {
 if (!bno08x.begin_I2C(0x4a, &Wire1, 0)) {
    Serial.println("Failed to find BNO08x chip");
    // while (1) {
    //   delay(10);
    // }
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
    // while(1) { }
  }
}

void SetupSerial1ToMegaPi() {
  Serial8.begin(9600, SERIAL_8N1);

  // Add values in the document
  // doc["sensor"] = "gps";
  // doc["time"] = 1351824120;

  // Add an array.
  //JsonArray data = doc.createNestedArray("data");
  //data.add(48.756080);
  //data.add(2.302038);

  Serial.println("Setup8ToMegaPi executed.");
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

void SetupDiagnosticDisplay() {
  Display.initR(INITR_BLACKTAB);
  Display.fillScreen(ST77XX_BLACK);

  diag.logln("Setting up diagnostic display...");
}

void setup() {

  Serial.begin(115200);
  Serial.println("Starting setup....");

  SetupDiagnosticDisplay();

  Wire.begin(); //This resets I2C bus to 100kHz
  Wire.setClock(1000000); //Sensor has max I2C freq of 1MHz

  // // fire up the VL53L5CX 8x8 Time of Flight (ToF) Array Sensor
  // // Setup8x8Sensor();

  // // fire up the BNO085 IMU
  // // SetupImuSensor();

  // // fire up the PAA5100JE-O
  // //SetupOpticalFlowSensor();

  // // fire up Serial1 connectivity to MegaPi
  // //SetupSerial1ToMegaPi();

  // fire up the display
  Display.initR(INITR_GREENTAB);
  Display.setRotation(1);

  // Initialize the SeeSaw
  if(!ss.begin(SEESAW_ADDR)) {
    diag.logln(F("Unable to find SeeSaw"));
    while(1);
  }

  diag.logln("Found Product 5740");

  ss.pinMode(SS_SWITCH_UP, INPUT_PULLUP);
  ss.pinMode(SS_SWITCH_DOWN, INPUT_PULLUP);
  ss.pinMode(SS_SWITCH_LEFT, INPUT_PULLUP);
  ss.pinMode(SS_SWITCH_RIGHT, INPUT_PULLUP);
  ss.pinMode(SS_SWITCH_SELECT, INPUT_PULLUP);

  // get starting position
  Position = ss.getEncoderPosition();

  diag.logln("Turning on interrupts");
  ss.enableEncoderInterrupt();
  diag.logln("Turning on GPIO interrupts");
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH_UP, 1);

  diag.logln("initializing main menu");
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

  diag.logln("ProcessingMainMenu");
  ProcessMainMenu();

  // // menu code done, now proceed to your code
  // Display.fillScreen(MENU_BACKGROUND);
}

void DoSerial8Stuff() {

// sample
// {
//   "operation": "move",
//   "data": {
//     "direction": "NW",
//     "speed": "50",
//     "timeMS": 500
//   }
// }

  JsonDocument doc;

  doc.clear();
  // go forward at full speed 
  doc["operation"] = "move";

  JsonObject dataDoc = doc["data"].to<JsonObject>();
  dataDoc["direction"] = DIRECTION_F;
  dataDoc["speed"] = 100;
  dataDoc["timeMS"] = 1000;
  
  serializeJson(doc, Serial8);
  Serial8.println();

  serializeJson(doc, Serial);
  Serial.println();
  // wait for 2 seconds
  delay(2000);

  // stop
  doc.clear();
  // go forward at full speed 
  doc["operation"] = "move";

  dataDoc = doc["data"].to<JsonObject>();
  dataDoc["direction"] = DIRECTION_NONE;
  dataDoc["speed"] = 0;
  dataDoc["timeMS"] = 1;

  serializeJson(doc, Serial8);
  Serial8.println();

  // go reverse at full speed
  doc.clear();
  doc["operation"] = "move";

  dataDoc = doc["data"].to<JsonObject>();
  dataDoc["direction"] = DIRECTION_R;
  dataDoc["speed"] = 100;
  dataDoc["timeMS"] = 1000;
  
  serializeJson(doc, Serial8);
  Serial8.println();

  serializeJson(doc, Serial);
  Serial.println();
  // wait for 2 seconds
  delay(2000);

  // stop


  // serializeJson(doc, Serial8);
  // Serial8.println();

  // serializeJson(doc, Serial);
  // Serial.println();
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
  
  // DisplayLidar();

  // DisplayImu();

  // DisplayOpticalFlow();

  //DoSerial8Stuff();

  diag.logln("loop....");
  delay(500);
}
