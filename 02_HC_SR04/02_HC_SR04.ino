#include <Servo.h>
#include <Ultrasonic.h>

#define TRIGGER_PIN  12
#define ECHO_PIN     13
#define RIGHT_MOTOR   9
#define LEFT_MOTOR   10
#define NEUTRAL     480
#define MAXREV        0
#define MAXTHR     1024

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);
Servo mDir, mEsq;

int vinD = NEUTRAL, vinE = NEUTRAL;   
int voutD, voutE; 
int up = 16;
char lastState;
int slow = NEUTRAL + 96;

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

void debug(float cmMsec) {
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

  delay(10000);
  
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

void setup() {
  mDir.attach(RIGHT_MOTOR);  
  mEsq.attach(LEFT_MOTOR);
  Serial.begin(9600);
  armESC();
  lastState = 'b';
}

void loop() {
  float cmMsec = radarFront();

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

  debug(cmMsec);
  
  setMotors();

  delay(100); 
}
