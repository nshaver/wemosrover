//////////////////////////
// wifi
//////////////////////////
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include "index.h"
const char* host= "wemosrover";
const char* ssid = "your_home_ssid";
const char* password = "your_home_password";
const char* apssid = "ssid_for_access_point";
const char* appassword = "password_for_access_point";
MDNSResponder mdns;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
unsigned long lastHandleClient=0;

////////////////////////////////////////////////////////////////
// adafuit motor shield v2.3 - i2c
////////////////////////////////////////////////////////////////
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

// drive motors
Adafruit_DCMotor *motor1=AFMS.getMotor(1);
Adafruit_DCMotor *motor2=AFMS.getMotor(2);

/* enable below if 4wd
Adafruit_DCMotor *motor3=AFMS.getMotor(3);
Adafruit_DCMotor *motor4=AFMS.getMotor(4);
*/

unsigned int drivespeed=255;
unsigned int thisspeed=0;
unsigned long conncnt=1;
unsigned long reqid=0;
int thisdir=0;
unsigned int ms_fr, ms_fl, ms_rl, ms_rr;
char drive[3];
int speed=0;

//////////////////////////
// oled - on esp8266 connect sda and scl to d1 and d2, connect 3.3v and ground
//////////////////////////
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
String text;

//////////////////////////
// PIEZO output pin
//////////////////////////
#define PIEZO 14

//////////////////////////
// wifi camera IP
//////////////////////////
int webcamip=2;

/////////////////////////////////////////////////
// end includes and global variables
/////////////////////////////////////////////////

/////////////////////////////////////////////////
// start misc functions
/////////////////////////////////////////////////

/*
 * showtext
 * x = start at x pixel
 * y = start at y pixel
 * size = font size
 * text = text to display
 * clearDisplay = true|false, clear display before drawing text
 */
void showtext(int x, int y, int size, String text, boolean clearDisplay){
  if (clearDisplay) display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.setTextSize(size);
	display.println(text);
  display.display();
}

/*
 * fDrive - set drive motors
 * m1/m2/m3/m4 = speed. if <0 then backward, if >0 then forward, if =0 then stop
 */
void fDrive(int s1, int s2, int s3, int s4 ){
	if (s1 > 0){
		motor1->run(FORWARD);
		motor1->setSpeed(s1);
	} else if (s1 < 0) {
		motor1->run(BACKWARD);
		motor1->setSpeed(-s1);
	} else {
		motor1->run(RELEASE);
	}

	if (s2 > 0){
		motor2->run(FORWARD);
		motor2->setSpeed(s2);
	} else if (s2 < 0) {
		motor2->run(BACKWARD);
		motor2->setSpeed(-s2);
	} else {
		motor2->run(RELEASE);
	}

	/* enable below if 4wd
	if (s3 > 0){
		motor3->run(FORWARD);
		motor3->setSpeed(s3);
	} else if (s3 < 0) {
		motor3->run(BACKWARD);
		motor3->setSpeed(-s3);
	} else {
		motor3->run(RELEASE);
	}

	if (s4 > 0){
		motor4->run(FORWARD);
		motor4->setSpeed(s4);
	} else if (s4 < 0) {
		motor4->run(BACKWARD);
		motor4->setSpeed(-s4);
	} else {
		motor4->run(RELEASE);
	}
	*/
}

/*
 * handleRoot
 *
 * create and transmit the control GUI
 */
void handleRoot() {
	server.send(200, "text/html", "<head></head><body style=\"font-size:24px;\"><a href=\"/v1\">Video on IP 1</a><br><head></head><body><a href=\"/v2\">Video on IP 2</a></body></html>");
}

void handleV1() {
	webcamip=2;
	conncnt++;
	beep(150, 750, 1);
	server.send(200, "text/html", html1+(String)webcamip+html2);

	// black out the area that we're about to write to
	display.fillRect(0, 30, 128, 10, BLACK);

	text="conn:"+(String)conncnt;
	showtext(0, 30, 1, text, false);
}

void handleV2() {
	webcamip=3;
	conncnt++;
	beep(150, 750, 1);
	server.send(200, "text/html", html1+(String)webcamip+html2);

	// black out the area that we're about to write to
	display.fillRect(0, 30, 128, 10, BLACK);

	text="conn:"+(String)conncnt;
	showtext(0, 30, 1, text, false);
}

/*
 * handleDrive
 *
 * receive GET parameters with drive and speed variables
 */
void handleDrive(){
	memset(drive, 0, sizeof(drive));
	speed=0;
	reqid=0;
	// get GET arguments
	if (server.args() > 0){
		for (uint8_t i=0; i<server.args(); i++){
			if (server.argName(i)=="drive") server.arg(i).toCharArray(drive, 3);
			if (server.argName(i)=="dist") speed=server.arg(i).toInt();
			if (server.argName(i)=="reqid") reqid=server.arg(i).toInt();
		}
	}

	if (reqid==0){
		server.send(200, "text/html", "invalid request");
		return;
	}

	/*
	 * motor layout, fDrive(m1, m2, m3, m4)
	 *  
	 *  m2 ^^^ m1
	 *     ---
	 *     ---
	 *  m3 ^^^ m4
	 */
	if (strcmp(drive, "x")==0){
		// stop
		fDrive(0, 0, 0, 0);
		Serial.println("stop");
		display.fillRect(0, 50, 128, 10, BLACK);
		text="stop";
		showtext(0, 50, 1, text, false);
	} else {
		// driving some direction, get speed
		thisspeed=drivespeed*speed/10;
		if (thisspeed>drivespeed) thisspeed=drivespeed;
		if (thisspeed<0) thisspeed=0;

		if (strcmp(drive, "n")==0){
			fDrive(thisspeed, thisspeed, thisspeed, thisspeed);	
			Serial.println("north");
			display.fillRect(0, 50, 128, 10, BLACK);
			text="north";
			showtext(0, 50, 1, text, false);
		} else if (strcmp(drive, "s")==0){
			fDrive(-thisspeed, -thisspeed, -thisspeed, -thisspeed);	
			Serial.println("south");
			display.fillRect(0, 50, 128, 10, BLACK);
			text="south";
			showtext(0, 50, 1, text, false);
		} else if (strcmp(drive, "e")==0){
			fDrive(-thisspeed, thisspeed, thisspeed, -thisspeed);	
			Serial.println("east");
			display.fillRect(0, 50, 128, 10, BLACK);
			text="east";
			showtext(0, 50, 1, text, false);
		} else if (strcmp(drive, "w")==0){
			fDrive(thisspeed, -thisspeed, -thisspeed, thisspeed);	
			Serial.println("west");
			display.fillRect(0, 50, 128, 10, BLACK);
			text="west";
			showtext(0, 50, 1, text, false);
		} else if (strcmp(drive, "nw")==0){
			fDrive(thisspeed, 0.5*thisspeed, 0.5*thisspeed, thisspeed);	
			Serial.println("northwest");
			display.fillRect(0, 50, 128, 10, BLACK);
			text="northwest";
			showtext(0, 50, 1, text, false);
		} else if (strcmp(drive, "sw")==0){
			fDrive(-thisspeed, -0.5*thisspeed, -0.5*thisspeed, -thisspeed);	
			Serial.println("southwest");
			display.fillRect(0, 50, 128, 10, BLACK);
			text="southwest";
			showtext(0, 50, 1, text, false);
		} else if (strcmp(drive, "se")==0){
			fDrive(-0.5*thisspeed, -thisspeed, -thisspeed, -0.5*thisspeed);
			Serial.println("southeast");
			display.fillRect(0, 50, 128, 10, BLACK);
			text="southeast";
			showtext(0, 50, 1, text, false);
		} else if (strcmp(drive, "ne")==0){
			fDrive(0.5*thisspeed, thisspeed, thisspeed, 0.5*thisspeed);
			Serial.println("northeast");
			display.fillRect(0, 50, 128, 10, BLACK);
			text="northeast";
			showtext(0, 50, 1, text, false);
		} else {
			// not defined, stop
			fDrive(0, 0, 0, 0);
			Serial.println("unkonwn");
			display.fillRect(0, 50, 128, 10, BLACK);
			text="stop";
			showtext(0, 50, 1, text, false);
		}
	}

	server.send(200, "text/html", "{\"reqid\":"+(String)reqid+",\"status\":\"ok\"}");
}

/*
 * handleNotFound
 *
 * tell the browser that the page is not found
 */
void handleNotFound(){
	server.send(404, "text/plain", "File not found");
}

void startWifi(){
	//////////////////////////////////////////////
	// wifi
	//////////////////////////////////////////////

	// turn off LED
	pinMode(BUILTIN_LED, OUTPUT);

	beep(150, 100, 1);
	beep(150, 100, 1);
	text="Waiting for "+(String)ssid;
	showtext(0, 0, 1, text, true);

	digitalWrite(BUILTIN_LED, HIGH);

	// setup wifi as both a client and as an AP
	WiFi.mode(WIFI_AP_STA);

	// start the access point
	WiFi.softAP(apssid, appassword);

	// start the client
	WiFi.begin(ssid, password);
	
	Serial.println("");

	unsigned int conn_tries=0;
	boolean wifi_client_gaveup=false;
	while (WiFi.status() != WL_CONNECTED) {
		conn_tries++;
		delay(500);
		Serial.print(".");
		if (conn_tries>20) {
			// give up, just be an AP via softAP
			wifi_client_gaveup=true;
			beep(200, 750, 1);
			text="AP Only: "+(String)apssid;
			showtext(0, 0, 1, text, true);
			break;
		}
	}

	if (wifi_client_gaveup==false){
		Serial.print("Connected to ssid: ");
		Serial.println(ssid);
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());

		// register hostname on network
		if (mdns.begin(host, WiFi.localIP())) {
			Serial.print("Hostname: ");
			Serial.println(host);
		}

		beep(150, 100, 1);
		beep(150, 100, 1);
		beep(150, 100, 1);
		text="AP="+(String)apssid+"\nCL="+(String)host+"\nIP="+WiFi.localIP().toString().c_str();
		showtext(0, 0, 1, text, true);
	}

	Serial.print("AP ssid: ");
	Serial.println(apssid);
	Serial.print("AP IP address: ");
	Serial.println(WiFi.softAPIP());

	// establish http bootloader updater
	httpUpdater.setup(&server);

	server.on("/", handleRoot);
	server.on("/v1", handleV1);
	server.on("/v2", handleV2);
	server.on("/drive", handleDrive);
	server.onNotFound(handleNotFound);
	
	server.begin();
	Serial.println("HTTP server started");

	// turn on LED
	digitalWrite(BUILTIN_LED, LOW);
	
}

void beep(int pitch, int duration, int times) {
	for (int t=0; t<times; t++){
		for (int i = 0; i < duration; i++) {
			digitalWrite(PIEZO, HIGH);
			delayMicroseconds(pitch);
			digitalWrite(PIEZO, LOW);
			delayMicroseconds(pitch);
		}
		delay(100);
	}
}

/////////////////////////////////////////////////
// end misc functions
/////////////////////////////////////////////////

void setup(){
	Serial.begin(115200);

	//////////////////////////////////////////////
	// setup piezo speaker
	//////////////////////////////////////////////
	pinMode(PIEZO, OUTPUT);

	//////////////////////////////////////////////
	// oled
	//////////////////////////////////////////////
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();

	//////////////////////////////////////////////
	// drive motors
	//////////////////////////////////////////////
	AFMS.begin();
	fDrive(0, 0, 0, 0);

	startWifi();
}

void loop() {
	// handle webpages every x ms
	server.handleClient();
}

