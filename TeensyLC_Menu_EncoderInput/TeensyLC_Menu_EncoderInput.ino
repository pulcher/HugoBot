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
#include <Adafruit_seesaw.h>
#include <Encoder.h>
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
#define TFT_MOSI 12  // Data out
#define TFT_SCLK 13  // Clock out

#define EN1_PIN 4
#define EN2_PIN 5
#define SE_PIN 6

// easy way to include fonts but change globally
// #define FONT_MICRO 1  // font for menus
// #define FONT_SMALL 2  // font for menus
// #define FONT_TITLE 3  // font for menus
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
int MenuOption1 = 0, MenuOption2 = 0, MenuOption3 = 0;

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

// you know the drill
Adafruit_ST7735 Display = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// fire up the seesaw interface.
Adafruit_seesaw ss(&Wire1);

// required, you must create either an Item menu (no inline editing) or an EditMenu (allows inline editing)
//ClassName YourMenuName(&DisplayObject, True=Touch input, False(Default)=mechanical input);
ItemMenu MainMenu(&Display);

// since we're showing both menu types, create an object for each where the item menu is the main and calls edit menus
// you can have an item menu call other item menus an edit menu can call an edit menu but in a round about way--not recommended
//ClassName YourMenuName(&DisplayObject, True=Touch input, False(Default)=mechanical input);
EditMenu OptionMenu(&Display);    // default is false, need not specify
EditMenu WirelessMenu(&Display);  // or you can still call false to force mechanical input selection
EditMenu ServoMenu(&Display);

//Encoder encoder(EN1_PIN, EN2_PIN);

void setup() {

  Serial.begin(115200);

  // button in the encoder
  pinMode(SE_PIN, INPUT_PULLUP);

  // fire up the display
  Display.initR(INITR_GREENTAB);
  Display.setRotation(3);

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
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH_UP, 1);

  MainMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, 20, 3, "Main", FONT_SMALL, FONT_TITLE);

  MenuOption1 = MainMenu.addNI("Options");
  MenuOption2 = MainMenu.addNI("Wireless");
  MenuOption3 = MainMenu.addNI("Servos");

  MainMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  MainMenu.setTitleBarSize(0, 0, 160, 20);
  MainMenu.setTitleTextMargins(10, 15);
  MainMenu.setMenuBarMargins(0, 160, 2, 1);
  MainMenu.setItemTextMargins(10, 15, 5);
  MainMenu.setItemColors(C_GREY, MENU_SELECTBORDER);

  OptionMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                  DATA_COLUMN, 30, 3, "Options", FONT_SMALL, FONT_TITLE);

  int addNI(const char *ItemText, float Data, float LowLimit, float HighLimit,
            float Increment, byte DecimalPlaces = 0, const char **ItemMenuText = NULL);

  OptionOption1 = OptionMenu.add565("Temp.", Temp2Adj, -1.0, 1.0, .05, 2, NULL);
  OptionOption2 = OptionMenu.add565("Read", 2, 0, sizeof(ReadoutItems) / sizeof(ReadoutItems[0]), 1, 0, ReadoutItems);
  OptionOption3 = OptionMenu.add565("Tune", 0, 0, 20, 1, 0, NULL);

  OptionMenu.SetItemValue(OptionOption1, 0.12);
  OptionMenu.SetItemValue(OptionOption3, 1);
  OptionMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  OptionMenu.setTitleBarSize(0, 0, 160, 30);
  OptionMenu.setTitleTextMargins(10, 5);
  OptionMenu.setItemTextMargins(10, 10, 5);
  OptionMenu.setMenuBarMargins(0, 160, 2, 1);
  OptionMenu.setItemColors(C_GREY, MENU_SELECTBORDER, MENU_HIGHBORDER);

  WirelessMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                    DATA_COLUMN, 30, 3, "Wireless Menu", FONT_SMALL, FONT_TITLE);
  WirelessOption1 = WirelessMenu.addNI("Address", 0, 0, 255, 1, 0);
  WirelessOption2 = WirelessMenu.addNI("Data", 2, 0, sizeof(DataRateItems) / sizeof(DataRateItems[0]), 1, 0, DataRateItems);
  WirelessOption3 = WirelessMenu.addNI("Air", 2, 0, sizeof(DataRateItems) / sizeof(DataRateItems[0]), 1, 0, DataRateItems);
  WirelessOption4 = WirelessMenu.addNI("Error", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);

  WirelessMenu.setTitleBarSize(0, 0, 160, 30);
  WirelessMenu.setTitleTextMargins(10, 5);
  WirelessMenu.setItemTextMargins(10, 10, 5);
  WirelessMenu.setMenuBarMargins(0, 160, 2, 1);
  WirelessMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  WirelessMenu.setItemColors(C_GREY, MENU_SELECTBORDER, MENU_HIGHBORDER);

  ServoMenu.init(MENU_TEXT, MENU_BACKGROUND, MENU_HIGHLIGHTTEXT, MENU_HIGHLIGHT, MENU_SELECTTEXT, MENU_SELECT,
                 DATA_COLUMN, 20, 5, "Servos", FONT_SMALL, FONT_SMALL);
  ServoMenu1 = ServoMenu.addNI("Address", 0, 0, 255, 1, 0);
  ServoMenu2 = ServoMenu.addNI("Prec", 2, 0, sizeof(PrecisionItems) / sizeof(PrecisionItems[0]), 1, 0, PrecisionItems);
  ServoMenu3 = ServoMenu.addNI("Tune", 2, 0, sizeof(TuneItems) / sizeof(TuneItems[0]), 1, 0, TuneItems);
  ServoMenu4 = ServoMenu.addNI("State", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);
  ServoMenu5 = ServoMenu.addNI("Tune", 2, 0, sizeof(TuneItems) / sizeof(TuneItems[0]), 1, 0, TuneItems);
  ServoMenu6 = ServoMenu.addNI("State", 1, 0, sizeof(OffOnItems) / sizeof(OffOnItems[0]), 1, 0, OffOnItems);

  ServoMenu.setTitleBarSize(0, 0, 160, 20);
  ServoMenu.setTitleTextMargins(10, 15);
  ServoMenu.setItemTextMargins(10, 15, 10);
  ServoMenu.setMenuBarMargins(0, 160, 2, 1);
  ServoMenu.setTitleColors(TITLE_TEXT, TITLE_BACK);
  ServoMenu.setItemColors(C_GREY, MENU_SELECTBORDER, MENU_HIGHBORDER);

  ProcessMainMenu();

  // menu code done, now proceed to your code
  Display.fillScreen(MENU_BACKGROUND);
}

void loop() {
}

// function to process main menu iteraction
// ideally this implementation makes it easy to launch your menu from anywhere in th
void ProcessMainMenu() {

  // set an inital flag that will be used to store what menu item the user exited on
  int MainMenuOption = 1;

  // blank out the screen
  Display.fillScreen(MENU_BACKGROUND);

  // draw the main menu
  MainMenu.draw();

  // run the processing loop until user move selector to title bar (which becomes exit, i.e. 0 return val)
  // and selectes it
  // note menu code can return - 1 for errors so run unitl non zero

  while (MainMenuOption != 0) {

    // standard encoder read
    Position = ss.getEncoderPosition(); //encoder.read();
    delay(DEBOUNCE);
    // attempt to debouce these darn things...
    if ((Position - oldPosition) > 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition(); //encoder.read();
        delay(DEBOUNCE);
      }

      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are increasing
      // any menu wrapping is handled in the library
      MainMenu.MoveUp();
    }
    // attempt to debouce these darn things...
    if ((Position - oldPosition) < 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition(); //encoder.read();
        delay(DEBOUNCE);
      }
      // once encoder calms down and is done cycling, move selector up
      // since encoder reads are decreasing
      // any menu wrapping is handled in the library
      MainMenu.MoveDown();
    }

    // but wait...the user pressed the button on the encoder
    //if (digitalRead(SE_PIN) == LOW) {
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
        // item 1 was the Option menu
        ProcessOptionMenu();
        // when done processing that menu, return here
        // clear display and redraw this main menu
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption2) {
        ProcessWirelessMenu();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }

      if (MainMenuOption == MenuOption3) {
        ProcessServoMenu();
        Display.fillScreen(MENU_BACKGROUND);
        MainMenu.draw();
      }
    }
  }

  // at this point MenuOption better be 0...
}

// menu to handle processing for a sub-menu
// since this menu will be a menu that allows edits (EditMenu object type)
// process is exactly the same as an ItemMenu
// meaning you simply use the same MoveUp, MoveDown and the library will know if you are
// wanting to move the selector or cycle through a range

void                                                      () {

  // the entire menu processing are basically 3 calls
  // YourMenu.MoveUp();
  // YourMenu.MoveDown();
  // EditMenuOption = YourMenu.selectRow();

  // set an inital flag that will be used to store what menu item the user exited on
  int EditMenuOption = 1;

  // blank out the screen
  Display.fillScreen(MENU_BACKGROUND);

  // draw the main menu
  OptionMenu.draw();

  // run the processing loop until user move selector to title bar (which becomes exit)
  // and selectes it
  while (EditMenuOption != 0) {

    // standard encoder read
    Position = ss.getEncoderPosition(); // encoder.read();
    delay(DEBOUNCE);
    // attempt to debouce these darn things...
    if ((Position - oldPosition) > 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();// encoder.read();
        delay(DEBOUNCE);
      }

      OptionMenu.MoveUp();
    }

    if ((Position - oldPosition) < 0) {

      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition();// encoder.read();
        delay(DEBOUNCE);
      }

      OptionMenu.MoveDown();
    }

    if (!ss.digitalRead(SS_SWITCH_SELECT)) {

      // debounce the selector button
      while (!ss.digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }
      // use the selectRow to
      // 1. select a row for editing
      // a. when a row is selected, moveup, movedown will then scroll through the editable values (values or a list)
      // 2. unselect a row when editing is done
      // 3. when selector is on the title bar annd selecRow is called a 0 is returned

      EditMenuOption = OptionMenu.selectRow();
    }
  }
}


void ProcessWirelessMenu() {

  int EditMenuOption = 1;

  Display.fillScreen(MENU_BACKGROUND);

  WirelessMenu.draw();

  while (EditMenuOption != 0) {

    Position = ss.getEncoderPosition(); //encoder.read();


    if ((Position - oldPosition) > 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition(); // encoder.read();
        delay(DEBOUNCE);
      }


      WirelessMenu.MoveUp();
    }

    if ((Position - oldPosition) < 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition(); //encoder.read();
        delay(DEBOUNCE);
      }

      WirelessMenu.MoveDown();
    }

    if (!ss.digitalRead(SS_SWITCH_SELECT)) {

      // debounce the selector button
      while (!ss.digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      EditMenuOption = WirelessMenu.selectRow();
    }
  }
}

void ProcessServoMenu() {

  int EditMenuOption = 1;

  Display.fillScreen(MENU_BACKGROUND);

  ServoMenu.draw();

  while (EditMenuOption != 0) {

    Position = ss.getEncoderPosition(); // encoder.read();
    delay(DEBOUNCE);
    if ((Position - oldPosition) > 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition(); // encoder.read();
        delay(DEBOUNCE);
      }
      ServoMenu.MoveUp();
    }

    if ((Position - oldPosition) < 0) {
      while (oldPosition != Position) {
        oldPosition = Position;
        Position = ss.getEncoderPosition(); // encoder.read();
        delay(DEBOUNCE);
      }
      ServoMenu.MoveDown();
    }

    if (!ss.digitalRead(SS_SWITCH_SELECT)) {

      // debounce the selector button
      while (!ss.digitalRead(SS_SWITCH_SELECT)) {
        delay(DEBOUNCE);
      }

      EditMenuOption = ServoMenu.selectRow();
    }
  }
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// end of example
//////////////////////////////////////////////////////////////////////////////////////////////////
