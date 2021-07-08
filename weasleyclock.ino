#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <AccelStepper.h>

const int stepsPerRevolution = 2038;

// Pins from https://chewett.co.uk/blog/1066/pin-numbering-for-wemos-d1-mini-esp8266/
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

// 4 is fullstep, 8 is for halfstepping
AccelStepper hand1Stepper(4, D1, D2, D3, D4);
AccelStepper hand2Stepper(4, D5, D6, D7, D8);

ESP8266WebServer server(80);

const char* ssid = "---"; //Enter Wi-Fi SSID
const char* password =  "---"; //Enter Wi-Fi Password
 
void setup() {
  Serial.begin(115200); //Begin Serial at 115200 Baud
  WiFi.begin(ssid, password);  //Connect to the WiFi network

  Serial.println("Hello!");
  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      Serial.println("Waiting to connect...");
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP

  server.on("/dothing", doThing);
  server.on("/", handle_index); //Handle Index page


  hand1Stepper.setMaxSpeed(2000.0);
  hand1Stepper.setAcceleration(100.0);
  hand1Stepper.setSpeed(300);
  hand2Stepper.setMaxSpeed(2000.0);
  hand2Stepper.setAcceleration(100.0);
  hand2Stepper.setSpeed(300);
  
  hand1Stepper.move(2038 / 4);
  hand2Stepper.move(2038 / 4);
  
  server.begin(); //Start the server
  Serial.println("Server listening");
}

void loop() {
  server.handleClient(); //Handling of incoming client requests
  
  // Move the motor one step
  hand1Stepper.run();
  hand2Stepper.run();
}

void handle_index() {
  //Print Hello at opening homepage
  server.send(200, "text/plain", "Hello! This is an index page.");

}

void doThing() { //Handler for the body path
  if (server.hasArg("plain")== false){ //Check if body received
    server.send(200, "text/plain", "Body not received");
    return;
  }

  int value = server.arg("plain").toInt();

  String message = "Body received:\n";
         message += value;
         message += "\n";
         
  hand1Stepper.moveTo(value);
  hand2Stepper.moveTo(value);

  server.send(200, "text/plain", message);
  Serial.println(message);
}
