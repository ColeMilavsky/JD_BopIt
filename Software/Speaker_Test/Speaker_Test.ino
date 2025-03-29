#include <DFRobotDFPlayerMini.h>

// Use the default hardware serial for communication with DFPlayer Mini
#define RED_LED 18
#define GREEN_LED 19

DFRobotDFPlayerMini myDFPlayer;

void setup() {
  // Start the hardware serial port (Serial) for communication with DFPlayer Mini
  //Serial.begin(115200);
  Serial.begin(9600);

  //myDFPlayer.begin(Serial);

  // Configure the LEDs
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Initialize the DFPlayer Mini with the hardware serial
  if (!myDFPlayer.begin(Serial)) {
    while(true) {
      digitalWrite(RED_LED, HIGH);
      delay(1000);
      digitalWrite(RED_LED, LOW);
      delay(1000);
    }
  }

  // Set the volume (0-30) and play the first file
  myDFPlayer.volume(20);
  myDFPlayer.play(1);
}

void loop() {
  // Blink the green LED
  digitalWrite(GREEN_LED, HIGH);
  delay(1000);
  digitalWrite(GREEN_LED, LOW);
  delay(1000);

  // Set the volume and play the first file continuously
  delay(1000);
  myDFPlayer.volume(20);
  myDFPlayer.play(1);

  delay(1000);
  myDFPlayer.volume(20);
  myDFPlayer.play(2);

  delay(1000);
  myDFPlayer.volume(20);
  myDFPlayer.play(3);
}