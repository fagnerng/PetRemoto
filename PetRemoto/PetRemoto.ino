/**
 *  This sketch waits for the button to be pressed. The motor starts in forward direction,
 *  then every time the button is pressed the motor moves in the other direction.
 */
#include <StepperMotor.h>
#include <Ethernet.h>
#include <SPI.h>
#include "RestClient.h"

// 4 pins of the stepper motor board
#define _PIN1 9
#define _PIN2 8
#define _PIN3 7
#define _PIN4 6

// Intderruption on PIN2, push-button connected to pull-up
#define ITR_PIN1 1
#define ITR_PIN2 2
#define ITR_NB 0
#define delayToCheck 10
volatile boolean rotating = false;
const char * serverURL = "192.168.25.11";
const int serverPort = 50001;
const char * serialVersion = "A1B2C3D4E5F6";
int checkDelay = 0;
RestClient client = RestClient(serverURL, serverPort);
String response;
StepperMotor stepper(_PIN1, _PIN2, _PIN3, _PIN4);
int oneDegree = 4048 / 360;
bool powerOnStatus= false;

/**
 * This method is called on the interruption raised on the falling front of the PIN2
 * The start flag is used to avoid rebound front. Interruptions are disabled inside the
 * interrupt vector method.
 * start is reset once it has been processed in the main loop()
 */
void buttonLess()
{
  if (!rotating) {
    rotating = true;
    stepper.move(-oneDegree * 10);
    stepper.stop();
    rotating = false;

  }
}
void buttonPlus()
{
  if (!rotating) {
    rotating = true;
    stepper.move(oneDegree * 10);
    stepper.stop();
    rotating = false;

  }
}

void setup()
{
  client.dhcp();
  stepper = StepperMotor(_PIN1, _PIN2, _PIN3, _PIN4);
  cli();
  stepper.setPeriod(1);
  pinMode(ITR_PIN1, INPUT_PULLUP);
  pinMode(ITR_PIN2, INPUT_PULLUP);
  attachInterrupt(0, buttonLess, FALLING);
  attachInterrupt(1, buttonPlus, FALLING);
  sei();
}

void loop()
{
    if(!powerOnStatus){
      powerOn();
    }
    if(checkDelay > delayToCheck){
      checkFeedRequest();
    }
    checkDelay++;
    delay(1000);
}


void adjustRotation(int degreesvalues) {
  int value = degreesvalues * oneDegree;
   stepper.move(value);
   stepper.stop();
}
void checkFeedRequest() {
  checkDelay = 0;
  response = "";
  int statusCode = client.get("/api/user/dispenser/check?refresh=true&serial=A1B2C3D4E5F6", &response);
  Serial.print(statusCode);
  if (statusCode == 200) {
    int mfeed = response.indexOf("true");
    Serial.print(" : ");
    Serial.println(response);
    if (mfeed != -1) {
      feed();
    }
  } else if (statusCode == 400) {
    checkDelay = delayToCheck / 2;
  }

}
void feed() {
  Serial.println("Alimentando");
  adjustRotation(45);
  setFeedOnServer();
}

void setFeedOnServer() {
  response = "";
  int statusCode = client.post("/api/user/dispenser/feed?serial=A1B2C3D4E5F6&feed=false", "POSTDATA", &response);
  Serial.print(statusCode);
  Serial.print(" : ");
  Serial.println(response);

}

void powerOn() {
  if (powerOnStatus) return;
  response = "";
  int statusCode = client.post("/api/user/dispenser/status?serial=A1B2C3D4E5F6&status=NORMAL", "POSTDATA", &response);
  if (statusCode == 200)
    powerOnStatus = true;
}


