#include <RCSwitch.h>
#include <PimaticProbe.h>
#include <RFControl.h>

// 13 - beeper
// 2 - Switch Left
// 3 - Switch Right
// 4 - 
// 5 - D0
// 6 - D1
// 7 - D2
// 8 - D3
// 9 - Red
// 10 - Green
// 11 - Blue
// 12 - Transmitter

// Color state code.
// Red - Defcon 3 - 1000
// Yellow - Defcon 2 - 0100
// Green - Defcon 1 - 0010
// Blue - Deactivated - 0001

volatile int lock_count = 0;
volatile int unlock_count = 0;
volatile bool locked = false;
int control_state = 0;
PimaticProbe probe = PimaticProbe(12, 1337);

void setup() {
  // put your setup code here, to run once:
  // Alarm.
  pinMode(13, OUTPUT);
  // Transmitter.
  pinMode(12, OUTPUT);
  // Switch left.
  pinMode(2, INPUT);
  // Switch right.
  pinMode(3, INPUT);

  // Set buzzer to off.
  digitalWrite(13, LOW);

  // TODO(amogha): Might not be necessary.
  attachInterrupt(digitalPinToInterrupt(2), unlock, RISING);
  attachInterrupt(digitalPinToInterrupt(3), lock, RISING);

  Serial.begin(9600);
}


void loop() {
  // Read radio inputs.
  int d0 = digitalRead(5);
  int d1 = digitalRead(6);
  int d2 = digitalRead(7);
  int d3 = digitalRead(8);
  setState(d0, d1, d2, d3);
  
  Serial.print("Counts: "); 
  Serial.print(lock_count);
  Serial.print(" ");
  Serial.print(unlock_count);
  Serial.print(" ");
  Serial.print(d3);
  Serial.print(" ");
  Serial.print(d2);
  Serial.print(" ");
  Serial.print(d1);
  Serial.print(" ");
  Serial.println(d0);
  delay(1);
}

void setState(int d0, int d1, int d2, int d3) {
  control_state = (d3 << 3) | (d2 << 2) | (d1 << 1) | (d0);
  // Update the state based on the data.
  Serial.println(control_state);
  // Set the led value based on the state.
  switch (control_state) {
    case 6: setLED(0, 255, 255);
            break;
    case 10: setLED(255, 0, 255);
            break;
  }
}

void setLED(int r, int g, int b) {
  // 
  analogWrite(9, r);
  analogWrite(10, g);
  analogWrite(11, b);
}

void transmit() {
  unsigned long buckets[8] = {189, 547, 5720, 0, 0, 0, 0, 0};
  RFControl::sendByCompressedTimings(12, buckets, "0010101", 10);
}

void unlock() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 500ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 500) {
   lock_count++;
   // Transmit defcon lowered.
   probe.transmit(false, 1, 1, 10);
  }
 last_interrupt_time = interrupt_time;
}

void lock() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  Serial.println(interrupt_time);
  // If interrupts come faster than 500ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 500) {
   unlock_count++;
   // Transmit defcon raised.
   probe.transmit(true, 1, 1, 10);
  }
 last_interrupt_time = interrupt_time;
}

