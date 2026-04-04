int fetPin = 3;

void setup() {                
  pinMode(fetPin, OUTPUT); 
}

// the loop routine runs over and over again forever:
void loop() {
      analogWrite(fetPin, 85); //33% duty cycle
}
