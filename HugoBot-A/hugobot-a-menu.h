#include "Colors.h"

// found in \Arduino\libraries\Adafruit-GFX-Library-master
#include "fonts\FreeSans9pt7b.h"
#include "fonts\FreeSans12pt7b.h"
#include "fonts\FreeSans18pt7b.h"
#include "fonts\FreeSansBold9pt7b.h"
#include "fonts\FreeSansBold12pt7b.h"
#include "fonts\FreeSansOblique9pt7b.h"

#define SEESAW_ADDR      0x49
#define SS_SWITCH_SELECT 1
#define SS_SWITCH_UP     2
#define SS_SWITCH_LEFT   3
#define SS_SWITCH_DOWN   4
#define SS_SWITCH_RIGHT  5

#define DATA_COLUMN 85

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
