
#include <ESP32Servo.h>
Servo servo;

int mini = 100;
int maxi = 100;
unsigned long prevMillis = 0;
const long interval = 1000; // Minimal delay time between servo changes
int flip = 0;               // For testing, flip between min/max values per servo change

void setup()
{
  // initialize serial:
  Serial.begin(115200); // set serial monitor baud rate to match
  servo.write(0);
  servo.attach(12, 544, 2400);
}

void loop()
{
  if (Serial.available() > 0)
  {

    maxi = Serial.parseInt();
    mini = Serial.parseInt();

    // Look for the newline/carriage return character to clear the buffer
    if (Serial.read() == '\n' || Serial.read() == '\r')
    {
      // Data fully read
    }

    if (maxi != 0)
    {
      Serial.print("Maxi: ");
      Serial.println(maxi);
    }

    Serial.print("Mini: ");
    Serial.println(mini);
  }

  unsigned long currentMillis = millis();
  if ((currentMillis - prevMillis) > interval)
  {
    prevMillis = millis();

    flip = !flip; // Flip is just for this test

    if (flip == 0)
      servo.write(mini);
    // delay(1000);
    if (flip == 1)
      servo.write(maxi);
    // delay(1000);
  }
}
