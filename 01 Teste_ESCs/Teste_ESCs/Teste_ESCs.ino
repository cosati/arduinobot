#include <Servo.h>

Servo mDir, mEsq;

int vinD = 416, vinE = 416;   
int up = 32;
int voutD, voutE; 
int v;

void armESC() {

  for (v = 180; v >= 0; v--) {
    mDir.write(v);
    mEsq.write(v);
    delay(15);
    Serial.print("Writting: ");
    Serial.println(v);
  }
  
  for (v = 0; v <= 180; v++) {
    mDir.write(v);
    mEsq.write(v);
    delay(15);
    Serial.print("Writting: ");
    Serial.println(v);  
  }
  
}

void setup() {
  mDir.attach(9);  
  mEsq.attach(10);
  Serial.begin(9600);
  armESC();
}

void loop() {
  voutD = map(vinD, 0, 1023, 0, 180);
  voutE = map(vinD, 0, 1023, 0, 180);
  /*Serial.print("vinD: ");
  Serial.print(vinD);
  Serial.print(" - ");
  Serial.print("voutD: ");
  Serial.println(voutD);*/
  Serial.print("vinE: ");
  Serial.print(vinE);
  Serial.print(" - ");
  Serial.print("voutE: ");
  Serial.println(voutE);
  mDir.write(voutD);
  mEsq.write(voutE);
  delay(5000);
  if (vinD <= 0 || vinD >= 1024) {
    up = -up;
  }
  vinD = vinD + up;
  vinE = vinE + up;
}
