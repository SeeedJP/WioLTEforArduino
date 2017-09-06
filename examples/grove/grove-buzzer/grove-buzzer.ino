#include <WioLTEforArduino.h>

#define BUZZER_PIN      (WIOLTE_D38)
#define FREQUENCY       (1000)
#define BUZZER_ON_TIME  (1000)
#define BUZZER_OFF_TIME (3000)

int Period;

void setup()
{
  pinMode(BUZZER_PIN, OUTPUT);

  Period = 1e6 / FREQUENCY;
}

void loop()
{
  unsigned long start = millis();
  while (millis() < start + BUZZER_ON_TIME) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(Period / 2);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(Period / 2);
  }

  digitalWrite(BUZZER_PIN, LOW);
  delay(BUZZER_OFF_TIME);
}

