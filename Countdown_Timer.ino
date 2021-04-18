#include <TimeLib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Button.h>
#include <DS3231.h>

byte segmentClock = 6;
byte segmentLatch = 5;
byte segmentData = 7;

int x = 0;
int setupmark = 1;
int hold = 0;
int pos = 1;
int mmax = 0;
int ErrorState = 0;

int nowday = 0;
int nowmonth = 0;
int nowyear = 0;

int targetday = 1;
int targetmonth = 1;
int targetyear = 20;

int printyear = 0;

time_t current = 0;
time_t target = 0;
time_t deltas = 0;
tmElements_t tgt;
tmElements_t cur;

int deltad = 0;

int endvar = 0;
int startup1 = 1;
int startup2 = 0;

int power = 1;

const int red = 3;
const int green = 2;

int inmenu = 0;
int toplevel = 1;
int menupos = 1;
int automode = 0;
int aboutpage = 1;
int displayclock = 0;
int settimeoff = 0;
int settimeon = 0;
int ontimepos = 1;
int offtimepos = 1;
int offtimemin = 0;
int offtimehour = 0;
int ontimehour = 0;
int about = 1;
int ontimemin = 0;
int resetmenu = 0;

Button menu = Button(11, BUTTON_PULLDOWN);
Button up = Button(10, BUTTON_PULLDOWN);
Button down = Button(9, BUTTON_PULLDOWN);
Button left = Button(8, BUTTON_PULLDOWN);
Button mode = Button(4, BUTTON_PULLDOWN);

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,16,2);

RTClib RTC;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

DateTime now = RTC.now();

pinMode(red, OUTPUT);
pinMode(green, OUTPUT);

pinMode(segmentClock, OUTPUT);
pinMode(segmentData, OUTPUT);
pinMode(segmentLatch, OUTPUT);

digitalWrite(segmentClock, LOW);
digitalWrite(segmentData, LOW);
digitalWrite(segmentLatch, LOW);

Serial.begin(9600);

Wire.begin();

digitalWrite(red, HIGH);
digitalWrite(green, LOW);

postNumber(' ', false);
postNumber(' ', false);
//postNumber(' ', false); <<<<<<<<<<<<<<<<<<<<<<<<< Add digit #3
digitalWrite(segmentLatch, LOW);
digitalWrite(segmentLatch, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:

DateTime now = RTC.now();

while (startup1 == 1){
  power = 1;
  lcd.display();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  targetday = now.day() + 1;
  targetmonth = now.month();
  targetyear = now.year() - 2000;
  pos = 1;
  
  //write setup screen
  lcd.setCursor(0,0);
  lcd.print("Target:");
  lcd.setCursor(8,0);
  lcd.print("mm/dd/yy");
  lcd.setCursor(8,1);
  lcd.print("      ^^");
  startup1 = 0;
  startup2 = 1;
  delay(200);
}

//enter setup loop
while (setupmark == 1){
  lcd.display();
  lcd.backlight();
  postNumber(' ', false);
  postNumber(' ', false);
  //postNumber(' ', false); <<<<<<<<<<<<<<<<<<<<<<<<< Add digit #3
  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);

//when menu button is pressed, see if the number of days in the month is okay. If so, exit setup loop.
  if (menu.isPressed()){
     if ((targetmonth == 1) || (targetmonth == 3) || (targetmonth == 5) || (targetmonth == 7) || (targetmonth == 8) || (targetmonth == 10) || (targetmonth == 12)){
        mmax = 31;
      }
      if (targetmonth == 2){
        mmax = 28;
      }
      if ((targetmonth == 4) || (targetmonth == 6) || (targetmonth == 9) || (targetmonth == 11)){
        mmax = 30;
      }
    if (targetday > mmax){
      lcd.setCursor(0,1);
      lcd.print("Error");
      delay(200);
      ErrorState = 1;
    }
    else {
    setupmark = 0;
    startup2 = 1;

    tgt.Day = targetday;
    tgt.Month = targetmonth-1;
    tgt.Year = targetyear - 1970+2000;
    target = makeTime(tgt);
   
    }
  }
  if (ErrorState == 1){
    if ((left.isPressed()) || (up.isPressed()) || (down.isPressed()) || (menu.isPressed()) || (mode.isPressed())){
      ErrorState == 0;
      lcd.setCursor(0,1);
      lcd.print("     ");
      delay(200);
    }
  }
  //change selection with left button
  if (left.isPressed()){
    if (pos < 3){
      pos++;
    }
    else {
      pos = 1;
    }
    lcd.setCursor(8,1);
    if (pos == 1){
      lcd.print("      ^^");
    }
    if (pos == 2){
      lcd.print("   ^^   ");
    }
    if (pos == 3){
      lcd.print("^^     ");
    }
  }
  //pos 1: change year
  if (pos == 1){
    if (up.isPressed()){
      targetyear++;
    }
    if (down.isPressed()){
      targetyear--;
    }
   if (targetyear < 0){
    targetyear = 99;
   }
   if (targetyear > 99){
    targetyear = 0;
   }
    lcd.setCursor(14,0);
    if (targetyear < 10){
      lcd.print("0");
    }
    lcd.print(targetyear);
  }
  //pos 2: change day, must fit within assigned month
    if (pos == 2){
      if ((targetmonth == 1) || (targetmonth == 3) || (targetmonth == 5) || (targetmonth == 7) || (targetmonth == 8) || (targetmonth == 10) || (targetmonth == 12)){
        mmax = 31;
      }
      if (targetmonth == 2){
        mmax = 28;
      }
      if ((targetmonth == 4) || (targetmonth == 6) || (targetmonth == 9) || (targetmonth == 11)){
        mmax = 30;
      }
    if (up.isPressed()){
      targetday++;
    }
    if (down.isPressed()){
      targetday--;
    }
   if (targetday < 1){
    targetday = mmax;
   }
   if (targetday > mmax){
    targetday = 1;
   }
    lcd.setCursor(11,0);
    if (targetday < 10){
      lcd.print("0");
    }
    lcd.print(targetday);
  }
  //pos 3: change day, does not retroactively check if day is okay, will be caught in attempt to exit loop.
    if (pos == 3){
    if (up.isPressed()){
      targetmonth++;
    }
    if (down.isPressed()){
      targetmonth--;
    }
   if (targetmonth < 1){
    targetmonth = 12;
   }
   if (targetmonth > 12){
    targetmonth = 1;
   }
    lcd.setCursor(8,0);
    if (targetmonth < 10){
      lcd.print("0");
    }
    lcd.print(targetmonth);
  }


delay(200);
}

if (startup2 == 1){
//exiting startup sequence, print main screen
  digitalWrite(green, LOW);
  targetyear = targetyear + 2000;
  lcd.display();
  lcd.backlight();
  power = 1;
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Today: ");
  lcd.setCursor(0,1);
  lcd.print("Target: ");
  
  lcd.setCursor(10,0);
  lcd.print("/");
  lcd.setCursor(10,1);
  lcd.print("/");
  lcd.setCursor(13,0);
  lcd.print("/");
  lcd.setCursor(13,1);
  lcd.print("/");
  
  nowmonth = now.month();
  if (nowmonth < 10){
    lcd.setCursor(8,0);
    lcd.print("0");
    lcd.setCursor(9,0);
    lcd.print(nowmonth);
  }
  else {
  lcd.setCursor(8,0);
  lcd.print(nowmonth);
  }

  nowday = now.day();
  if (nowday < 10){
    lcd.setCursor(11,0);
    lcd.print("0");
    lcd.setCursor(12,0);
    lcd.print(nowday);
  }
  else {
  lcd.setCursor(11,0);
  lcd.print(nowday); 
  }

  nowyear = now.year();
  lcd.setCursor(14,0);
  printyear = nowyear - 2000;
  lcd.print(nowyear);

  if (targetday < 10){
  lcd.setCursor(11,1);
  lcd.print("0");
  lcd.setCursor(12,1);
  lcd.print(targetday);
  }
  else {
  lcd.setCursor(11,1);
  lcd.print(targetday);
  }

  if (targetmonth < 10){
  lcd.setCursor(8,1);
  lcd.print("0");
  lcd.setCursor(9,1);
  lcd.print(targetmonth);
  }
  else {
  lcd.setCursor(8,1);
  lcd.print(targetmonth);
  }

  lcd.setCursor(14,1);
  lcd.print(targetyear);

startup2 = 0;
delay(20);
}


if (now.year() != nowyear){
  nowyear = now.year();
  lcd.setCursor(14,0);
  printyear = nowyear - 2000;
  lcd.print(nowyear);
}

if (now.month() != nowmonth){
  nowmonth = now.month();
  if (nowmonth < 10){
    lcd.setCursor(8,0);
    lcd.print("0");
    lcd.setCursor(9,0);
    lcd.print(nowmonth);
  }
  else {
  lcd.setCursor(8,0);
  lcd.print(nowmonth);
}
}

if(now.day() != nowday){
  nowday = now.day();
  if (nowday < 10){
    lcd.setCursor(11,0);
    lcd.print("0");
    lcd.setCursor(12,0);
    lcd.print(nowday);
  }
  else {
  lcd.setCursor(11,0);
  lcd.print(nowday); 
  }
}

cur.Day = nowday;
cur.Month = nowmonth-1;
cur.Year = nowyear - 1970;
current = makeTime(cur);

deltas = target - current;
deltad = deltas / 86400;

if (automode == 1){
  if ((now.hour() == offtimehour) && (now.minute() == offtimemin) && (now.second() == 0)){
    power = 0;
  }
  if ((now.hour() == ontimehour) && (now.minute() == ontimemin) && (now.second() == 0)){
    power = 1;
  }
}

if (mode.isPressed()){
  if (power == 1){
    power = 0;
  }
  else {
    power = 1;
  }
  delay(200);
}

if (power == 1){
  digitalWrite(red, HIGH);
  digitalWrite(green, LOW);
  lcd.display();
  lcd.backlight();
  if (deltad > 999){
    showNumber(999);
  }
  else {
  showNumber(deltad);
  }
  digitalWrite(segmentLatch, LOW);
  digitalWrite(segmentLatch, HIGH);
}

if (power==0){
  postNumber(' ', false);
  postNumber(' ', false);
  //postNumber(' ', false); <<<<<<<<<<<<<<<<<<<<<<<<< Add digit #3
  digitalWrite(segmentLatch, LOW);
  digitalWrite(segmentLatch, HIGH);
  digitalWrite(red, LOW);
  digitalWrite(green, HIGH);
  lcd.noDisplay();
  lcd.noBacklight();
}

if (deltad == 0){
  power=1;
  lcd.display();
  lcd.backlight();
  digitalWrite(red, HIGH);
  digitalWrite(green, LOW);
  lcd.clear();
  lcd.print("Countdown Over");
  lcd.setCursor(0,1);
  lcd.print("Press any Button");
  endvar = 1;
}

while (endvar == 1){
  if ((menu.isPressed()) || (left.isPressed()) || (up.isPressed()) || (down.isPressed()) || (mode.isPressed())){
    endvar = 0;
    setupmark = 1;
    startup1 = 1;
  }
}

if (menu.isPressed()){
  inmenu = 1;
  toplevel = 1;
  lcd.clear();
  lcd.backlight();
  lcd.display();
  delay(200);
}

while (inmenu == 1){
  if (toplevel == 1){

      
    lcd.setCursor(0,0);
    lcd.print("Settings: ^ v");
    if (up.isPressed()){
      menupos++;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Settings: ^ v");
    }
    if (down.isPressed()){
      menupos--;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Settings: ^ v");
    }
    if (menupos > 6){
      menupos = 1;
    }
    if (menupos < 1){
      menupos = 6;
    }
    if (menupos == 1){
      lcd.setCursor(11,1);
      lcd.print("About");
      if (left.isPressed()){
        lcd.clear();
        about = 1;
        while (about == 1){
          if ((up.isPressed()) || (down.isPressed())){
            lcd.clear();
            if (aboutpage == 1){
              aboutpage = 2;
            }
            else{
              aboutpage = 1;
            }
          }
          if (aboutpage == 1){
            lcd.setCursor(0,0);
            lcd.print("Dev: Jim Heaney^");
            lcd.setCursor(0,1);
            lcd.print("jim-heaney.com v");
          }
          if (aboutpage == 2){
            lcd.setCursor(0,0);
            lcd.print("The Construct  ^");
            lcd.setCursor(0,1);
            lcd.print("@RIT July 2020 v");
          }
          delay(200);
          if (menu.isPressed()){
            about = 0;
            lcd.clear();
            delay(200);
          }
        }
      }
    }
    if (menupos == 3){
      if (automode == 0){
        lcd.setCursor(2,1);
        lcd.print("Auto-Time: Off");
      }
      if (automode == 1){
        lcd.setCursor(2,1);
        lcd.print("Auto-Time: On");
      }
      if (left.isPressed()){
        lcd.clear();
        if (automode == 1){
          automode = 0;
        }
        else {
          automode = 1;
        }
      }
    }
      if (menupos == 2){
        lcd.setCursor(3,1);
        lcd.print("Display Clock");
        if (left.isPressed()){
          displayclock = 1;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Date:   /  /");
          lcd.setCursor(0,1);
          lcd.print("Time:   :  :");
          delay(200);
        }
        while (displayclock == 1){
          
          DateTime now = RTC.now();
          lcd.setCursor(6,0);
          if (now.month() >10){
            lcd.print(now.month());
          }
          else{
            lcd.print("0");
            lcd.print(now.month());
          }
          lcd.setCursor(9,0);
          if (now.day() > 10){
            lcd.print(now.day());
          }
          else{
            lcd.print("0");
            lcd.print(now.day());
          }
          lcd.setCursor(12,0);
          lcd.print(now.year());
          lcd.setCursor(6,1);
            if (now.hour() >10){
              lcd.print(now.hour());
            }
            else{
              lcd.print("0");
              lcd.print(now.hour());
            }
          
          lcd.setCursor(9,1);
          if (now.minute() > 10){
            lcd.print(now.minute());
          }
          else {
            lcd.print("0");
            lcd.print(now.minute());
          }
          lcd.setCursor(12,1);
          if (now.second() > 10){
            lcd.print(now.second());
          }
          else {
            lcd.print("0");
            lcd.print(now.second());
          }
          delay(200);
                    if ((left.isPressed()) || (menu.isPressed())){
            lcd.clear();
            delay(20);
            displayclock = 0;
            toplevel == 1;
                    }
        }
      }
    if (menupos == 4){
      lcd.setCursor(4,1);
      lcd.print("Set Time Off");
      if (left.isPressed()){
        lcd.clear();
        settimeoff = 1;
      }
      while (settimeoff == 1){
        if (offtimehour > 23){
          offtimehour = 0;
        }
        if (offtimehour < 0){
          offtimehour = 23;
        }
        if (offtimemin > 59){
          offtimemin = 0;
        }
        if (offtimemin < 0){
          offtimemin = 59;
        }
        lcd.setCursor(0,0);
        lcd.print("Time Off: ");
        lcd.setCursor(10,0);
        if (offtimehour > 9){
          lcd.print(offtimehour);
          }
        else{
          lcd.print("0");
          lcd.print(offtimehour);
          }
        lcd.setCursor(12,0);
        lcd.print(":");
        if (offtimemin > 9){
          lcd.print(offtimemin);
          }
        else{
          lcd.print("0");
          lcd.print(offtimemin);
          }
        if (left.isPressed()){
          if (offtimepos == 1){
            offtimepos = 2;
          }
          else{
            offtimepos = 1;
          }
        if (offtimepos == 1){
          lcd.setCursor(10, 1);
          lcd.print("   ^^");  
          lcd.setCursor(13,0);
        }
        if (offtimepos == 2){
          lcd.setCursor(10, 1);
          lcd.print("^^   ");  
        }
        }
        delay(200);
        if (up.isPressed()){
          if (offtimepos == 1){
            offtimemin = offtimemin + 1;
          }
          if (offtimepos == 2){
            offtimehour = offtimehour + 1;
          }
        }
        if (down.isPressed()){
          if (offtimepos == 1){
            offtimemin = offtimemin - 1;
          }
          if (offtimepos == 2){
            offtimehour = offtimehour - 1;
          }
        }
        if (menu.isPressed()){
          settimeoff = 0;
          lcd.clear();
        } 
      }
    }
    if (menupos == 5){
      lcd.setCursor(4,1);
      lcd.print("Set Time On");
      if (left.isPressed()){
        lcd.clear();
        settimeon = 1;
      }
      while (settimeon == 1){
        if (ontimehour > 23){
          ontimehour = 0;
        }
        if (ontimehour < 0){
          ontimehour = 23;
        }
        if (ontimemin > 59){
          ontimemin = 0;
        }
        if (ontimemin < 0){
          ontimemin = 59;
        }
        lcd.setCursor(0,0);
        lcd.print("Time On: ");
        lcd.setCursor(10,0);
        if (ontimehour > 9){
          lcd.print(ontimehour);
          }
        else{
          lcd.print("0");
          lcd.print(ontimehour);
          }
        lcd.setCursor(12,0);
        lcd.print(":");
        if (ontimemin > 9){
          lcd.print(ontimemin);
          }
        else{
          lcd.print("0");
          lcd.print(ontimemin);
          }
        if (left.isPressed()){
          if (ontimepos == 1){
            ontimepos = 2;
          }
          else{
            ontimepos = 1;
          }
        if (ontimepos == 1){
          lcd.setCursor(10, 1);
          lcd.print("   ^^");  
          lcd.setCursor(13,0);
        }
        if (ontimepos == 2){
          lcd.setCursor(10, 1);
          lcd.print("^^   ");  
        }
        }
        delay(200);
        if (up.isPressed()){
          if (ontimepos == 1){
            ontimemin = ontimemin + 1;
          }
          if (ontimepos == 2){
            ontimehour = ontimehour + 1;
          }
        }
        if (down.isPressed()){
          if (ontimepos == 1){
            ontimemin = ontimemin - 1;
          }
          if (ontimepos == 2){
            ontimehour = ontimehour - 1;
          }
        }
        if (menu.isPressed()){
          settimeon = 0;
          lcd.clear();
        } 
      }
    }
    if (menupos == 6){
      lcd.setCursor(3,1);
      lcd.print("Reset Target?");
      if (left.isPressed()){
        lcd.clear();
        resetmenu = 1;
      }
      while (resetmenu == 1){
        if (menu.isPressed()){
          resetmenu = 0;
          lcd.clear();
        }
        lcd.setCursor(0,0);
        lcd.print("Are you sure?");
        lcd.setCursor(0,1);
        lcd.print("^: Yes, v: No");
        if (up.isPressed()){
          inmenu = 0;
          toplevel = 0;
          resetmenu = 0;
          endvar = 0;
          setupmark = 1;
          startup1 = 1;
          postNumber(' ', false);
          postNumber(' ', false);
          //postNumber(' ', false); <<<<<<<<<<<<<<<<<<<<<<<<< Add digit #3
          digitalWrite(segmentLatch, LOW);
          digitalWrite(segmentLatch, HIGH);
          lcd.clear();
          }
        if (down.isPressed()){
          resetmenu = 0;
          lcd.clear();
        }
        delay(200);
        }
      }
    }
  delay(200);
  if (menu.isPressed()){
  inmenu = 0;
  lcd.clear();
  delay(20);
  lcd.display();
  lcd.backlight();
  power = 1;
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Today: ");
  lcd.setCursor(0,1);
  lcd.print("Target: ");
  
  lcd.setCursor(10,0);
  lcd.print("/");
  lcd.setCursor(10,1);
  lcd.print("/");
  lcd.setCursor(13,0);
  lcd.print("/");
  lcd.setCursor(13,1);
  lcd.print("/");
  
  nowmonth = now.month();
  if (nowmonth < 10){
    lcd.setCursor(8,0);
    lcd.print("0");
    lcd.setCursor(9,0);
    lcd.print(nowmonth);
  }
  else {
  lcd.setCursor(8,0);
  lcd.print(nowmonth);
  }

  nowday = now.day();
  if (nowday < 10){
    lcd.setCursor(11,0);
    lcd.print("0");
    lcd.setCursor(12,0);
    lcd.print(nowday);
  }
  else {
  lcd.setCursor(11,0);
  lcd.print(nowday); 
  }

  nowyear = now.year();
  lcd.setCursor(14,0);
  printyear = nowyear - 2000;
  lcd.print(nowyear);

  if (targetday < 10){
  lcd.setCursor(11,1);
  lcd.print("0");
  lcd.setCursor(12,1);
  lcd.print(targetday);
  }
  else {
  lcd.setCursor(11,1);
  lcd.print(targetday);
  }

  if (targetmonth < 10){
  lcd.setCursor(8,1);
  lcd.print("0");
  lcd.setCursor(9,1);
  lcd.print(targetmonth);
  }
  else {
  lcd.setCursor(8,1);
  lcd.print(targetmonth);
  }

  lcd.setCursor(14,1);
  lcd.print(targetyear);
  digitalWrite(green, LOW);
  digitalWrite(red, HIGH);
      }
delay(200);
}
delay(200);
}

void showNumber(float value)
{
  int number = abs(value); //Remove negative signs and any decimals

  //Serial.print("number: ");
  //Serial.println(number);

  for (byte x = 0 ; x < 2 ; x++) // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Set # of digits here
  {
    int remainder = number % 10;

    postNumber(remainder, false);

    number /= 10;
  }

  //Latch the current segment data
  digitalWrite(segmentLatch, LOW);
  digitalWrite(segmentLatch, HIGH); //Register moves storage register on the rising edge of RCK
}

//Given a number, or '-', shifts it out to the display
void postNumber(byte number, boolean decimal)
{
  //    -  A
  //   / / F/B
  //    -  G
  //   / / E/C
  //    -. D/DP

#define a  1<<0
#define b  1<<6
#define c  1<<5
#define d  1<<4
#define e  1<<3
#define f  1<<1
#define g  1<<2
#define dp 1<<7

  byte segments;

  switch (number)
  {
    case 1: segments = b | c; break;
    case 2: segments = a | b | d | e | g; break;
    case 3: segments = a | b | c | d | g; break;
    case 4: segments = f | g | b | c; break;
    case 5: segments = a | f | g | c | d; break;
    case 6: segments = a | f | g | e | c | d; break;
    case 7: segments = a | b | c; break;
    case 8: segments = a | b | c | d | e | f | g; break;
    case 9: segments = a | b | c | d | f | g; break;
    case 0: segments = a | b | c | d | e | f; break;
    case ' ': segments = 0; break;
    case 'c': segments = g | e | d; break;
    case '-': segments = g; break;
  }

  if (decimal) segments |= dp;

  //Clock these bits out to the drivers
  for (byte x = 0 ; x < 8 ; x++)
  {
    digitalWrite(segmentClock, LOW);
    digitalWrite(segmentData, segments & 1 << (7 - x));
    digitalWrite(segmentClock, HIGH); //Data transfers to the register on the rising edge of SRCK
  }
}

