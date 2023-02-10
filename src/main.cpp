#include <Arduino.h>
#include "reflow_oven.h"

#define HEATER_PIN 3
#define THERMOCOUPLE_DO_PIN  4
#define THERMOCOUPLE_CS_PIN  5
#define THERMOCOUPLE_CLK_PIN 6

#define HYSTERESIS 2

#define RFO_UPDATE_INTERVAL 700
#define RFO_LOGGING_INTERVAL 250
unsigned long rfo_last_logged;
unsigned long mills;

static const int reflow_profile[][2] = {
/*   °C       ms   */
  { 180,     10000 },
};


Reflow_Oven rfo(HEATER_PIN, THERMOCOUPLE_CLK_PIN, THERMOCOUPLE_CS_PIN, THERMOCOUPLE_DO_PIN,
                reflow_profile, sizeof(reflow_profile) / sizeof(reflow_profile[0]), HYSTERESIS, RFO_UPDATE_INTERVAL);

void setup()
{
  Serial.begin(115200);
  while (!Serial) {}

  mills = rfo_last_logged =  millis();
}

void loop()
{
  mills = millis();

  if (Serial.available()) {
    String cmd = Serial.readString();

    if (cmd == "start")
      rfo.begin();
    if (cmd == "stop")
      rfo.stop();
  }

  if (mills - rfo_last_logged >= RFO_LOGGING_INTERVAL) {
    rfo_last_logged = mills;
    rfo.log();
  }

  if (rfo.is_finished()) return;

  rfo.update();
}