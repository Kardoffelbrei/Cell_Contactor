#include <Arduino.h>
#include <HX711_ADC.h>
#include <analogWrite.h>
#include "filter.h"
#include "config.h"

HX711_ADC LoadCell(HX_DATA, HX_SCK);


volatile bool newDataReady;
double measured_force = 0;
double filtered_force = 0;

float filterBuffer[FILTERCOUNT];
int filterCounter;

void update_filter(double force){
  filterBuffer[filterCounter] = force;
  filterCounter += 1;

  if (filterCounter >= FILTERCOUNT)
  {
    filterCounter = 0; 
  }
  
  filtered_force = 0;

  for(int i=0; i < FILTERCOUNT; ++i)
  {
    filtered_force += filterBuffer[i];
  }
  filtered_force /= FILTERCOUNT;
}

void dataReadyISR()
{
  if (LoadCell.update())
  {
    newDataReady = true;
  }
}

void hx_reader_setup()
{
  LoadCell.begin();
  LoadCell.start(2000, true);

  if (LoadCell.getTareTimeoutFlag())
  {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1)
      ;
  }
  else
  {
    LoadCell.setCalFactor(HX_CALIBRATION); // set calibration value (float)
    Serial.println("Startup is complete");
  }

  attachInterrupt(digitalPinToInterrupt(HX_DATA), dataReadyISR, FALLING);
  pinMode(ANALOG_FORCE_OUTPUT, OUTPUT);
}

void hx_tara()
{
  LoadCell.tareNoDelay();
}

double get_force()
{
  return measured_force;
}

double get_filtered_force(){
  return filtered_force;
}

bool contact_detected()
{
  return measured_force > CONTACT_FORCE;
}

void hx_reader_loop()
{

  if (newDataReady)
  {
    measured_force = LoadCell.getData() * 0.009807; // convert to newton
    update_filter(measured_force);
    newDataReady = false;
    // Serial.println(measured_force, 5);
    // write voltage proportional to force to GPIO
    // 0 - 20 N -> 0 - 2000 -> 0 - 255 PWM duty cycle
    analogWrite(ANALOG_FORCE_OUTPUT, constrain(map((measured_force * 100), 0, 2000, 0, 255), 0, 255));
  }

  // check if last tare operation is complete
  if (LoadCell.getTareStatus() == true)
  {
    //Serial.println("Tare complete");
  }
}