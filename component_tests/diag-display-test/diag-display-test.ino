#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS         0
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8

Adafruit_ST7735 tft = Adafruit_ST7735(&SPI1, TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;

float x = 0.0, y = 0.0, angle = 0.0;
int xTicks = 0, yTicks = 0;
const char* words[] = {"apple", "banana", "cherry", "date", "elderberry", "fig", "grape", "honeydew", "iceberg", "jackfruit", "kiwi", "lemon", "mango", "nectarine", "orange", "pineapple", "quince", "raspberry", "strawberry", "tangerine", "ugli", "victoria", "watermelon", "xigua", "yellow", "zucchini"};
const char* arrayOfStrings[10];

char outputBuffer[50];
char xStr[16];
char yStr[16];
char angleStr[16];

void setup(void) {
  Serial.begin(115200);
  Serial.print(F("Hello! ST77xx Diag Testing"));

  srand(time(NULL)); // initialize random seed

  for(int i = 0; i < 10; i++) {
    int randomIndex = rand() % (sizeof(words) / sizeof(char*));
    arrayOfStrings[i] = words[randomIndex];
    Serial.println(arrayOfStrings[i]);
  }

  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setRotation(1);

  Serial.println(F("Initialized"));

  // uint16_t time = millis();
  // tft.fillScreen(ST77XX_BLACK);
  // time = millis() - time;

  // Serial.println(time, DEC);
  // delay(500);

  // // large block of text
  // tft.fillScreen(ST77XX_BLACK);
  // testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", ST77XX_WHITE);
  // delay(1000);

  // // tft print function!
  // tftPrintTest();

  tft.fillScreen(ST77XX_BLACK);
  displayLog();
  delay(4000);

  Serial.println("done");
}

void loop() {
  int xRand = random(2001) - 1000;
  int yRand = random(2001) - 1000;

  xTicks = xTicks + xRand;
  yTicks = yTicks + yRand;

  x = xTicks * 0.23;
  y = yTicks * 0.41;

  displayLog();

  delay(50);
}

void displayLog() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 5);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  dtostrf(x, 10, 5, xStr);
  dtostrf(y, 10, 5, yStr);
  dtostrf(angle, 9, 5, angleStr);

  outputBuffer[0] = '\0';
  sprintf(outputBuffer, "xt: %d, x: %s", xTicks, xStr);
  tft.println(outputBuffer);
  Serial.println(outputBuffer);

  outputBuffer[0] = '\0';
  sprintf(outputBuffer, "yt: %d, y: %s", yTicks, yStr);
  tft.println(outputBuffer);
  Serial.println(outputBuffer);

  outputBuffer[0] = '\0';
  sprintf(outputBuffer, "Angle: %s", angleStr);
  tft.println(outputBuffer);
  Serial.println(outputBuffer);

  // tft.setTextColor(ST77XX_GREEN);
  // tft.setTextSize(1);
  // for(int i = 0; i < 10; i++) {
  //   tft.println(arrayOfStrings[i]);
  //   Serial.println(arrayOfStrings[i]);
  // }
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}
