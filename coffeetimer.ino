#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <LiquidCrystal.h>
#include <stdbool.h>

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

unsigned long LAST_PRESS_THRESHOLD_SECONDS = 2;

unsigned long lastPress = -1;

long brew_start = -1;
long brew_start_temp = -1;
long brew_end = -1;
bool is_brewing = false;
long brewing_until = -1;

void setup()
{
  lcd.begin(16, 2);
  lcd.print("Coffee last made:");
  Serial.begin(9600);          //  setup serial
}

void loop() {
  unsigned long now = millis();
  checkAndResetMillisOverflow(now);
  unsigned long diff = now - lastPress;
  unsigned long seconds = diff / 1000;
  
  uint8_t buttons = lcd.readButtons();
  int lightSensor = analogRead(0);    // read the input pin
  bool coffeeReset = false; 

  if (lightSensor > 100 || buttons) {
      lastPress = now;
      coffeeReset = true;
  }

  if (is_brewing) {
    coffeeIsBrewing(seconds, now);
  }
  else {
    // coffee is ready to be enjoyed
    displayCoffeeIsReady(seconds);
    
    if(coffeeReset) {
      // startBrew() needs to be nested within !is_brewing
      startBrew(now);
    }
  }
}

/* 
  Note: getBrewDuration() returns an 8 byte signed number because '-1' represents when 
        the brew durationi is unknown. But it still needs to be able to return 4 bytes
        of positive values, corresponding with the valid values for 'millis()'.
        https://www.arduino.cc/en/Reference/Millis
*/
long long getBrewDuration() {
  return (brew_start < 0 || brew_end < 0) ? -1 : (brew_end - brew_start);
}

void endBrew(unsigned long now) {
  is_brewing = false;
  brew_end = now;
  brew_start = brew_start_temp;
  brewing_until = -1;
  lastPress = now;
  lcd.setBacklight(GREEN);
}

void startBrew(unsigned long now) {
  brew_start_temp = now;
  is_brewing = true;

  // 'long long' is very intentional
  long long brewDuration = getBrewDuration();
  brewing_until = brewDuration > 0 ? now + brewDuration : -1;
}

void displayCoffeeIsReady(unsigned long seconds) {
  lcd.setCursor(0, 0);
  lcd.print("Coffee last made:   ");
  lcd.setCursor(0, 1);
  char buffer[12];
  
  if (lastPress == -1) {
      lcd.print("long ago? maybe.");
      lcd.setBacklight(YELLOW);
  }
  else {
    lcd.setBacklight(GREEN);  
    if (seconds < 1) {
        lcd.print("right now       ");
    }
    else if (seconds < 60) {
        lcd.print(itoa(seconds, buffer, 10));
        lcd.print("s ago         ");
    }
    else if (seconds < 60 * 60) {
        lcd.print(itoa(seconds/60, buffer, 10));
        lcd.print("m ago         ");
    }
    else if (seconds < 60 * 60 * 4) {
        lcd.print(itoa(seconds/60/60, buffer, 10));
        lcd.print(seconds/60 % 60 > 30 ? ".5" : "");
        lcd.print("h ago        ");
    }
    else {
      lcd.setBacklight(RED);
      lcd.print("Too long ago!    ");
    }
  }
}

void coffeeIsBrewing(unsigned long seconds, unsigned long now) {
  // in brewing mode
  lcd.setCursor(0, 0);
  lcd.print("Coffee is brew'n");
  lcd.setCursor(0, 1);
  lcd.setBacklight(BLUE);
  char buff[12];

  if (brewing_until > 0 && now < brewing_until) {
    // brewing, and the end time has been learned 
    // display countdown
    long brewDiffSeconds = (brewing_until - now) / 1000;
    long minutesRemaining = brewDiffSeconds / 60;
    long secondsRemaining = brewDiffSeconds % 60;

    lcd.print("Countdown: ");
    lcd.print(itoa(minutesRemaining, buff, 10));
    if (secondsRemaining < 10) {
      lcd.print(":0");  
    }
    else {
      lcd.print(":");
    }
    lcd.print(itoa(secondsRemaining, buff, 10));
    lcd.print(" ");
    endIfReady(seconds, now);
  }
  else {
    endIfReady(seconds, now);
  }
}

void endIfReady(unsigned long seconds, unsigned long now) {
  // coffee is brewing, end time unknown
  if (seconds > LAST_PRESS_THRESHOLD_SECONDS) {
    // this IF needs to be nested
    endBrew(now);
    // clear LCD
    lcd.print("                ");
  }
  else {
    lcd.print("Still...        ");
  }
}

void checkAndResetMillisOverflow(unsigned long now) {
  if (now < lastPress) {  
    lastPress = -1;
    brew_start = -1;
    brew_start_temp = -1;
    brew_end = -1;
    brewing_until = -1;
  }
}


