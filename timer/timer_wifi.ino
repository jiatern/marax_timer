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

// Wi-Fi SSID credentials
const char* ssid = "SSID"; //type your ssid
const char* password = "PASSWORD"; //type your password

// set to true/false when using another type of reed sensor
bool reedOpenSensor = true;
bool displayOn = true;
bool refillWarning = false;
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
const unsigned char suimidfing [] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xff, 
	0xff, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0xfe, 
	0x00, 0x3f, 0x00, 0x7f, 0xff, 0x80, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0x22, 0x7f, 0xff, 0x80, 
	0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x7f, 0x3f, 0xff, 0x80, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 
	0x3f, 0x0f, 0xff, 0x80, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0x87, 0xff, 0x80, 0xff, 0xff, 
	0xc0, 0x00, 0x00, 0x00, 0x07, 0xe7, 0xff, 0x80, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x03, 0xe7, 
	0xff, 0x80, 0xff, 0xff, 0x80, 0x03, 0xff, 0xf0, 0x01, 0xcf, 0xff, 0x80, 0xff, 0xff, 0x80, 0x1f, 
	0xff, 0xfe, 0x00, 0x9f, 0xff, 0x80, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xff, 0x80, 0x3f, 0xff, 0x80, 
	0xff, 0xfc, 0x01, 0xff, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0x80, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xff, 
	0xf0, 0x1f, 0xff, 0x80, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0x80, 0xff, 0xc0, 
	0x1f, 0xff, 0x3f, 0xff, 0xfc, 0x07, 0xff, 0x80, 0xff, 0x80, 0x3f, 0xe1, 0x3f, 0x7f, 0xfc, 0x07, 
	0xff, 0x80, 0xff, 0x00, 0x7f, 0xc5, 0x9f, 0x3f, 0xfe, 0x07, 0xff, 0x80, 0xfe, 0x00, 0x7f, 0x8d, 
	0x9f, 0x1f, 0xff, 0x27, 0xff, 0x80, 0xfc, 0x10, 0xff, 0x1c, 0xc9, 0x0f, 0xff, 0x07, 0xff, 0x80, 
	0xf8, 0x21, 0xff, 0x3c, 0xc1, 0x0f, 0x7f, 0x83, 0xff, 0x80, 0xf8, 0x03, 0xfe, 0x7c, 0xe0, 0x27, 
	0x3f, 0x80, 0xff, 0x80, 0xf0, 0x03, 0xfc, 0xfc, 0xf0, 0x27, 0x37, 0xc2, 0x7f, 0x80, 0xf8, 0x07, 
	0xfc, 0xfe, 0xfc, 0x33, 0x93, 0xc6, 0x7f, 0x80, 0xfc, 0x07, 0xf9, 0xfe, 0x7f, 0x31, 0x93, 0xc7, 
	0x3f, 0x80, 0xf8, 0x0f, 0xf9, 0xfe, 0x7f, 0x79, 0xc3, 0xc7, 0x1f, 0x80, 0xf9, 0x8f, 0xf3, 0xff, 
	0x3e, 0x7c, 0x43, 0x8f, 0x0f, 0x80, 0xf3, 0x9f, 0xf3, 0xff, 0x1e, 0x7e, 0x23, 0x1f, 0xc7, 0x80, 
	0xf3, 0x9f, 0xe7, 0xff, 0x8c, 0xff, 0x12, 0x7f, 0xf3, 0x80, 0xf3, 0x9f, 0xe1, 0xfe, 0x46, 0xff, 
	0x10, 0x7f, 0xf3, 0x80, 0xf3, 0x3f, 0xe0, 0x00, 0x60, 0x00, 0x30, 0xff, 0xf9, 0x80, 0xf2, 0x3f, 
	0xcd, 0x01, 0xfb, 0x00, 0xb0, 0xff, 0xf9, 0x80, 0xf8, 0x3f, 0xcf, 0xff, 0xff, 0xff, 0xf1, 0xff, 
	0xf9, 0x80, 0xf8, 0x3f, 0xcf, 0xff, 0xff, 0xff, 0xf1, 0xff, 0xf9, 0x80, 0xfc, 0x3f, 0xcf, 0xff, 
	0xff, 0xff, 0xf0, 0xff, 0xf9, 0x80, 0xfc, 0x3f, 0x8f, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xf3, 0x80, 
	0xf8, 0x17, 0xcf, 0xff, 0xff, 0xff, 0xf0, 0x7f, 0xf3, 0x80, 0xf8, 0x13, 0xcf, 0xff, 0x01, 0xff, 
	0xe4, 0x3f, 0xc7, 0x80, 0xf8, 0x13, 0xcf, 0xff, 0x81, 0xff, 0xe4, 0x80, 0x0f, 0x80, 0xf8, 0x19, 
	0xcf, 0xff, 0xff, 0xff, 0xe4, 0xe0, 0x3f, 0x80, 0xfe, 0x19, 0xcf, 0xff, 0xff, 0xff, 0xe1, 0xfe, 
	0x7f, 0x80, 0xff, 0x0c, 0x07, 0xff, 0xff, 0xff, 0xe0, 0xfe, 0x7f, 0x80, 0xff, 0x0e, 0x07, 0xff, 
	0xff, 0xff, 0xc0, 0xfc, 0xff, 0x80, 0xfe, 0x7e, 0x61, 0xff, 0xff, 0xff, 0x86, 0x78, 0xff, 0x80, 
	0xfc, 0x7a, 0x48, 0xff, 0xff, 0xff, 0x0f, 0x19, 0xff, 0x80, 0xf9, 0xf0, 0x0c, 0x1f, 0xff, 0xf8, 
	0x0f, 0x83, 0xff, 0x80, 0xf3, 0x00, 0x1e, 0x01, 0xff, 0xc0, 0x0f, 0xe3, 0xff, 0x80, 0xf0, 0x00, 
	0x70, 0x00, 0x00, 0x00, 0x0f, 0xc3, 0xff, 0x80, 0xe0, 0x3e, 0x20, 0x00, 0x00, 0x20, 0x0f, 0x81, 
	0xff, 0x80, 0xf3, 0x1f, 0x00, 0x00, 0xc7, 0x00, 0x00, 0x01, 0xff, 0x80, 0xf3, 0x8f, 0xc0, 0x00, 
	0xc2, 0x00, 0x00, 0x01, 0xff, 0x80, 0xf8, 0xc7, 0x8f, 0x82, 0x00, 0x00, 0x00, 0xff, 0xff, 0x80, 
	0xfc, 0x27, 0x80, 0x41, 0x99, 0xc0, 0x03, 0xff, 0xff, 0x80, 0xff, 0x07, 0x80, 0x00, 0xc1, 0x80, 
	0x1f, 0xff, 0xff, 0x80, 0xff, 0x87, 0x8f, 0x80, 0x01, 0x03, 0xff, 0xff, 0xff, 0x80, 0xff, 0x0f, 
	0x9f, 0x80, 0x00, 0x01, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0x98, 0x00, 0x00, 0x01, 0xff, 0xff, 
	0xff, 0x80, 0xff, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xc7, 0x00, 
	0x00, 0x01, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0x8f, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80
};

const byte numChars = 32;
char receivedChars[numChars];
static byte ndx = 0;
char endMarker = '\n';
char rc;
WiFiServer server(80);

void setup() {
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

  // Connect to Wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.persistent(false);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G); // some routers need this to prevent DHCP issues
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  t.update();
  detectChanges();
  getMachineInput();
  wifiAdmin();
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
    // Serial.println("Request serial update"); 
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
	    delay(200);
	    if (timerCount > 15) {
		    shotCount++;
		    if (shotCount == 9) {
			  shotCount = 0;
        refillWarning = true;
        }
		  EEPROM.begin(EEPROM_SIZE);
		  EEPROM.put(eepromAddr, shotCount);
		  EEPROM.commit();
		  EEPROM.end();
	   }
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
      display.setCursor(display.width() / 2 - 1 + 17, 24);
      display.print(getTimer());
	  if (refillWarning) {			// display shotCount & refill water warning
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
        } else if (String(receivedChars[0]) == "+") { // change to "V" for V1 Gicar
          display.print("+"); // change to "S" for V1 Gicar
        } else {
          display.print("X");
        }
      } else {
		    display.drawBitmap(0, 0, suimidfing, 73, 64, SSD1306_WHITE);
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

void wifiAdmin() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Run actions if option is clicked
  if (request.indexOf("/RESET") != -1) {
  shotCount = 0;
  } 
  if (request.indexOf("/ADD") != -1){
  shotCount++;
  }
  if (request.indexOf("/WRITE") != -1){
  EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(eepromAddr, shotCount);
	EEPROM.commit();
  } 
  
  //Build HTML
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<style>");
  client.println("* {");
  client.println("font-size: 22px;");
  client.println("font-family: Helvetica !important;");
  client.println("text-align: center;");
  client.println("}");
  client.println("</style>");
  client.println("MaraX Monitor<br><br>");
  client.println("HX Temperature: ");
  client.print(receivedChars[14]);
  client.print(receivedChars[15]);
  client.print(receivedChars[16]);
  client.print(" C<br>");
  client.println("Steam Temperature: ");
  client.print(receivedChars[6]);
  client.print(receivedChars[7]);
  client.print(receivedChars[8]);
  client.print(" C<br>");
  client.print("<p style=\"font-size:34px !important\">Shot Count is now: ");
  client.print(shotCount);
  client.print("</p>");
  client.println("Click <a href=\"/RESET\">here</a> to reset the Shot Count to 0.<br>");
  client.println("Click <a href=\"/ADD\">here</a> to add Shot Count by 1.<br><br>");
  client.println("Click <a href=\"/WRITE\">here</a> to confirm writing to EEPROM.<br>");
  client.println("</html>");
}
