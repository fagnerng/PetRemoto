/* RestClient simple GET request
 *
 * by Chris Continanza (csquared)
 */

#include <Ethernet.h>
#include <SPI.h>
#include "RestClient.h"
#include <CustomStepper.h>

/** Constantes de configurações  **/
const char * serverURL = "192.168.25.11";
const int serverPort = 50001;
const char * serialVersion = "A1B2C3D4E5F6";
const int delayToCheck = 10; /**em segundos **/
int checkDelay = 0;
boolean powerOnStatus = false;
RestClient client = RestClient(serverURL, serverPort);


/** Pinos para o stepper **/
const int pinStepper1 = 10;
const int pinStepper2 = 11;
const int pinStepper3 = 12;
const int pinStepper4 = 13;

CustomStepper stepper(pinStepper1, pinStepper2, pinStepper3, pinStepper4, (byte[]) {
  8, B1000, B1100, B0100, B0110, B0010, B0011, B0001, B1001
}, 4075.7728395, 12, CW);

/** Pinos para ajuste **/
const int pinAdjustPlus = 8;
const int pinAdjustLess = 9;

boolean rotatedeg = false;





//Setup
void setup() {
  Serial.begin(9600);
  pinMode(pinAdjustPlus, INPUT);
  pinMode(pinAdjustLess, INPUT);
  // Connect via DHCP
  Serial.println("connect to network");
  client.dhcp();

  Serial.println("Setup!");
}

String response;
void loop() {
  
  if (!rotatedeg) {
    Serial.println(rotatedeg);
    Serial.println(stepper.isDone());
    powerOn();
    if (checkDelay > delayToCheck) {
      checkFeedRequest();
    }
    if (!checkAdjust()) {
      checkDelay++;
      delay(1000);
    }
  }
  if (stepper.isDone()) {
    rotatedeg = false;
  }
  stepper.run();
}
boolean checkAdjust() {
  int adjustPlus, adjustLess;
  adjustPlus = digitalRead(pinAdjustPlus);

  if (adjustPlus == HIGH) {
    adjustRotation(+10);
    return true;
  }
  adjustLess = digitalRead(pinAdjustLess);
  if (adjustLess == HIGH) {
    adjustRotation(-10);
    return true;
  } return false;
}

void adjustRotation(int degreesvalues) {
  rotatedeg = true;
  int value = degreesvalues;
  if (degreesvalues < 0) {
    value = -degreesvalues;
    stepper.setDirection(CW);
  } else {
    stepper.setDirection(CCW);
  }
  stepper.rotateDegrees(value);
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


