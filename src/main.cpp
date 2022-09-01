// ************************************************************ //
// *  This source code is provided by the author "AS IS",       * //
// *  without warranty of any kind, either express or implied.  * //
// ************************************************************ //
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
//#include "Adafruit_Keypad.h
#include "CuteBuzzerSounds.h"
const char *ssid = "";
const char *password = "";

AsyncWebServer server(80);

#include "SPI.h"
#include "TFT_eSPI.h"
#include "RTClib.h"
#include "JPEG_functions.h"

#include <FS.h>

//#include <SD.h>
//#define USE_ARDUINO_INTERRUPTS false

//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
//#include "esp_log.h"
//#include "esp32-hal-log.h"
#include <stdio.h>
#include <time.h>
#include <JPEGDecoder.h>
#include "TMP36.h"
#include "JPEGDecoder.h"

#define TMP_SENSOR 12
#define LED_BUILTIN 2
#define ECG_INPUT A0
#define ECG_PLUS 32
#define ECG_MINUS 26
#define PULSE_BLINK 39
#define BUZZER 21
#define BACKLIGHT 15
//#define BUTTON 13
//#define NO_LOG

#define AA_FONT_SMALL "Roboto-Regular-12"
#define AA_FONT_MEDIUM "Roboto-Regular-20"
#define AA_FONT_LARGE "NotoSansBold36"

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
int x;		  /* used in for loop */
int y;

int val = 0;
int oldVal_TFT = 0;
int val_TFT = 0;
bool wiFi = true;
;
File root;
File dataFile;

// JPEG decoder library

byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 5;
//TMP36 tmp36(TMP_SENSOR);



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
	tft.fillRect(0, h - 10, w - 1, h - 1, ILI9341_BLACK);

	tft.setCursor(0, h - 10);
	//tft.setTextSize(1.5);
	tft.unloadFont();

	tft.loadFont(AA_FONT_SMALL);
	
	tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
	tft.print(" Alexa prompted me to ask for \"Lisinopril\"?");
}

void setupScreen()
{
	bool buttonPressed = false;
	tft.fillScreen(ILI9341_BLACK);
	tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
	tft.loadFont(AA_FONT_MEDIUM);
	tft.setCursor(tft.width() / 2 - 45, 1);
	tft.println("ECG Monitor");


	tft.drawLine(0, 35, tft.width(), 35, ILI9341_RED);
	//tft.drawLine(5, 70, tft.width() - 1, 70, RED);
	//tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
	
	tft.setCursor(0, 230);
	//tft.setTextSize(1.5);
	tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
	tft.print("Press button to continue...");
	delay(3000);
	/*
	while (!buttonPressed)
	{
		if (digitalRead(BUTTON) == HIGH)
		{
			buttonPressed = true;
		}
	}
*/
	//  tft.setTextSize(2);
tft.unloadFont();

	tft.loadFont(AA_FONT_SMALL);

	tft.setCursor(1, 43);
	tft.print("Pulse:");

	tft.setTextColor(ILI9341_RED, ILI9341_BLACK);

	tft.print("Not found");
	//tft.print(beatspermin);

	drawGrid();
}

void setup()
{

	Serial.begin(115200);

	Serial.println("");

	pinMode(13, INPUT);
	pinMode(15, OUTPUT);
	pinMode(32, INPUT);
	pinMode(33, INPUT);
	pinMode(35, INPUT);
	//digitalWrite(SD_CS, HIGH); // Touch controller chip select (if used)
	digitalWrite(TFT_CS, HIGH); // TFT screen chip select
	//digitalWrite(5, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)

	delay(50);

	if (!rtc.begin())
	{
		Serial.println("Couldn't find RTC");
		//	while (1);
	}
	else
	{
		Serial.print("RTC found.");
	}

	cute.init(BUZZER);

	cute.play(S_MODE1);
	for (size_t i = 0; 50 < 20; i++)
	{
		//if (digitalRead(35) == HIGH)
		//	{

		cute.play(S_SURPRISE);
		WiFi.mode(WIFI_STA);
		WiFi.begin(ssid, password);
		// Wait for connection
		while (WiFi.status() != WL_CONNECTED)
		{
			delay(100);
			Serial.print(".");
		}
		Serial.println("");
		Serial.print("Connected to ");
		Serial.println(ssid);
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());

		server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
				  { request->send(200, "text/plain", "Hi! I am ESP32."); });

		AsyncElegantOTA.begin(&server); // Start ElegantOTA
		server.begin();
		Serial.println("HTTP server started");
		wiFi = true;
		break;
	}

	cute.play(S_HAPPY);

	pinMode(ECG_MINUS, INPUT);
	pinMode(ECG_PLUS, INPUT);

	tft.begin();

	// setting PWM properties
	const int freq = 5000;
	const int ledChannel = 0;
	const int ledRChannel = 1;
	const int ledGChannel = 2;
	const int ledBChannel = 3;
	const int resolution = 8;

	ledcSetup(ledChannel, freq, resolution);

	// attach the channel to the GPIO to be controlled
	ledcAttachPin(15, ledChannel);

	for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++)
	{
		// changing the LED brightness with PWM
		ledcWrite(ledChannel, dutyCycle);
		delay(20);
	}

	// decrease the LED brightness
	for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--)
	{
		// changing the LED brightness with PWM
		ledcWrite(ledChannel, dutyCycle);
		delay(15);
	}



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

	float celsius = 21.00; //tmp36.getTempC();

	//create a variable and store the current temperature in
	//fahrenheit in it using the getTempF function
	float fahrenheit = 68; //tmp36.getTempF();

	float volts = 12; //tmp36.getVoltage();

	//Print the data to the Serial monitor
	Serial.print("Celsius: ");
	Serial.print(celsius);
	Serial.print(" Fahrenheit: ");
	Serial.println(fahrenheit);
	delay(500);
	tft.loadFont(AA_FONT_LARGE);
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
	//delay(3000);
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



	root.close();
  #*/

	Serial.println("Initialising SPIFFs...");
	if (!SPIFFS.begin())
	{
		Serial.println("SPIFFS initialisation failed!");
		//while (1) yield(); // Stay here twiddling thumbs waiting
	}
	else
	{
		Serial.println("\r\nInitialisation done.");
	}
	// ESP32 will crash if any of the fonts are missing
	bool font_missing = false;
	if (SPIFFS.exists("/Roboto-Regular-12.vlw") == false)
		font_missing = true;
	if (SPIFFS.exists("/Roboto-Regular-20.vlw") == false)
		font_missing = true;

	if (font_missing)
	{
		Serial.println("\r\nFont missing in SPIFFS, did you upload it?");
		//   while(1) yield();
	}
	else
		Serial.println("\r\nFonts found OK.");

#ifndef NO_LOG
		//writeFile(SD, "/ecg.csv", "time,value,bpm,temp");
#endif // !NO_LOG

	Serial.print("Temp Celcius: ");
	Serial.println(celsius);

	int x = (tft.width() - 300) / 2 - 1;
	int y = (tft.height() - 300) / 2 - 1;

	tft.setRotation(1); // landscape
	tft.fillScreen(random(0xFFFF));

	delay(2000);
	tft.fillScreen(random(0xFFFF));

	tft.setRotation(0);
	drawJpeg("/nhs.jpg", 0, 0); // This draws a jpeg pulled off the SD Card
	delay(2000);

	drawJpeg("/Mouse480.jpg", 0, 0); // This draws a jpeg pulled off the SD Card
	delay(500);

	drawJpeg("/nhs.jpg", 0, 0); // This draws a jpeg pulled off the SD Card
	delay(900);

	tft.setRotation(1); // landscape
	setupScreen();
}

void SendValueToTFT1(int value)
//Output the current analogue value to the TFT display
{

	int h = tft.height();
	int w = tft.width();
	float celcius = 0; //tmp36.getTempC();

	int padding = tft.textWidth("999999"); // get the width of the text in pixels
  tft.setTextColor(TFT_GREEN, TFT_BLUE);
  tft.setTextPadding(padding);


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
		//	tft.setTextSize(2);
		 tft.fillRect (35, 43, tft.width(), 20, TFT_BLACK); // Overprint with a filled rectangle
			tft.setCursor(38, 43);
			tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
			
			tft.print(BPM);
		//	tft.setTextColor(ILI9341_BLACK, ILI9341_BLACK);
		//	tft.print("      ");

			

			tft.setCursor(71, 43);
			tft.setTextColor(TFT_GREEN, ILI9341_BLACK);
			tft.print("HRV: ");
			tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
			tft.printf("%d", 0);

			tft.setCursor(120, 43);
			tft.setTextColor(TFT_GREEN, ILI9341_BLACK);
			tft.print("SpO2: ");
			tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
			tft.printf("%d%%", 99);

				tft.setCursor(190, 43);
			tft.setTextColor(TFT_GREEN, ILI9341_BLACK);
			tft.print("BP (sys/dia): ");
			tft.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
			tft.printf("%d / %d", 120, 65);

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
		tft.unloadFont();

		tft.loadFont(AA_FONT_LARGE);
		//  tft.setTextSize(4);
		tft.setCursor(10, tft.height() / 2);
		tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
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
	if (wiFi)
	{
		AsyncElegantOTA.loop();
	}
}
