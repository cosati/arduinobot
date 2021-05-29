#include <Servo.h>
#include <Ultrasonic.h>

//RGB
#define REDEYE        3
#define GREENEYE      5
#define BLUEEYE       6

//INFRARED
#define BACKIR        4
#define FRONTIR       7

//ESCs
#define RIGHT_MOTOR   9
#define LEFT_MOTOR   10

//ULTRASONIC
#define TRIGGER_PIN  12
#define ECHO_PIN     13

//LDR
#define pinLDR       A0

//RELAYS
#define LRELAY       A1
#define RRELAY       A2

//ESCs values
#define NEUTRAL     480
#define MAXREV        0
#define MAXTHR     1024

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);
Servo mDir, mEsq;
Servo frontServo;

int vinD = NEUTRAL, vinE = NEUTRAL;   
int voutD, voutE; 
int up = 16;
char lastState;
int slow = NEUTRAL + 96;
boolean IRfront, IRback;

void clockwise() {
  if (vinD > MAXREV && vinE < MAXTHR) {
    vinD -= up;
    vinE += up;
  }  
  Serial.print("Clockwise!        ");
}

void counterClockwise() {
  if (vinD < MAXTHR && vinE > MAXREV) {
    vinD += up;
    vinE -= up;
  }  
  Serial.print("Counter clockwise!        ");
}

void reverse() {
  vinD = 832;
  vinE = 320;
  Serial.print("Reverse!      ");
}

void findPath() {
  Serial.println("Looking for new path!       ");
  reverse();
  setMotors();
  while(radarFront() < 20.00) {
    Serial.println("Still reversing!      ");
    delay(100);
    continue;
  }
  breakStop();
  setMotors();
  clockwise();
  setMotors();
  while(radarFront() < 100.00) {
    Serial.println("Turning Clockwise!      ");
    delay(100);
    continue;
  }
  Serial.println("Found new path!      ");
  forward();
  setMotors();
  return;
}

void reverseA() {
  if (vinD == NEUTRAL && vinE == NEUTRAL) {
    delay(1500);
    Serial.println("Delaying!");
  }
  if (vinD > MAXREV && vinE > MAXREV) {
    vinD -= up;
    vinE -= up;
  }  
  Serial.print("Run, Forrest!       ");
}

void forward() {
  vinD = 672;
  vinE = 672;
  Serial.print("Forward!        ");
}

void forwardA() {
  if (vinD < MAXTHR && vinE < MAXTHR) {
    vinD += up;
    vinE += up;
  }
  Serial.print("To Infiniity, and Beyond!       ");
}

void slowDown() {  
  if (vinD > slow && vinE > slow) {
    vinD -= up;
    vinE -= up;
  } else if (vinD < slow && vinE < slow) {
    vinD += up;
    vinE += up;
  } else {  
    vinD = 640;
    vinE = 640;
  }
  Serial.print("Slowing Down!       ");
  return;
}

void breakStop() {
  vinD = NEUTRAL;
  vinE = NEUTRAL;

  Serial.print("Stop!       ");
}

float radarFront() {
  float cmMsec;
  long microsec = ultrasonic.timing();

  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);

  return cmMsec;
}

void debug(float cmMsec, boolean IRfront, boolean IRback) {
  Serial.print("Front: ");
  Serial.print(IRfront ? "FALL | " : "FLOOR | ");

  Serial.print("Back: ");
  Serial.print(IRback ? "NOTHING | " : "OBSTACLE! | ");

  Serial.print("Last State: ");
  Serial.print(lastState);
  Serial.print(" ");
  Serial.print("MD: ");
  Serial.print(vinD);
  Serial.print(", ME: ");
  Serial.print(vinE);
  Serial.print(", CM: ");
  Serial.println(cmMsec);
}

void setMotors() {
  voutD = map(vinD, 0, 1023, 0, 180);
  voutE = map(vinE, 0, 1023, 0, 180);
  
  mDir.write(voutD);
  mEsq.write(voutE);
}

// Calibrate ESCs
void armESC() {
  int v, d = 50;
  int n = map(NEUTRAL, 0, 1023, 0, 180);

  Serial.println("Initializing ESCs");
  
  mDir.write(90);
  mEsq.write(90);

  analogWrite(REDEYE, 500);
  analogWrite(GREENEYE, 0);
  analogWrite(BLUEEYE, 0);
  delay(1000);

  
  analogWrite(GREENEYE, 500);
  delay(1000);

  analogWrite(REDEYE, 0);
  analogWrite(BLUEEYE, 0);
  delay(1000);

  digitalWrite(LRELAY, HIGH);
  digitalWrite(RRELAY, HIGH);
  
  //Neutral up to max throtle
  for (v = n; v < 180; v++) { 
    mDir.write(v);
    mEsq.write(v);
    delay(d);
    Serial.print("R Front: ");
    Serial.println(v);
  }

  delay(1000);

  //Max Throtle down to neutral
  for (v = 180; v >= n; v--) {
    mDir.write(v);
    mEsq.write(v);
    delay(d);
    Serial.print("L Back: ");
    Serial.println(v);
  }

  delay(1000);

  //Max Throtle down to max reverse
  for (v = n; v > 0; v--) {
    mDir.write(v);
    mEsq.write(v);
    delay(d);
    Serial.print("L Back: ");
    Serial.println(v);
  }

  delay(1000);

  //Max reverse up to neutral
  for (v = 0; v <= n; v++) {
    mDir.write(v);
    mEsq.write(v);
    delay(d);
    Serial.print("R Front: ");
    Serial.println(v);
  }

  delay(1000);

  return;

}

int autoLights(int light) {
  switch(light) {
    case 0 ... 100:
      return 700;
    case 101 ... 200:
      return 490;
    case 201 ... 300:
      return 310;
    case 301 ... 400:
      return 150;
    case 401 ... 500:
      return 80;
    case 501 ... 600:
      return 45;
    case 601 ... 700:
      return 30;
    case 701 ... 800:
      return 20;  
    case 801 ... 899:
      return 10;
    default:
      return 0;
  }
}

void autoMode(float cmMsec) {
  //Dodging obstacle
  if (lastState == 'r' || lastState == 'c') {
    // Reversing while obstacle in less than 30cm
    if (cmMsec <= 30.0) {
      reverseA();
      lastState = 'r';
    } else if (lastState != 'c') { //Start turning
      breakStop();
      lastState = 'c';
    } else if (cmMsec < 100.0) { //Still looking
      clockwise();
      lastState = 'c';
    } else { // Found new path
      breakStop();
      lastState = 'b';
    }
  } else if (cmMsec > 80.0) {
    if (lastState != 'f' && lastState != 's'){
      breakStop();
    } else {
      forwardA();
    }
    lastState = 'f';
  } else if (cmMsec > 30) {
    if (lastState != 'f' && lastState != 's'){
      breakStop();
    } else {
      slowDown();
    }
    lastState = 's';
  } else {
    breakStop();
    lastState = 'r';
  } 
  return;
}

void setup() {
  //Infrared sensors
  pinMode(BACKIR, INPUT);
  pinMode(FRONTIR, INPUT);
  
  //RGB
  pinMode(REDEYE, OUTPUT); 
  pinMode(GREENEYE, OUTPUT); 
  pinMode(BLUEEYE, OUTPUT); 

  //ESCs Relays
  pinMode(LRELAY, OUTPUT);
  pinMode(RRELAY, OUTPUT);
  digitalWrite(LRELAY, LOW);
  digitalWrite(RRELAY, LOW);

  //ESCs
  mDir.attach(RIGHT_MOTOR);  
  mEsq.attach(LEFT_MOTOR);
  
  Serial.begin(9600);
  
  armESC();
  
  lastState = 'b';
}

void loop() {
  int light = analogRead(pinLDR);
  float cmMsec = radarFront();
  IRfront = digitalRead(FRONTIR);
  IRback = digitalRead(BACKIR);

  //Autopilot
  autoMode(cmMsec);
  setMotors();

  //LEDs Output
  analogWrite(REDEYE, autoLights(light));
  analogWrite(GREENEYE, autoLights(light));
  analogWrite(BLUEEYE, autoLights(light));
  Serial.println(light);
  
  //debug(cmMsec, IRfront, IRback);
  
  delay(100); 
}
