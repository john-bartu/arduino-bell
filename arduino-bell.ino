//clock
#include <Wire.h>
#include "Sodaq_DS3231.h"

//WIFI
#include "WiFiEsp.h"
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
//#include "LedControl.h"
//LedControl lc = LedControl(11, 10, 9, 1);
SoftwareSerial Serial1(5, 6); // RX, TX
#endif


struct Alarm {
  int h;
  int m;
  bool e;
};

int ringPIN = 12;

Alarm ALARMS[20];

//WIFI Settings
char ssid[] = "john_bartu"; // your network SSID (name)
char pass[] = "mojetesty";  // your network password

int status = WL_IDLE_STATUS;
char server[] = "bartulajan.pl"; // website url
WiFiEspClient client;

//Clock
char weekDay[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };


void setup()
{



  pinMode(ringPIN, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(ringPIN, LOW);
digitalWrite(13, LOW);

  Serial.begin(9600);
  Serial1.begin(9600);
  WiFi.init(&Serial1);

  Wire.begin();
  rtc.begin();

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  printWifiStatus();

  Serial.println();
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial



  Serial.println("CLOCK UPDATE");
  UpdateClock();

  //  Serial.println("ALARMS UPDATE");
  //
  for (int i = 0; i < 20 ; i++) {
    UpdateAlarm(i);
    digitalWrite(13, LOW);
  }

  showAlarms();
}

void UpdateClock() {
  int years = 0;

  while (years < 2015) {
    if (client.connectSSL(server, 443)) {
      Serial.println("Connected to server");
      // Make a HTTP request
      client.println("GET /clock.php HTTP/1.1");
      client.println("Host: www.bartulajan.pl");
      client.println("Connection: close");
      client.println();
    }

    Serial.println("Getting httpcontent");
    String httpcontent = "";
    while (client.available()) {
      char c = client.read();
      httpcontent += c;
    }

    Serial.println("Searching for body");
    String bodycontent = "";
    bodycontent = httpcontent.substring(httpcontent.indexOf('@@@') + 3, httpcontent.indexOf('###'));

    Serial.println("Body found");
    Serial.println("Content:");
    Serial.println(bodycontent);
digitalWrite(13, HIGH);
    /////from body to time
    Serial.println("Parsing to time... ");
    int times[7];

    for (int i = 0; i < 7; i++) {
      times[i] = getValue(bodycontent, ':', i).toInt();
      Serial.println(getValue(bodycontent, ':', i).toInt());
    }

    years = times[0];

    setRTC(times);

    // if the server's disconnected, stop the client
    if (!client.connected()) {
      Serial.println("Disconnecting from server...");
      client.stop();

    }
  }
}

void UpdateAlarm(int id) {
  int thish = 0;
  while (thish < 7) {
    if (client.connectSSL(server, 443)) {
      Serial.println("Connected to server");
      // Make a HTTP request
      client.print("GET /alarm");
      client.print(id);
      client.println(".txt HTTP/1.1");
      client.println("Host: www.bartulajan.pl");
      client.println("Connection: close");
      client.println();
    }

    Serial.println("Getting httpcontent");
    String httpcontent = "";
    while (client.available()) {
      char c = client.read();
      httpcontent += c;
    }

    Serial.println("Searching for body");
    String bodycontent = "";
    bodycontent = httpcontent.substring(httpcontent.indexOf('@@@') + 3, httpcontent.indexOf('###'));

    Serial.println("Body found");
    Serial.println("Content:");
    Serial.println(bodycontent);

    /////from body to time
    Serial.println("Parsing to time... ");


    int h = getValue(bodycontent, ',', 0).toInt();
    thish = h;
    int m = getValue(bodycontent, ',', 1).toInt();
    String es = getValue(bodycontent, ',', 2);
    bool e;
    if (es == "1") e = true; else e = false;

    Alarm customVar = {
      h,
      m,
      e
    };
    ALARMS[id] = customVar;
    Serial.print("Alarm: ");
    Serial.print(customVar.h);
    Serial.print(":");
    Serial.print(customVar.m);
    Serial.print(" E:");
    Serial.println(customVar.e);


    // if the server's disconnected, stop the client
    if (!client.connected()) {
      Serial.println("Disconnecting from server...");
      client.stop();

    }

  }
}
int millis_delay = 10000;

unsigned long  millis_last;
unsigned long  millis_reset;
bool mode_day = false;
void loop()
{
  digitalWrite(13, HIGH);
  unsigned long  millis_now = millis();
  DateTime time_now = rtc.now();
  int h, m, s, dow;
  h = time_now.hour();
  m = time_now.minute();
  s = time_now.second();
  dow = time_now.dayOfWeek();

  for (int i = 0; i < 20; i++) {
    if (ALARMS[i].e) {
      if (dow != 0 || dow != 6) {
        if (h == ALARMS[i].h) {
          if (m == ALARMS[i].m) {
            if (s == 0 || s == 1 || s == 2 || s == 3 || s == 4) {
              //              PrintAlarm((String)i, (String)h, (String)m);
              ringBell();
            }
          }
        }
      }
    }
  }

  if (millis_now - millis_last > millis_delay) {
    if (mode_day) {
      mode_day = false;
      millis_last = millis_now;
    } else {
      mode_day = true;
      millis_last = millis_now;
    }




    Serial.print("Date: ");
    Serial.print(time_now.year(), DEC);
    Serial.print('/');
    Serial.print(time_now.month(), DEC);
    Serial.print('/');
    Serial.print(time_now.date(), DEC);

    Serial.print(" Time: ");
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
    //    PrintTime((String)h, (String)m, (String)s, (String)(dow + 1));
  } else {
    //    PrintData((String)dy, (String)mh, (String)yr);
  }
  //PrintAlarm("3","9","45");
  delay(100);
}



void ringBell() {
  Serial.println("BELL RANG..");
  digitalWrite(ringPIN, HIGH);
  delay(8000);
  digitalWrite(ringPIN, LOW);
  Serial.println("..END");
}

void showAlarms() {
  for (int i = 0; i < 20; i++) {
    Serial.print(" ALARM: ");
    Serial.print(ALARMS[i].h);
    Serial.print(" : ");
    Serial.print(ALARMS[i].m);
    Serial.print(" -E ");
    Serial.println(ALARMS[i].e);

  }

}



void setRTC(int times[]) {
  DateTime dt (times[0], times[1], times[2], times[3], times[4], times[5], times[6]);
  rtc.setDateTime(dt );

}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


//
//void PrintTime(String  h, String m, String s, String d) {
//  //lc.clearDisplay(0);
//  if (h.length() == 1) {
//    lc.setChar(0, 7, '0', false);
//    lc.setChar(0, 6, h[0], true);
//  } else {
//    lc.setChar(0, 7, h[0], false);
//    lc.setChar(0, 6, h[1], true);
//  }
//
//  if (m.length() == 1) {
//    lc.setChar(0, 5, '0', false);
//    lc.setChar(0, 4, m[0], true);
//  } else {
//    lc.setChar(0, 5, m[0], false);
//    lc.setChar(0, 4, m[1], true);
//  }
//
//  if (s.length() == 1) {
//    lc.setChar(0, 3, '0', false);
//    lc.setChar(0, 2, s[0], false);
//  } else {
//    lc.setChar(0, 3, s[0], false);
//    lc.setChar(0, 2, s[1], false);
//  }
//
//  lc.setChar(0, 1, ' ', false);
//  lc.setChar(0, 0, d[0], false);
//
//}
//void PrintData(String  d, String m, String y) {
//  //lc.clearDisplay(0);
//  if (d.length() == 1) {
//    lc.setChar(0, 7, '0', false);
//    lc.setChar(0, 6, d[0], true);
//  } else {
//    lc.setChar(0, 7, d[0], false);
//    lc.setChar(0, 6, d[1], true);
//  }
//
//
//  if (m.length() == 1) {
//    lc.setChar(0, 5, '0', false);
//    lc.setChar(0, 4, m[0], true);
//  } else {
//    lc.setChar(0, 5, m[0], false);
//    lc.setChar(0, 4, m[1], true);
//  }
//
//
//  lc.setChar(0, 3, y[0], false);
//  lc.setChar(0, 2, y[1], false);
//
//  lc.setChar(0, 1, y[2], false);
//  lc.setChar(0, 0, y[3], false);
//}
//void PrintAlarm(String a, String  h,  String m) {
//  //lc.clearDisplay(0);
//  lc.setChar(0, 7, 'A', false);
//
//  if (a.length() == 1) {
//    lc.setChar(0, 6, ' ', false);
//    lc.setChar(0, 5, a[0], false);
//  } else {
//    lc.setChar(0, 6, a[0], false);
//    lc.setChar(0, 5, a[1], false);
//  }
//  lc.setChar(0, 4, ' ', false);
//  if (h.length() == 1) {
//    lc.setChar(0, 3, '0', false);
//    lc.setChar(0, 2, h[0], true);
//  } else {
//    lc.setChar(0, 3, h[0], false);
//    lc.setChar(0, 2, h[1], true);
//  }
//
//  if (m.length() == 1) {
//    lc.setChar(0, 1, '0', false);
//    lc.setChar(0, 0, m[0], false);
//  } else {
//    lc.setChar(0, 1, m[0], false);
//    lc.setChar(0, 0, m[1], false);
//  }
//}
//
//void printAlarm(int _h, int _m, bool _e) {
//
//  Serial.print(_h);
//  Serial.print(":");
//  Serial.print(_m);
//  if (_e) {
//    Serial.println(" Enabled");
//  } else {
//    Serial.println(" Disabled");
//  }
//}
//
//void printString(String s) {
// lc.clearDisplay(0);
//  for (int i = 0; i < 8; i++) {
//    if (i < s.length()) {      lc.setChar(0, 7 - i, s[i], false);
//    } else {
//      lc.setChar(0, 7 - i, ' ', false);
//    }
//  }
//}
