#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <LiquidCrystal.h>

#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

int lcd_key     = 0;
int adc_key_in  = 0;

unsigned long lastPress = 0;

void setup()
{
  lcd.begin(16, 2);
  lcd.print("Coffee last made:");
}

void loop() {
  unsigned long now = millis();
  unsigned long diff = now - lastPress;
  uint8_t buttons = lcd.readButtons();
  char buffer[12];

  lcd.setCursor(0, 1);
  unsigned long seconds = diff/1000;
  if (seconds < 60) {
      lcd.print(itoa(diff/1000, buffer, 10));
      lcd.print("s ago    ");
  }
  else if (seconds < 60 * 60) {
      lcd.print(itoa(diff/1000/60, buffer, 10));
      lcd.print("m ago    ");
  }
  else if (seconds < 60 * 60 * 4) {
      lcd.print(itoa(diff/1000/60/60, buffer, 10));
      lcd.print("h ago    ");
  }
  else {
    lcd.setBacklight(RED);
    lcd.print(itoa(diff/1000/60/60, buffer, 10));
    lcd.print("Too long ago!    ");
  }
  
  if(buttons) {
    lastPress = now;
    lcd.setBacklight(GREEN);
  }
}
