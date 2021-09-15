#include <WioLTEforArduino.h>

// You can use WIOLTE_(D20|A4|A6) with `Wio.PowerSupplyGrove(true);`
#define BUTTON_PIN  (WIOLTE_D38)

#define COLOR_ON	127, 127, 127
#define COLOR_OFF	  0,   0,   0

WioCellular Wio;

volatile bool StateChanged = false;
volatile bool State = false;

void change_state() {
  State = !State;
  StateChanged = true;
}

void setup() {
  SerialUSB.begin(115200);
  Wio.Init();

  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(BUTTON_PIN, change_state, RISING);
}

void loop() {
  if (StateChanged) {
    SerialUSB.print(State ? '*' : '.');

    if (State) {
      Wio.LedSetRGB(COLOR_ON);
    }
    else {
      Wio.LedSetRGB(COLOR_OFF);
    }

	StateChanged = false;
  }
}
