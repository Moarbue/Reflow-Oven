#include "reflow_oven.h"

#include <assert.h>

static unsigned long mills;

Reflow_Oven::Reflow_Oven(uint8_t heater_pin,            uint8_t thermocouple_clk_pin,
                         uint8_t thermocouple_cs_pin,   uint8_t thermocouple_do_pin,
                         const int reflow_profile[][2], size_t reflow_profile_entries,
                         uint8_t hysteresis,            size_t update_intervall)
{
    assert(reflow_profile_entries <= REFLOW_PROFILE_MAX_ENTRIES);

    pinMode(heater_pin, OUTPUT);

    this->heater_pin             = heater_pin;
    this->tc                     = new MAX6675(thermocouple_clk_pin, thermocouple_cs_pin, thermocouple_do_pin);
    this->t_current              = 0.f;
    this->reflow_profile_entries = reflow_profile_entries;
    this->reflow_profile_index   = reflow_profile_entries;
    this->hysteresis             = hysteresis;
    this->last_mills             = millis();
    this->last_updated           = millis();
    this->update_intervall       = update_intervall;
    this->regulate               = false;
    memcpy(this->reflow_profile, reflow_profile, sizeof(reflow_profile[0]) * reflow_profile_entries);

    this->initialized = true;
}


void Reflow_Oven::begin()
{
    if (!initialized) return;

    reflow_profile_index = 0;
}

bool Reflow_Oven::update()
{
    if (!initialized) return false;
    if (is_finished())
        return false;

    int t_desired, t_hold;

    mills = millis();

    if (mills - last_updated < update_intervall) return true;
    update_intervall = mills;

    t_current = tc->readCelsius();
    if (t_current == NAN)
        return false;
    
    t_desired = reflow_profile[reflow_profile_index][0];
    t_hold    = reflow_profile[reflow_profile_index][1];

    if (!regulate && t_current > (t_desired - hysteresis) && t_current < (t_desired + hysteresis)) {
        regulate = true;
        last_mills = mills;
    }

    if (t_current > (t_desired + hysteresis)) {
        heater_off();
        return true;
    }
    else if (t_current < (t_desired - hysteresis)) {
        heater_on();
        return true;
    }

    if (regulate && (mills - last_mills >= (unsigned long)t_hold)) {
        reflow_profile_index++;
        regulate = false;
    }

    return true;
}

void Reflow_Oven::stop()
{
    if (!initialized) return;

    reflow_profile_index = reflow_profile_entries;
}

bool Reflow_Oven::is_finished()
{
    if (!initialized) return true;

    if (reflow_profile_index >= reflow_profile_entries)
        return true;

    return false;
}

void Reflow_Oven::log()
{
    if (is_finished())
        Serial.println("\r\nState = OFF");
    else {
        Serial.println("\r\nState = ON");
        Serial.print  ("Tcur  = ");
        Serial.print  (t_current);
        Serial.println("°C");
        Serial.print  ("Tdes  = ");
        Serial.print  (reflow_profile[reflow_profile_index][0]);
        Serial.println("°C");
        Serial.print  ("Thold = ");
        Serial.print  (reflow_profile[reflow_profile_index][1]);
        Serial.println("ms");
        Serial.print  ("Trem  = ");
        if (regulate)
            Serial.print  (reflow_profile[reflow_profile_index][1] - (mills - last_mills));
        else
            Serial.print  (reflow_profile[reflow_profile_index][1]);
        Serial.println("ms");
    }
}

void Reflow_Oven::heater_off()
{
    digitalWrite(heater_pin, LOW);
}

void Reflow_Oven::heater_on()
{
    digitalWrite(heater_pin, HIGH);
}