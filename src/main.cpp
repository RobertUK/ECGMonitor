// ************************************************************ //
// *  This source code is provided by the author "AS IS",       * //
// *  without warranty of any kind, either express or implied.  * //
// ************************************************************ //
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char *ssid = "X22";
const char *password = "@ssh000le123";

AsyncWebServer server(80);

#include "SPI.h"
#include "TFT_eSPI.h"
#include "RTClib.h"
//#include <FS.h>

//#include <SD.h>
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
//#include "Adafruit_Keypad.h"
#include "CuteBuzzerSounds.h"
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
//#include "esp_log.h"
//#include "esp32-hal-log.h"
#include <stdio.h>
#include <time.h>
//#include <JPEGDecoder.h>
#include "TMP36.h"
/*
 S_CONNECTION   S_DISCONNECTION S_BUTTON_PUSHED
 S_MODE1        S_MODE2         S_MODE3
 S_SURPRISE     S_OHOOH         S_OHOOH2
 S_CUDDLY       S_SLEEPING      S_HAPPY
 S_SUPER_HAPPY  S_HAPPY_SHORT   S_SAD
 S_CONFUSED     S_FART1         S_FART2
 S_FART3        S_JUMP 20

 */
#define TMP_SENSOR 12
#define LED_BUILTIN 2
#define ECG_INPUT 34
#define ECG_PLUS 32
#define ECG_MINUS 33
#define PULSE_BLINK 26
#define BUZZER 27
//#define BUTTON 13
//#define NO_LOG

String fileName = "Roboto-Regular-12";
String fileName1 = "Roboto-Regular-20";
String fileName2 = "Roboto-Regular-30";
String fileName3 = "Roboto-Regular-40";

const byte ROWS = 4; // rows
const byte COLS = 3; // columns
//define the symbols on the buttons of the keypads

bool processingFormat = false;

const int OUTPUT_TYPE = SERIAL_PLOTTER;
// Pin 13 is the on-board LED
//const int PULSE_FADE = 33;
const int THRESHOLD = 3000; // Adjust this number to avoid noise when idle
DateTime lastDateLogged = DateTime(1900, 1, 1);
unsigned long lastMillis;
PulseSensorPlayground pulseSensor;
const int LoopTime = 3000;
//const int LoopTime = 10000;

unsigned long LoopTimer = 0;
int BPM = 0;
int counter = 0;
int sensorValue = 0;
double x11, y11, x22, y22;
const double spacing = 1.1;
//float celsius;
//float fahrenheit;
// Set a debugging flag

// Show data on the TFT flag
bool ShowTraceOnTFT = true;

bool InsertZero = false;

//flag to invoke loggin to OpenLog
bool LogData = false;
bool leadsWereOff;

TFT_eSPI tft = TFT_eSPI();
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char buf[50]; /* buffer to hold the text of the date - ready to print */
char day[4];  /* buffer used to hold the string value of the day. Eg "Tue" */
int x;        /* used in for loop */
int y;

int val = 0;
int oldVal_TFT = 0;
int val_TFT = 0;
bool wiFi = true;;
File root;
File dataFile;

// JPEG decoder library

byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 5;
TMP36 tmp36(TMP_SENSOR);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS& fs, const char* path, const char* message) {
	//Serial.printf("Appending to file: %s\n", path);
	//Serial.printf("Appending: %s\n", message);
	File file = fs.open(path, FILE_WRITE);
	if (!file) {
		//Serial.println("Failed to open file for appending");
		exit;
	}
	if (file.print(message)) {
		//Serial.println("Message appended");
	}
	else {
		Serial.println("Append failed");
	}
	file.close();
}

uint16_t read16(fs::File& f) {
	uint16_t result;
	((uint8_t*)&result)[0] = f.read(); // LSB
	((uint8_t*)&result)[1] = f.read(); // MSB
	return result;
}


uint32_t read32(fs::File& f) {
	uint32_t result;
	((uint8_t*)&result)[0] = f.read(); // LSB
	((uint8_t*)&result)[1] = f.read();
	((uint8_t*)&result)[2] = f.read();
	((uint8_t*)&result)[3] = f.read(); // MSB
	return result;
}


// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS& fs, const char* path, const char* message) {
	Serial.printf("Writing file: %s\n", path);

	File file = fs.open(path, FILE_APPEND);
	if (!file) {
		Serial.println("Failed to open file for writing");
		return;
	}
	if (file.print(message)) {
		Serial.println("File written");
	}
	else {
		Serial.println("Write failed");
	}
	file.close();
}
*/

void drawGrid(void)
{

  int h = tft.height();
  int w = tft.width();

  Serial.println(h);
  tft.fillRect(1, 65, w - 1, h - 80, ILI9341_BLUE);

  //horizon
  for (int16_t x = 65; x < h; x += 40)
  {
    tft.fillRect(1, x, w - 1, 1, ILI9341_RED);
  }

  //vert
  for (int16_t x = 1; x < w; x += 40)
  {
    tft.fillRect(x, 65, 1, h - 80, ILI9341_RED);
  }

  //v end
  tft.fillRect(w - 1, 65, 1, h - 80, ILI9341_RED);
  //h end
  //tft.fillRect(1, 220, w-1, 1, ILI9341_RED);

  //footer
  //tft.fillRect(1, 1, w- 1, 1, ILI9341_MAGENTA);
  tft.fillRect(1, h - 10, w - 1, h - 1, ILI9341_BLACK);

  tft.setCursor(0, h - 10);
  tft.setTextSize(1.5);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.print(" Alexa prompted me to ask for \"Lisinopril\"?");
}

void setupScreen()
{
  bool buttonPressed = false;
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(2);
  tft.setCursor(tft.width() / 2 - 45, 1);
  tft.println("ECG Monitor");
  tft.setTextColor(ILI9341_RED);

  tft.drawLine(0, 35, tft.width(), 35, ILI9341_RED);
  //tft.drawLine(5, 70, tft.width() - 1, 70, RED);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);

  tft.setCursor(0, 230);
  tft.setTextSize(1.5);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.print("Press button to continue...");
  /*
	while (!buttonPressed)
	{
		if (digitalRead(BUTTON) == HIGH)
		{
			buttonPressed = true;
		}
	}
*/
  tft.setTextSize(2);

  tft.setCursor(1, 43);
  tft.print("PULSE ");

  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);

  tft.print("n/a");
  //tft.print(beatspermin);

  drawGrid();
}

void setup()
{

  Serial.begin(115200);

  Serial.println("");

  pinMode(13, INPUT);
  pinMode(32, INPUT);
  pinMode(33, INPUT);
  pinMode(35, INPUT);
  //digitalWrite(SD_CS, HIGH); // Touch controller chip select (if used)
  digitalWrite(TFT_CS, HIGH); // TFT screen chip select
  //digitalWrite(5, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)

  esp_log_level_set("*", ESP_LOG_VERBOSE);
  ESP_LOGE("TAG", "Error");
  ESP_LOGW("TAG", "Warning");
  // ESP_LOGI("TAG", "Info");
  ESP_LOGD("TAG", "Debug");
  ESP_LOGV("TAG", "Verbose");
  log_w("no token received");
  delay(50);
  //	if (!rtc.begin()) {
  //	Serial.println("Couldn't find RTC");
  //	while (1);
  //}
  //	else
  //	{
  //	Serial.print("RTC found.");
  //	}

  cute.init(BUZZER);

  cute.play(S_BEEP);
  for (size_t i = 0; 50 < 2; i++)
  {
    if (digitalRead(35) == HIGH)
    {
      wiFi = true;
       cute.play(S_SURPRISE);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      // Wait for connection
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Hi! I am ESP32.");
      });

      AsyncElegantOTA.begin(&server); // Start ElegantOTA
      server.begin();
      Serial.println("HTTP server started");
      break;
    }
    delay(100);
    if (i>=45)
    {
      wiFi = false;
    }

  }

  cute.play(S_HAPPY);

  pinMode(ECG_MINUS, INPUT);
  pinMode(ECG_PLUS, INPUT);

  tft.begin();

  tft.fillScreen(ILI9341_BLUE);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(tft.width() / 2 - 45, 1);
  Serial.print("configuring:...");
  tft.setTextColor(ILI9341_RED);

  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running, setting clock to compile time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    delay(600);
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  else
  {
    Serial.print("RTC Running. Time: ");
    Serial.println(rtc.now().timestamp());
  }

  float celsius = tmp36.getTempC();

  //create a variable and store the current temperature in
  //fahrenheit in it using the getTempF function
  float fahrenheit = tmp36.getTempF();

  float volts = tmp36.getVoltage();

  //Print the data to the Serial monitor
  Serial.print("Celsius: ");
  Serial.print(celsius);
  Serial.print(" Fahrenheit: ");
  Serial.println(fahrenheit);
  delay(500);

  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLUE);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(1, 1);
  tft.println("Initialising...");
  tft.println();
  tft.println("Temperature:");
  tft.print(celsius);
  tft.println("c");
  tft.println();
  tft.print(fahrenheit);
  tft.println("f");
  tft.println();
  tft.print("tmp volts:");
  tft.println(volts);
  tft.println();
  tft.print("Time: ");
  tft.println(rtc.now().timestamp());
  delay(3000);
  pulseSensor.analogInput(ECG_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  //pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin())
  {
    /*
		   PulseSensor initialization failed,
		   likely because our particular Arduino platform interrupts
		   aren't supported yet.

		   If your Sketch hangs here, try PulseSensor_BPM_Alternative.ino,
		   which doesn't use interrupts.
		*/
    for (;;)
    {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      digitalWrite(LED_BUILTIN, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
    }
  }

  /*rotation=0 to 4 to orient the screen*/
  uint8_t rotation = 1;
  tft.setRotation(rotation);
  //testText();
  //setupScreen();

  // set up for plotting values
  x = 0;
  y = 0;
  //===============================================
  //INTERRUPT TIMER SETUP - BEGIN
/*
	for (uint16_t i = 0; i < 3; i++)
	{
		if (!SD.begin(SD_CS)) {
			if (i == 19) {
				Serial.println("Card Mount Failed. End of retry.");
				exit;
			}
			Serial.print("Card Mount Failed. Retry: ");
			Serial.println(i);
			delay(100);
			//loop();
		}
		else {
			exit;
		}
	}

	uint8_t cardType = SD.cardType();

	if (cardType == CARD_NONE) {
		Serial.println("No SD card attached");
		exit;
	}

	Serial.print("SD Card Type: ");
	if (cardType == CARD_MMC) {
		Serial.println("MMC");
	}
	else if (cardType == CARD_SD) {
		Serial.println("SDSC");
	}
	else if (cardType == CARD_SDHC) {
		Serial.println("SDHC");
	}
	else {
		Serial.println("UNKNOWN");
	}

	uint64_t cardSize = SD.cardSize() / (1024 * 1024);
	Serial.printf("SD Card Size: %lluMB\n", cardSize);

	Serial.println("initialisation done.");

	root = SD.open("/");

	printDirectory(root, 0);

	////Serial.println("Initialising SPIFFs...");
	////if (!SPIFFS.begin()) {
	////	Serial.println("SPIFFS initialisation failed!");
	////	//while (1) yield(); // Stay here twiddling thumbs waiting
	////}
	////else
	////{
	////	Serial.println("\r\nInitialisation done.");
	////}

	root.close();
  #*/
#ifndef NO_LOG
  //writeFile(SD, "/ecg.csv", "time,value,bpm,temp");
#endif // !NO_LOG

  Serial.print("Temp Celcius: ");
  Serial.println(celsius);

  int x = (tft.width() - 300) / 2 - 1;
  int y = (tft.height() - 300) / 2 - 1;

  tft.setRotation(1); // landscape
  tft.fillScreen(random(0xFFFF));
  /*
	drawSdJpeg("/nhs.jpg", 0, 0);     // This draws a jpeg pulled off the SD Card
	delay(2000);

	tft.setRotation(1);  // landscape
	tft.fillScreen(random(0xFFFF));
	drawSdJpeg("/Mouse480.jpg", 0, 0);     // This draws a jpeg pulled off the SD Card
	delay(500);

	tft.setRotation(1);  // landscape
	tft.fillScreen(random(0xFFFF));
	drawSdJpeg("/nhs.jpg", 0, 0);     // This draws a jpeg pulled off the SD Card
	delay(900);
*/
  tft.setRotation(1); // landscape
  setupScreen();
}

void SendValueToTFT1(int value)
//Output the current analogue value to the TFT display
{

  int h = tft.height();
  int w = tft.width();
  float celcius = tmp36.getTempC();

  //sensorValue = map(sensorValue, 0, 4095, 0, 200);
  if (micros() > LoopTimer)
  {

    LoopTimer += LoopTime;
    //sensorValue = analogRead(value);
    int value1 = map(value, 0, 4095, 0, 1600);

    value1 = value1 >> 3;
    if (counter > w / spacing)
    {
      counter = 0;
      //tft.fillScreen(BLACK);

      drawGrid();
      tft.setTextSize(2);
      tft.setCursor(65, 43);
      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
      tft.print(BPM);
      tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
      tft.print(" BPM ");

      tft.setCursor(120, 43);
      tft.setTextColor(TFT_DARKGREY, ILI9341_BLACK);
      tft.print("Temp:");
      tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
      tft.printf("%.2f", celcius);
      delay(200);
    }
    if (counter == 0)
    {
      x11 = 0;
      y11 = h - value1;
    }

    if (counter > 0)
    {
      x22 = counter * spacing;
      // Change origin to left, bottom
      y22 = tft.height() - value1 - 15;
      //Serial.println(y22);
      //val_TFT = map(y22, 10, 200, 10, 200);
      if (y22 > h - 65)
      {
        y22 = h - 65;
      }
      //Serial.println(y22);
      tft.drawLine(x11, y11 + 50, x22, y22 + 50, ILI9341_WHITE);
      x11 = x22;
      y11 = y22;
    }
    counter++;
    delay(2);
  }
}

void loop()
{
  if ((digitalRead(ECG_PLUS) == 1) || (digitalRead(ECG_MINUS) == 1))
  {
    leadsWereOff = true;
    Serial.println("Leads off detect!");
    tft.setTextSize(4);
    tft.setCursor(10, tft.height() / 2);
    tft.setTextColor(ILI9341_BLUE, ILI9341_BLACK);
    tft.print("Check leads!");
    cute.play(S_SAD);
    delay(1000);
    return;
  }
  else
  {
    if (leadsWereOff)
    {
      leadsWereOff = false;
      cute.play(S_OHOOH);
      drawGrid();
    }
  }
  val = analogRead(ECG_INPUT); // read the input pin

  if (pulseSensor.sawNewSample())
  {
    /*
		   Every so often, send the latest Sample.
		   We don't print every sample, because our baud rate
		   won't support that much I/O.
		*/
    if (--samplesUntilReport == (byte)0)
    {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

      /* outputs GSR readings along with pulse readings in the same line, separated by commas */
      pulseSensor.outputSample();
      SendValueToTFT1(val);
#ifndef NO_LOG
      //	char csvCharArr[100];
      //	csvRow(csvCharArr, val);
      //	appendFile(SD, "/ecg.csv", csvCharArr);
#endif

      if (pulseSensor.sawStartOfBeat())
      {
        cute.play(S_BEEP);
        BPM = pulseSensor.getBeatsPerMinute();
      }
    }
    //if (ShowTraceOnTFT == true) {

    //delay(5);
    //}
    //Serial.println("l");
  }
  if(wiFi)
  {
  AsyncElegantOTA.loop();
  }
}
void printDirectory(File dir, int numTabs)
{
  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory())
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
/*
void drawBmp(const char* filename, int16_t x, int16_t y) {

	if ((x >= tft.width()) || (y >= tft.height())) return;

	File bmpFS;

	// Open requested file on SD card
	bmpFS = SD.open(filename);

	if (!bmpFS)
	{
		Serial.print("File not found");
		return;
	}

	uint32_t seekOffset;
	uint16_t w, h, row, col;
	uint8_t  r, g, b;

	uint32_t startTime = millis();

	if (read16(bmpFS) == 0x4D42)
	{
		read32(bmpFS);
		read32(bmpFS);
		seekOffset = read32(bmpFS);
		read32(bmpFS);
		w = read32(bmpFS);
		h = read32(bmpFS);

		if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
		{
			y += h - 1;

			tft.setSwapBytes(true);
			bmpFS.seek(seekOffset);

			uint16_t padding = (4 - ((w * 3) & 3)) & 3;
			uint8_t lineBuffer[w * 3];

			for (row = 0; row < h; row++) {
				bmpFS.read(lineBuffer, sizeof(lineBuffer));
				uint8_t* bptr = lineBuffer;
				uint16_t* tptr = (uint16_t*)lineBuffer;
				// Convert 24 to 16 bit colours
				for (uint16_t col = 0; col < w; col++)
				{
					b = *bptr++;
					g = *bptr++;
					r = *bptr++;
					*tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
				}
				// Read any line padding
				if (padding) bmpFS.read((uint8_t*)tptr, padding);
				// Push the pixel row to screen, pushImage will crop the line if needed
				tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
			}
			Serial.print("Loaded in "); Serial.print(millis() - startTime);
			Serial.println(" ms");
		}
		else Serial.println("BMP format not recognized.");
	}
	bmpFS.close();
}

void csvRow(char* csvCharArr, unsigned int value)
{
	float celsius;
	char csvRowData[99] = "\0";
	//Serial.printf("csvRowData1: %s\n", csvRowData);
	DateTime now = rtc.now();
	unsigned long currentMillis = 0;
	if (lastDateLogged == now)
	{
		currentMillis = millis() - lastMillis;
		//lastMillis = currentMillis;
	}
	else
	{
		lastMillis = millis();
		currentMillis = 0;
	}

	lastDateLogged = now;

	//Serial.println(val);

	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	day = now.day();
	month = now.month();
	year = now.year();// +2000;
	hour = now.hour();
	minute = now.minute();
	second = now.second();
	char charVal[5];
	char charMillis[10];
	char charNow[40];
	char charTmp[7];
	//harNow = now.timestamp().c_str();
	char charBPM[5];
	//char csvRow[100];
	time_t t = now.unixtime();
	struct tm* lt = localtime(&t);

	sprintf(charVal, "%d", val);
	sprintf(charBPM, "%d", BPM);
	sprintf(charMillis, "%ul", currentMillis);
	sprintf(charTmp, "%.2f", celsius);
	strftime(charNow, 50, "%d %b %Y %H:%H:%M:%S", lt);
	strcat(csvRowData, charNow);
	strcat(csvRowData, ":");
	strcat(csvRowData, charMillis);

	strcat(csvRowData, ",");
	strcat(csvRowData, charVal);
	strcat(csvRowData, ",");
	strcat(csvRowData, charBPM);
	strcat(csvRowData, ",");
	strcat(csvRowData, charTmp);
	strcat(csvRowData, "\n");
	//Serial.print("csvRowData:");
	//Serial.print(csvRowData);
	strcpy(csvCharArr, csvRowData);
}




//####################################################################################################
// Draw a JPEG on the TFT pulled from SD Card
//####################################################################################################
// xpos, ypos is top left corner of plotted image
void drawSdJpeg(const char* filename, int xpos, int ypos) {

	// Open the named file (the Jpeg decoder library will close it)
	File jpegFile = SD.open(filename, FILE_READ);  // or, file handle reference for SD library
	//File jpegFile = SPIFFS.open(filename, FILE_READ);  // or, file handle reference for SD library
	if (!jpegFile) {
		Serial.print("ERROR: File \""); Serial.print(filename); Serial.println("\" not found!");
		jpegFile.close();
		return;
	}

	Serial.println("===========================");
	Serial.print("Drawing file: "); Serial.println(filename);
	Serial.println("===========================");

	// Use one of the following methods to initialise the decoder:
	boolean decoded = JpegDec.decodeSdFile(jpegFile);  // Pass the SD file handle to the decoder,
	//boolean decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)

	if (decoded) {
		// print information about the image to the serial port
		jpegInfo();
		// render the image onto the screen at given coordinates
		jpegRender(xpos, ypos);
	}
	else {
		Serial.println("Jpeg file format not supported!");
	}
	jpegFile.close();
}

void showTime(uint32_t msTime) {
	tft.setCursor(0, 0);
	tft.setTextFont(1);
	tft.setTextSize(1);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.print(F(" JPEG drawn in "));
	tft.print(msTime);
	tft.println(F(" ms "));
	Serial.print(F(" JPEG drawn in "));
	Serial.print(msTime);
	Serial.println(F(" ms "));
}
//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void jpegRender(int xpos, int ypos) {

	//jpegInfo(); // Print information from the JPEG file (could comment this line out)

	uint16_t* pImg;
	uint16_t mcu_w = JpegDec.MCUWidth;
	uint16_t mcu_h = JpegDec.MCUHeight;
	uint32_t max_x = JpegDec.width;
	uint32_t max_y = JpegDec.height;

	bool swapBytes = tft.getSwapBytes();
	tft.setSwapBytes(true);

	// Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
	// Typically these MCUs are 16x16 pixel blocks
	// Determine the width and height of the right and bottom edge image blocks
	uint32_t min_w = min(mcu_w, max_x % mcu_w);
	uint32_t min_h = min(mcu_h, max_y % mcu_h);

	// save the current image block size
	uint32_t win_w = mcu_w;
	uint32_t win_h = mcu_h;

	// record the current time so we can measure how long it takes to draw an image
	uint32_t drawTime = millis();

	// save the coordinate of the right and bottom edges to assist image cropping
	// to the screen size
	max_x += xpos;
	max_y += ypos;

	// Fetch data from the file, decode and display
	while (JpegDec.read()) {    // While there is more data in the file
		pImg = JpegDec.pImage;   // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

		// Calculate coordinates of top left corner of current MCU
		int mcu_x = JpegDec.MCUx * mcu_w + xpos;
		int mcu_y = JpegDec.MCUy * mcu_h + ypos;

		// check if the image block size needs to be changed for the right edge
		if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
		else win_w = min_w;

		// check if the image block size needs to be changed for the bottom edge
		if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
		else win_h = min_h;

		// copy pixels into a contiguous block
		if (win_w != mcu_w)
		{
			uint16_t* cImg;
			int p = 0;
			cImg = pImg + win_w;
			for (int h = 1; h < win_h; h++)
			{
				p += mcu_w;
				for (int w = 0; w < win_w; w++)
				{
					*cImg = *(pImg + w + p);
					cImg++;
				}
			}
		}

		// calculate how many pixels must be drawn
		uint32_t mcu_pixels = win_w * win_h;

		// draw image MCU block only if it will fit on the screen
		if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
			tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
		else if ((mcu_y + win_h) >= tft.height())
			JpegDec.abort(); // Image has run off bottom of screen so abort decoding
	}

	tft.setSwapBytes(swapBytes);

	showTime(millis() - drawTime); // These lines are for sketch testing only
}

//####################################################################################################
// Print image information to the serial port (optional)
//####################################################################################################
// JpegDec.decodeFile(...) or JpegDec.decodeArray(...) must be called before this info is available!
void jpegInfo() {

	// Print information extracted from the JPEG file
	Serial.println("JPEG image info");
	Serial.println("===============");
	Serial.print("Width      :");
	Serial.println(JpegDec.width);
	Serial.print("Height     :");
	Serial.println(JpegDec.height);
	Serial.print("Components :");
	Serial.println(JpegDec.comps);
	Serial.print("MCU / row  :");
	Serial.println(JpegDec.MCUSPerRow);
	Serial.print("MCU / col  :");
	Serial.println(JpegDec.MCUSPerCol);
	Serial.print("Scan type  :");
	Serial.println(JpegDec.scanType);
	Serial.print("MCU width  :");
	Serial.println(JpegDec.MCUWidth);
	Serial.print("MCU height :");
	Serial.println(JpegDec.MCUHeight);
	Serial.println("===============");
	Serial.println("");
}
*/
