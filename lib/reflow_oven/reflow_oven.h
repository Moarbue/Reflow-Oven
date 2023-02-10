#ifndef _REFLOW_OVEN_H_
#define _REFLOW_OVEN_H_

#include <Arduino.h>
#include <max6675.h>

#define REFLOW_PROFILE_MAX_ENTRIES 50

class Reflow_Oven {

    private:
    uint8_t heater_pin;
    uint8_t hysteresis;

    MAX6675 *tc;
    float t_current;

    int reflow_profile[REFLOW_PROFILE_MAX_ENTRIES][2];
    size_t reflow_profile_entries;
    size_t reflow_profile_index;

    unsigned long last_mills;
    unsigned long last_updated;
    size_t update_intervall;

    bool regulate;
    bool initialized;

    void heater_on();
    void heater_off();

    public:
    Reflow_Oven(uint8_t heater_pin,            uint8_t thermocouple_clk_pin,
                uint8_t thermocouple_cs_pin,   uint8_t thermocouple_do_pin,
                const int reflow_profile[][2], size_t reflow_profile_entries,
                uint8_t hysteresis = 2,        size_t update_intervall = 1000);
    
    void begin();
    bool update();
    void stop();
    bool is_finished();
    void log();
};

#endif // _REFLOW_OVEN_H_