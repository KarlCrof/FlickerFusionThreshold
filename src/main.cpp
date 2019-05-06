//precompiler directives
#include <Arduino.h>

//function prototypes
void ISR_button();
int myMap(int value, int xmin, int xmax, int ymin, int ymax);

//global variables
uint8_t ledPin = 0;
uint8_t buttonPin = digitalPinToInterrupt(12);
const uint32_t DEBOUNCEDELAY_MS = 200;
bool ledState = 0;
volatile bool buttonPressed = 0;

void setup() {
  Serial.begin(500000); //open serial line with baud rate 0.5MHz
  pinMode(ledPin, OUTPUT); //pin mode declaration
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(buttonPin,ISR_button,FALLING);
  //analogReadResolution(12); //note: 12bit a/dc returns 0 to 4095 value
}

void loop() {
  static uint32_t prevMicros_us = 0;
  uint32_t potValue = analogRead(A0); //read 10bit analog value from potentiometer
  int flickerFreq = myMap(potValue, 1, 1023, 1, 500); //map to freq range 1-500Hz
  float flickerDelay_us = (1000000.0/(float)flickerFreq)*0.5; //half period in micro-s

  //toggle the LED after the LED delay (set by the potentiometer)
  if ((micros() - prevMicros_us) >=  flickerDelay_us){
    prevMicros_us = micros(); //reset the LED on/off timer
    if (ledState == LOW){ //change the state of the LED
      ledState = HIGH;
    }
    else if (ledState == HIGH){
      ledState = LOW;
    }
    digitalWrite(ledPin,ledState);//output state to the LED
  }

  //button press triggers serial output displaying frequency
  if (buttonPressed){
    buttonPressed = 0; //reset button pressed flag
    //print potentiometer, freq information
    Serial.print("A/DC value = ");
    Serial.print(potValue);
    Serial.print(" Period = ");
    Serial.print(flickerDelay_us*2/1000.0);
    Serial.print("ms, Frequency = ");
    Serial.print(flickerFreq);
    Serial.println("Hz");
  }
}
void ISR_button(){
  noInterrupts();
  static uint32_t lastButtonPress_ms = 0;
  //ignore period of time after button has been pressed (debounce delay)
  if ((millis()-lastButtonPress_ms) >= DEBOUNCEDELAY_MS){
    lastButtonPress_ms = millis(); //reset the button debounce timer
    buttonPressed = 1;
  }
}

int myMap(int value, int xmin, int xmax, int ymin, int ymax){
  //generically maps the range xmin-xmax to ymin-ymax
  //returns corresponding y value for given x value
  float m = (float)(ymax-ymin) / (float)(xmax-xmin); //gradient
  float c = (float)ymin - (m * (float)xmin);//y intercept
  return round((m*value + c));
}