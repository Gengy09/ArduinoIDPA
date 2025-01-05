#include <pt.h> // Protothread Library

const int StepPinX = 2; // Schrittmotor mit 140 Grad
const int DirPinX = 5;  // Schrittmotor mit 140 Grad
const int StepPinY = 3; // Schrittmotor mit 360 Grad
const int DirPinY = 6;  // Schrittmotor mit 360 Grad
const int EnablePin = 8;
const int Signalgeber = A0; // Für den Optischen Endschalter
const int MotorEinzug = A1; // Relais für Einzugskolben
const int Riemencw = 11;  // Riemen 
const int Riemenccw = 12 ;  // Riemen Richtung
const int Tackercw = 10;  // Tacker
const int Tackerccw = A3;  // Tacker Richtung

// Motor-Einstellungen
const int STEPS_PER_REV = 200; // Standard für 1.8° Step Motoren (200 Schritte pro Umdrehung)
const float DEGREE_TO_STEP = STEPS_PER_REV / 360.0; // Schritte pro Grad

// Protothreads
static struct pt pt1, pt2, pt3, pt4, pt5;

void setup() {
  pinMode(StepPinX, OUTPUT);
  pinMode(DirPinX, OUTPUT);
  pinMode(StepPinY, OUTPUT);
  pinMode(DirPinY, OUTPUT);
  pinMode(EnablePin, OUTPUT);
  pinMode(Signalgeber, INPUT_PULLUP);
  pinMode(MotorEinzug, OUTPUT); 
  digitalWrite(MotorEinzug, LOW); // Relais initial ausschalten
  pinMode(Riemencw, OUTPUT);
  pinMode(Riemenccw, OUTPUT);
  pinMode(Tackercw, OUTPUT);
  pinMode(Tackerccw, OUTPUT);

  digitalWrite(EnablePin, LOW); // EN-Pin aktivieren
  digitalWrite(DirPinX, HIGH); // HIGH: Uhrzeigersinn, LOW: Gegen Uhrzeigersinn

  // Protothreads initialisieren
  PT_INIT(&pt1);
  PT_INIT(&pt2);
  PT_INIT(&pt3);
  PT_INIT(&pt4);
  PT_INIT(&pt5);
}

// Funktion zum Drehen eines Motors
void rotateMotor(int stepPin, int dirPin, int steps, bool clockwise) {
  digitalWrite(dirPin, clockwise ? HIGH : LOW);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delay(2);
    digitalWrite(stepPin, LOW);
    delay(2);
  }
}

// Protothread 1: Motor Y dreht nach 5 Sekunden Verzögerung um 360 Grad
static int protothread1(struct pt *pt) {
  static unsigned long startTime;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, digitalRead(Signalgeber) == LOW); // Warten auf Signalgeber
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 6100); // 5 Sekunden warten
    rotateMotor(StepPinY, DirPinY, 360 * DEGREE_TO_STEP, true); // Motor Y drehen
  }
  PT_END(pt);
}

// Protothread 2: Motor X dreht 140 Grad gegen den Uhrzeigersinn und zurück
static int protothread2(struct pt *pt) {
  static unsigned long startTime;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, digitalRead(Signalgeber) == LOW); // Warten auf Signalgeber
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 100); // 2 Sekunden warten
    rotateMotor(StepPinX, DirPinX, 140  * DEGREE_TO_STEP, false); // Gegen Uhrzeigersinn
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 6000); // 5 Sekunden warten
    rotateMotor(StepPinX, DirPinX, 140 * DEGREE_TO_STEP, true); // Uhrzeigersinn
  }
  PT_END(pt);
}

// Protothread 3: Relais alle 5 Sekunden schalten
static int protothread3(struct pt *pt) {
  static unsigned long startTime;
  PT_BEGIN(pt);
  while (1) {
    digitalWrite(MotorEinzug, HIGH); // Relais einschalten
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 1200); // 2 Sekunden warten
    digitalWrite(MotorEinzug, LOW); // Relais ausschalten
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 1200); // 5 Sekunden warten
  }
  PT_END(pt);
}

// Protothread 4: Motor Riemen dreht 3 Sekunden vorwärts, dann rückwärts
static int protothread4(struct pt *pt) {
  static unsigned long startTime;
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, digitalRead(Signalgeber) == LOW); // Warten auf Signalgeber
     startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 1000); // 3 Sekunden warten
    // Motor vorwärts drehen (CW)
    digitalWrite(Riemencw, HIGH);  // Richtung vorwärts
    digitalWrite(Riemenccw, LOW);
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 1000); // 3 Sekunden warten

    // Motor stoppen
    digitalWrite(Riemencw, LOW);
    digitalWrite(Riemenccw, LOW);
    delay(3000); // Kleine Pause

    // Motor rückwärts drehen (CCW)
    digitalWrite(Riemencw, LOW);
    digitalWrite(Riemenccw, HIGH); // Richtung rückwärts
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 1200); // 3 Sekunden warten

    // Motor stoppen
    digitalWrite(Riemencw, LOW);
    digitalWrite(Riemenccw, LOW);
  }
  PT_END(pt);
}
 

// Protothread 5: Motor Tacker dreht 3 Sekunden vorwärts, dann rückwärts
static int protothread5(struct pt *pt) {
  static unsigned long startTime;
  PT_BEGIN(pt);
  while (1) {
    // Warten auf Signalgeber
    PT_WAIT_UNTIL(pt, digitalRead(Signalgeber) == LOW);

    // Wartezeit statt delay(3000)
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 1800); // 3 Sekunden Pause

    // Motor vorwärts drehen (CW)
    digitalWrite(Tackercw, HIGH);  // Richtung vorwärts
    digitalWrite(Tackerccw, LOW);
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 1000); // 1.4 Sekunden warten

    // Motor stoppen
    digitalWrite(Tackercw, LOW);
    digitalWrite(Tackerccw, LOW);

    // Wartezeit statt delay(100)
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 100); // 0.1 Sekunden Pause

    // Motor rückwärts drehen (CCW)
    digitalWrite(Tackercw, LOW);
    digitalWrite(Tackerccw, HIGH); // Richtung rückwärts
    startTime = millis();
    PT_WAIT_UNTIL(pt, millis() - startTime >= 1100); // 1.4 Sekunden warten

    // Motor stoppen
    digitalWrite(Tackercw, LOW);
    digitalWrite(Tackerccw, LOW);
  }
  PT_END(pt);
}



void loop() {
  protothread1(&pt1);
  protothread2(&pt2);
  protothread3(&pt3);
  protothread4(&pt4);
 protothread5(&pt5);
}
