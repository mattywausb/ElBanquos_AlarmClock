#define BUTTON_PIN 2
#define MOSFET_GATE_PIN 10


void setup() {
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  pinMode(MOSFET_GATE_PIN,OUTPUT);
  digitalWrite(MOSFET_GATE_PIN,LOW);

}

void loop() {
  if(digitalRead(BUTTON_PIN)==LOW){
    digitalWrite(MOSFET_GATE_PIN,HIGH);
    delay(1500);
    digitalWrite(MOSFET_GATE_PIN,LOW);
  }
}
