#define D5 (14)
#define D6 (12)
#define D7 (13)

#define PUMP_PIN D7
#define EEPROM_SIZE 12

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Timer.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
SoftwareSerial mySerial(D5, D6);
Timer t;

// set to true/false when using another type of reed sensor
bool reedOpenSensor = true;
bool displayOn = true;
bool flashPressed = false; // flash button pressed status is false at boot
int timerCount = 0;
int prevTimerCount = 0;
bool timerStarted = false;
long timerStartMillis = 0;
long timerStopMillis = 0;
long timerDisplayOffMillis = 0;
long serialUpdateMillis = 0;
int pumpInValue = 0;
int eepromAddr = 0; // starting address of EEPROM
int shotCount; // declare shotCount
int savedshotCount; // declare shotCount to be written into EEPROM

const byte numChars = 32;
char receivedChars[numChars];
static byte ndx = 0;
char endMarker = '\n';
char rc;

void setup() {
  WiFi.mode(WIFI_OFF);

  pinMode(0, INPUT_PULLUP);
  Serial.begin(9600);
  mySerial.begin(9600);
  EEPROM.begin(EEPROM_SIZE); 		// start EEPROM
  
  EEPROM.get(eepromAddr, shotCount);	// read shotCount data from EEPROM
  Serial.print("Current Shout Count = ");
  Serial.println(shotCount);
  EEPROM.end();
  
  pinMode(PUMP_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  t.every(100, updateDisplay);

  memset(receivedChars, 0, numChars );

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();
  mySerial.write(0x11);
}

void loop() {
  t.update();
  detectChanges();
  getMachineInput();
  if (digitalRead(0) == LOW) {	// detect flash button pressing
    Serial.println("Flash button is pressed!");
    flashPressed = true;
  }
  delay(180); // add some delays to allow buttons to depress
  if (flashPressed == true) {	// reset shotCount to zero and write to EEPROM
    shotCount = 0;
    savedshotCount = shotCount;
    EEPROM.put(eepromAddr, savedshotCount);
	  EEPROM.commit();
    flashPressed = false;
  }
}

void getMachineInput() {
  while (mySerial.available() ) {
    serialUpdateMillis = millis();
    rc = mySerial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0';
      ndx = 0;
      Serial.println(receivedChars);
    }
  }

  if (millis() - serialUpdateMillis > 5000) {
    serialUpdateMillis = millis();
    memset(receivedChars, 0, numChars);
    Serial.println("Request serial update");
    mySerial.write(0x11);
  }
}

void detectChanges() {
  digitalWrite(LED_BUILTIN, digitalRead(PUMP_PIN));
  if(reedOpenSensor) {
    pumpInValue = digitalRead(PUMP_PIN);
  } else {
    pumpInValue = !digitalRead(PUMP_PIN);
  }
  if (!timerStarted && !pumpInValue) {
    timerStartMillis = millis();
    timerStarted = true;
    displayOn = true;
    Serial.println("Start pump");
  }
  if (timerStarted && pumpInValue) {
    if (timerStopMillis == 0) {
      timerStopMillis = millis();
    }
    if (millis() - timerStopMillis > 500) {
      timerStarted = false;
      timerStopMillis = 0;
      timerDisplayOffMillis = millis();
      display.invertDisplay(false);
      Serial.println("Stop pump");
    }
  } else {
    timerStopMillis = 0;
  }
  if (!timerStarted && displayOn && timerDisplayOffMillis >= 0 && (millis() - timerDisplayOffMillis > 1000 * 60 * 60)) {
    timerDisplayOffMillis = 0;
    timerCount = 0;
    prevTimerCount = 0;
    displayOn = false;
    Serial.println("Sleep");
  }
}

String getTimer() {
  char outMin[2];
  if (timerStarted) {
    timerCount = (millis() - timerStartMillis ) / 1000;
    if (timerCount > 15) {
      prevTimerCount = timerCount;
      shotCount++;
      if (shotCount == 9) {
        savedshotCount = 0;
      } else {
          savedshotCount = shotCount;
        }
	    EEPROM.put(eepromAddr, savedshotCount);
	    EEPROM.commit();
    }
  } else {
    timerCount = prevTimerCount;
  }
  if (timerCount > 99) {
    return "99";
  }
  sprintf( outMin, "%02u", timerCount);
  return outMin;
}

void updateDisplay() {
  display.clearDisplay();
  if (displayOn) {
    if (timerStarted) {
      display.setTextSize(7);
      display.setCursor(25, 8);
      display.print(getTimer());
    } else {
      // draw line
      display.drawLine(74, 0, 74, 63, SSD1306_WHITE);
      // draw time seconds
      display.setTextSize(4);
      display.setCursor(display.width() / 2 - 1 + 17, 20);
      display.print(getTimer());
	  if (shotCount == 9) {			// display shotCount & refill water warning
          display.setTextSize(1);
          display.setCursor(86,1);
          display.print("Refill");
          display.setTextSize(1);
          display.setCursor(86,10);
          display.print("water!");
        } else {
          display.setTextSize(2);
          display.setCursor(82,1);
          display.print(shotCount);
          display.setTextSize(1);
          display.setCursor(97,8);
          display.print("shots");
          }
      // draw machine state C/S
      if (receivedChars[0] ) {
        display.setTextSize(2);
        display.setCursor(1, 1);
        if (String(receivedChars[0]) == "C") {
          display.print("C");
        } else if (String(receivedChars[0]) == "V") {
          display.print("S");
        } else {
          display.print("X");
        }
      }
      if (String(receivedChars).substring(18, 22) == "0000") {
        // not in boost heating mode
        // draw fill circle if heating on
        if (String(receivedChars[23]) == "1") {
          display.fillCircle(45, 7, 6, SSD1306_WHITE);
        }
        // draw empty circle if heating off
        if (String(receivedChars[23]) == "0") {
          display.drawCircle(45, 7, 6, SSD1306_WHITE);
        }
      } else {
        // in boost heating mode
        // draw fill rectangle if heating on
        if (String(receivedChars[23]) == "1") {
          display.fillRect(39, 1, 12, 12, SSD1306_WHITE);
        }
        // draw empty rectangle if heating off
        if (String(receivedChars[23]) == "0") {
          display.drawRect(39, 1, 12, 12, SSD1306_WHITE);
        }
      }
      // draw temperature
      if (receivedChars[14] && receivedChars[15] && receivedChars[16]) {
        display.setTextSize(3);
        display.setCursor(1, 20);
        if (String(receivedChars[14]) != "0") {
          display.print(String(receivedChars[14]));
        }
        display.print(String(receivedChars[15]));
        display.print(String(receivedChars[16]));
        display.print((char)247);
        if (String(receivedChars[14]) == "0") {
          display.print("C");
        }
      }
      // draw steam temperature
      if (receivedChars[6] && receivedChars[7] && receivedChars[8]) {
        display.setTextSize(2);
        display.setCursor(1, 48);
        if (String(receivedChars[6]) != "0") {
          display.print(String(receivedChars[6]));
        }
        display.print(String(receivedChars[7]));
        display.print(String(receivedChars[8]));
        display.print((char)247);
        display.print("C");
      }
    }
  }
  display.display();
}
