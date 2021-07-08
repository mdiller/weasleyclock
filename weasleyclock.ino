#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <AccelStepper.h>
#include <ArduinoJson.h>

#define WIFI_SSID "<defined in config.h>"
#define WIFI_PASSWORD "<defined in config.h>"

#include "config.h"


#define STEPS_PER_REVOLUTION 2038

// Pins from https://chewett.co.uk/blog/1066/pin-numbering-for-wemos-d1-mini-esp8266/
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

// 4 is for the type of stepper motor i have. See: https://www.pjrc.com/teensy/td_libs_AccelStepper.html
// Note that the order of the pins should be IN1 IN3 IN2 IN4. if you do it 1-4 in order, the tracking wont work properly with negative (CCW) moves.
AccelStepper hand1Stepper(4, D1, D3, D2, D4);
AccelStepper hand2Stepper(4, D5, D7, D6, D8);

ESP8266WebServer server(80);

void setup() {
	Serial.begin(115200);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	Serial.println("Hello!");
	
	while (WiFi.status() != WL_CONNECTED) {
			delay(500);
			Serial.println("Waiting to connect...");
	}
	
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	server.on("/", showInfo);
	server.on("/stepHand", stepHand);
	server.on("/moveHand", moveHand);

	hand1Stepper.setMaxSpeed(300.0);
	hand1Stepper.setAcceleration(100.0);
	hand1Stepper.setSpeed(300);
	hand2Stepper.setMaxSpeed(300.0);
	hand2Stepper.setAcceleration(100.0);
	hand2Stepper.setSpeed(300);
	
	// hand1Stepper.move(STEPS_PER_REVOLUTION / 4);
	// hand2Stepper.move(STEPS_PER_REVOLUTION / 4);
	
	server.begin();
	Serial.println("Server listening");
}

void loop() {
	server.handleClient();
	
	// Move the motor one step
	hand1Stepper.run();
	hand2Stepper.run();
}

// dumps info to the returned html page
void showInfo() {
	String body = "Info";

	body += "\nhand1: " + String(hand1Stepper.currentPosition());
	body += "\nhand2: " + String(hand2Stepper.currentPosition());

	server.send(200, "text/plain", body);
}

// steps the hand the given number of steps, can be negative
void stepHand() {
	if (server.hasArg("plain") == false){
		server.send(200, "text/plain", "Body not received");
		return;
	}

	DynamicJsonDocument json(1024);
	deserializeJson(json, server.arg("plain"));

	int hand = json["hand"];
	int steps = json["steps"];

	// Step the hand for given steps
	if (hand == 1) {
		hand1Stepper.move(steps);
	}
	else if (hand == 2) {
		hand2Stepper.move(steps);
	}
	else {
		server.send(400, "text/plain", "Invalid hand");
		return;
	}

	String message = "stepHand()";
	message += "\n hand: " + String(hand);
	message += "\nsteps: " + String(steps);

	server.send(200, "text/plain", message);
	Serial.println(message);
}

// Gets a clock position from the given location
int getClockPositionFromLocation(String location) {
	String locations[] = {
		"home",
		"travelling",
		"lost",
		"family",
		"work",
		"store"
	};
	for (int i = 0; i < 6; i++) {
		if (locations[i] == location) {
			return i * 2; // This gives the position on the clock face
		}
	}
	return 0; // if given a bad value, say we're at home
}

// moves the hand to the passed in clock position [0-12), can be decimal
void moveHand() {
	if (server.hasArg("plain") == false){
		server.send(200, "text/plain", "Body not received");
		return;
	}

	DynamicJsonDocument json(1024);
	deserializeJson(json, server.arg("plain"));

	int hand = json["hand"];
	double position = json["position"];
	String location = json["location"]; // optional param, denotes a physical location that matches a place on the clock
	if (json.containsKey("location")) {
		position = getClockPositionFromLocation(location);
	}

	int stepPos = round((position / 12) * STEPS_PER_REVOLUTION);

	// Step the hand for given steps
	if (hand == 1) {
		hand1Stepper.moveTo(stepPos);
	}
	else if (hand == 2) {
    stepPos = 0 - stepPos; // this one interacts with a gear, so we want it to spin the other way
		hand2Stepper.moveTo(stepPos);
	}
	else {
		server.send(400, "text/plain", "Invalid hand");
		return;
	}

	String message = "moveHand()";
	message += "\n    hand: " + String(hand);
	message += "\nposition: " + String(position);
	message += "\n stepPos: " + String(stepPos);

	server.send(200, "text/plain", message);
	Serial.println(message);
}
