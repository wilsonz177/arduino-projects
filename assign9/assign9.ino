#include <SparkFun_MMA8452Q.h>
#include <Wire.h> // Must include Wire library for I2C


MMA8452Q accel;
int sleepMode = LOW;
int sleepLight = 13;
int buttonPin = 2;
int buttonState;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long timeAsleep = 0;
unsigned long timeOfSleepModeChangeToHigh = 0;
int motionDetected = LOW;
int stepButton = 8;
int stepCount = 0;

const int filter_tcounts = 7;
float temperatures[filter_tcounts];
int tcount = 0;
float avgTemp = 0;

bool zLastTimeBelowThreshold = true;

/*protocol:
   0x21 '!' magic number
   0x30 debug string
   0x31 error string
   0x32 temp sensor reading float
   0x33 int step counts from accelerometer
   0x34 unsigned long time spent asleep
   0x35 millis() total time program running
   0x36 accelerometer values
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(stepButton, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  analogReference(DEFAULT);
  pinMode(A1, INPUT);
  Serial.println("hi");
  accel.init();
  Serial.println("yo");
}


void loop() {
  // put your main code here, to run repeatedly:
  checkStepButton();
  checkSleepButton();
  digitalWrite(13, sleepMode);
  detectMotion();
  timeUpdate();
  readTemp();
  sendDebug("Debug string");
  errorString();
  sendTemp();
  sendAcc();
  sendSteps();
  sendTimeAsleep();
  sendTotalTime();
}

void sendTotalTime() {
  Serial.write(0x21);
  Serial.write(0x35);
  Serial.write(millis() >> 24);  //Sends first byte
  Serial.write(millis() >> 16);  //second byte
  Serial.write(millis() >> 8);   //third byte
  Serial.write(millis());        //fourth byte
}

void sendTimeAsleep() {
  Serial.write(0x21);
  Serial.write(0x34);
  Serial.write(timeAsleep >> 24);  //Sends first byte
  Serial.write(timeAsleep >> 16);  //second byte
  Serial.write(timeAsleep >> 8);   //third byte
  Serial.write(timeAsleep);        //fourth byte
}

void sendSteps() {
  Serial.write(0x21);
  Serial.write(0x33);
  Serial.write(stepCount >> 8);
  Serial.write(stepCount);
}

void sendTemp() {
  Serial.write(0x21);
  Serial.write(0x32);
  float t1 = avgTemp;
  unsigned long rawBits;
  rawBits = *(unsigned long *) &t1; //rawbits is the float temp converted to long
  Serial.write(rawBits >> 24);  //Sends first byte
  Serial.write(rawBits >> 16);  //second byte
  Serial.write(rawBits >> 8);   //third byte
  Serial.write(rawBits);        //fourth byte
}

void errorString() { //ERROR STRING 0x31
  if (avgTemp > 20) {
    Serial.write(0x21);
    Serial.write(0x31);
    char a[] = "Error String";
    int length = strlen(a);
    Serial.write(length >> 8); //Sends the first byte of length
    Serial.write(length);      //Sends the second byte of length
    Serial.write(a); //Sends chars
  }
}

void sendDebug(char a []) {
  Serial.write(0x21);
  Serial.write(0x30);
  int length = strlen(a);
  Serial.write(length >> 8); //Sends the first byte of length
  Serial.write(length);      //Sends the second byte of length
  Serial.write(a); //Sends chars
}

//reads temperature and filters it to receive an average temperature
void readTemp() {
  float temp = ((((float)analogRead(A1) / 1024 * 5) - 0.75) * 100) + 25;
  temperatures[tcount % filter_tcounts] = temp;
  tcount += 1;
  avgTemp = 0;
  for (int i = 0; i < filter_tcounts; ++i) {
    avgTemp += temperatures[i];
  }
  avgTemp = avgTemp / (float)filter_tcounts;
}

void sendAcc() {
  Serial.write(0x21);
  Serial.write(0x36);
  float x1 = accel.cx;
  float y1 = accel.cy;
  float z1 = accel.cz;
  unsigned long xRawBits;
  xRawBits = *(unsigned long *) &x1; //rawbits is the float temp converted to long
  Serial.write(xRawBits >> 24);  //Sends first byte
  Serial.write(xRawBits >> 16);  //second byte
  Serial.write(xRawBits >> 8);   //third byte
  Serial.write(xRawBits);        //fourth byte
  unsigned long yRawBits;
  yRawBits = *(unsigned long *) &y1; //rawbits is the float temp converted to long
  Serial.write(yRawBits >> 24);  //Sends first byte
  Serial.write(yRawBits >> 16);  //second byte
  Serial.write(yRawBits >> 8);   //third byte
  Serial.write(yRawBits);        //fourth byte
  unsigned long zRawBits;
  zRawBits = *(unsigned long *) &z1; //rawbits is the float temp converted to long
  Serial.write(zRawBits >> 24);  //Sends first byte
  Serial.write(zRawBits >> 16);  //second byte
  Serial.write(zRawBits >> 8);   //third byte
  Serial.write(zRawBits);        //fourth byte
}

// a step is a accel z value going above the threshold and
// then later going below the threshold
//delta timing is added so that 1 step wont be perceived as
//  multiple steps
void pedometer() {
  if (zLastTimeBelowThreshold) {
    if (accel.cz > 1.7) {
      zLastTimeBelowThreshold = false;
    } else {
      zLastTimeBelowThreshold = true;
    }
  } else {
    if (accel.cz <= 1.7) {
      stepCount += 1;
      zLastTimeBelowThreshold = true;
    }
  }
}

//checks if button is pressed to set stepcount to zero
void checkStepButton() {
  if (digitalRead(stepButton) == LOW) {
    stepCount = 0;
    Serial.write(0x21);
    Serial.write(0x37);
    Serial.write(millis() >> 24);  //Sends first byte
    Serial.write(millis() >> 16);  //second byte
    Serial.write(millis() >> 8);   //third byte
    Serial.write(millis());        //fourth byte
  }
}

//accelerometer code, detects if there is motion depending on ur orientation
void detectMotion() {
  if (accel.available()) {
    accel.read();
    //    printCalculatedAccels();
    byte pl = accel.readPL();
    if (sleepMode == HIGH) {
      switch (pl) {
        case PORTRAIT_U:
          //          Serial.print("Portrait Up");
          if ((accel.cy > -1.4 && accel.cy < -0.6) && (accel.cx < 0.4 && accel.cx > -0.4) && (accel.cz < 0.4 && accel.cz > -0.4)) {
            motionDetected = LOW;
          } else {
            //            Serial.println("motion detected");
            motionDetected = HIGH;
          }
          break;
        case PORTRAIT_D:
          //          Serial.print("Portrait Down");
          if ((accel.cy < 1.4 && accel.cy > 0.6) && (accel.cx < 0.4 && accel.cx > -0.4) && (accel.cz < 0.4 && accel.cz > -0.4)) {
            motionDetected = LOW;
          } else {
            //            Serial.println("motion detected");
            motionDetected = HIGH;
          }
          break;
        case LANDSCAPE_R:
          //          Serial.print("Landscape Right");
          if ((accel.cx < 1.4 && accel.cx > 0.6) && (accel.cy < 0.4 && accel.cy > -0.4) && (accel.cz < 0.4 && accel.cz > -0.4)) {
            motionDetected = LOW;
          } else {
            //            Serial.println("motion detected");
            motionDetected = HIGH;
          }
          break;
        case LANDSCAPE_L:
          //          Serial.print("Landscape Left");
          if ((accel.cx > -1.4 && accel.cx < -0.6) && (accel.cy < 0.4 && accel.cy > -0.4) && (accel.cz < 0.4 && accel.cz > -0.4)) {
            motionDetected = LOW;
          } else {
            //            Serial.println("motion detected");
            motionDetected = HIGH;
          }
          break;
        case LOCKOUT:
          //          Serial.print("Flat");
          if ((accel.cz < 1.4 && accel.cz > 0.6) && (accel.cx < 0.4 && accel.cx > -0.4) && (accel.cy < 0.4 && accel.cy > -0.4)) {
            motionDetected = LOW;
          } else {
            //            Serial.println("motion detected");
            motionDetected = HIGH;
          }
          break;
      }
    } else {
      pedometer();
    }
    //    Serial.println();
  }
}


void timeUpdate() {
  if (sleepMode == HIGH && motionDetected == LOW) {
    timeAsleep = millis() - timeOfSleepModeChangeToHigh;
  }
  if (motionDetected == HIGH) {
    //    Serial.println("NOT ASLEEP");
  }
  //  Serial.print("Time update: ");
  //  Serial.print(timeAsleep);
  //  Serial.println();
  //  Serial.println(timeAsleep);
}

void printCalculatedAccels() {
  float x = accel.cx;
  float y = accel.cy;
  float z = accel.cz;
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.print(z);
  Serial.print(",");
}

void checkSleepButton() {
  // read the state of the button on or off? 1 is off 0 is on
  int reading = digitalRead(buttonPin);

  // if the reading is not the same as last time
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {

    //if button state has changed
    if (reading != buttonState) {

      //button state will now but what is read
      buttonState = reading;

      //if buttonstate/reading is low(was pressed)
      if (buttonState == LOW) {
        sleepMode = !sleepMode;
        timeAsleep = 0;
        if (sleepMode == HIGH) {
          timeOfSleepModeChangeToHigh = millis();
        }
      }
    }
  }
  lastButtonState = reading;

}
