/*
  SD card connections:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Sodaq_DS3231.h>
#include "LedControl.h"
LedControl lc = LedControl(7, 8, 9, 10);
String weekDay[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

int ringPIN = 2;

int Alarms_H[20];
int Alarms_M[20];
bool Alarms_E[20];
bool zmianaczasu;

int millis_delay = 10000;
void setup()
{
  lc.shutdown(0, false);
  lc.setIntensity(0, 5);
  lc.clearDisplay(0);
  printString("HELL0");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ringPIN, OUTPUT);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Wire.begin();
  rtc.begin();
  digitalWrite(ringPIN, LOW);

}
unsigned long  millis_last;
unsigned long  millis_reset;
bool mode_day = false;
void loop()
{
  unsigned long  millis_now = millis();
  DateTime time_now = rtc.now();
  int h, m, s, dow;
  h = time_now.hour();
  if(zmianaczasu){
    h+=1;
  }
  m = time_now.minute();
  s = time_now.second();
  dow = time_now.dayOfWeek();
  for (int i = 0; i < 20; i++) {
    if (Alarms_E[i]) {
      if (dow != 0 || dow != 6) {
        if (h == Alarms_H[i]) {
          if (m == Alarms_M[i]) {
            if (s == 0 || s == 1 || s == 2 || s == 3 || s == 4) {
              PrintAlarm((String)i, (String)h, (String)m);
              ringBell();
            } else {
            }
          } else {
          }
        } else {
        }
      }
    }
  }

  if (millis_now - millis_reset > 30000) {
    lc.shutdown(0, false);
    lc.setIntensity(0, 5);
    lc.clearDisplay(0);
    millis_reset = millis_now;
  }

  if (millis_now - millis_last > millis_delay) {
    if (mode_day) {
      mode_day = false;
      millis_last = millis_now;
    } else {
      mode_day = true;
      millis_last = millis_now;
    }
    Serial.print("Time: ");
    Serial.print(h);
    Serial.print(" : ");
    Serial.print(m);
    Serial.print(" : ");
    Serial.print(s);
    Serial.print(" ");
    Serial.println(weekDay[dow]);
  }
  int dy, mh, yr;
  dy = time_now.date();
  mh = time_now.month();
  yr = time_now.year();

  if (mode_day) {
    PrintTime((String)h, (String)m, (String)s, (String)(dow + 1));
  } else {
    PrintData((String)dy, (String)mh, (String)yr);
  }
  //PrintAlarm("3","9","45");
  delay(100);
}

void ringBell() {
  Serial.println("Dzwoni dzwonek.");
  digitalWrite(ringPIN, HIGH);
  delay(8000);
  digitalWrite(ringPIN, LOW);
  Serial.println("Koniec dzwonienia");
}

void PrintTime(String  h, String m, String s, String d) {
  if (h.length() == 1) {
    lc.setChar(0, 7, '0', false);
    lc.setChar(0, 6, h[0], true);
  } else {
    lc.setChar(0, 7, h[0], false);
    lc.setChar(0, 6, h[1], true);
  }

  if (m.length() == 1) {
    lc.setChar(0, 5, '0', false);
    lc.setChar(0, 4, m[0], true);
  } else {
    lc.setChar(0, 5, m[0], false);
    lc.setChar(0, 4, m[1], true);
  }

  if (s.length() == 1) {
    lc.setChar(0, 3, '0', false);
    lc.setChar(0, 2, s[0], false);
  } else {
    lc.setChar(0, 3, s[0], false);
    lc.setChar(0, 2, s[1], false);
  }

  lc.setChar(0, 1, ' ', false);
  lc.setChar(0, 0, d[0], false);

}
void PrintData(String  d, String m, String y) {
  if (d.length() == 1) {
    lc.setChar(0, 7, '0', false);
    lc.setChar(0, 6, d[0], true);
  } else {
    lc.setChar(0, 7, d[0], false);
    lc.setChar(0, 6, d[1], true);
  }


  if (m.length() == 1) {
    lc.setChar(0, 5, '0', false);
    lc.setChar(0, 4, m[0], true);
  } else {
    lc.setChar(0, 5, m[0], false);
    lc.setChar(0, 4, m[1], true);
  }


  lc.setChar(0, 3, y[0], false);
  lc.setChar(0, 2, y[1], false);

  lc.setChar(0, 1, y[2], false);
  lc.setChar(0, 0, y[3], false);
}
void PrintAlarm(String a, String  h,  String m) {
  lc.setChar(0, 7, 'A', false);

  if (a.length() == 1) {
    lc.setChar(0, 6, ' ', false);
    lc.setChar(0, 5, a[0], false);
  } else {
    lc.setChar(0, 6, a[0], false);
    lc.setChar(0, 5, a[1], false);
  }
  lc.setChar(0, 4, ' ', false);
  if (h.length() == 1) {
    lc.setChar(0, 3, '0', false);
    lc.setChar(0, 2, h[0], true);
  } else {
    lc.setChar(0, 3, h[0], false);
    lc.setChar(0, 2, h[1], true);
  }

  if (m.length() == 1) {
    lc.setChar(0, 1, '0', false);
    lc.setChar(0, 0, m[0], false);
  } else {
    lc.setChar(0, 1, m[0], false);
    lc.setChar(0, 0, m[1], false);
  }
}

void printAlarm(int _h, int _m, bool _e) {

  Serial.print(_h);
  Serial.print(":");
  Serial.print(_m);
  if (_e) {
    Serial.println(" Enabled");
  } else {
    Serial.println(" Disabled");
  }
}

void printString(String s) {
  lc.clearDisplay(0);
  for (int i = 0; i < 8; i++) {
    if (i < s.length()) {
      lc.setChar(0, 7 - i, s[i], false);
    } else {
      lc.setChar(0, 7 - i, ' ', false);
    }
  }
}


void applySetting(String settingName, String settingValue, int action, int id) {
  if (action == 0) {
    Alarms_H[id] = settingValue.toInt();

    Serial.print(" Hour");
  }
  if (action == 1) {
    Alarms_M[id] = settingValue.toInt();

    Serial.print(" Minute");
  }

  if(settingName == "czas"){
    int temp  = settingValue.toInt();
    if(temp==1){
      zmianaczasu=true;
    }else{
      zmianaczasu=false;
    }
  }

  if (action == 2) {
    Alarms_E[id] = toBoolean(settingValue);

    Serial.print(" Enabled");
  }
}


boolean toBoolean(String settingValue) {
  if (settingValue.toInt() == 1) {
    return true;
  } else {
    return false;
  }
}
