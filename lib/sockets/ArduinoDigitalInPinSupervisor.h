/*
 * ArduinoDigitalInPinSupervisor.h
 *
 *  Created on: 25.01.2018
 *      Author: niklaudi
 */

#ifndef SRC_ARDUINODIGITALINPINSUPERVISOR_H_
#define SRC_ARDUINODIGITALINPINSUPERVISOR_H_

#include <Button.h>

class ArduinoDigitalInPinSupervisor: public PinSupervisor
{
public:
  ArduinoDigitalInPinSupervisor(int arduinoPin);
  virtual ~ArduinoDigitalInPinSupervisor();

  bool getPinState();

private:
  int m_arduinoPin;

private:  // forbidden functions
  ArduinoDigitalInPinSupervisor();                                                      // default constructor
  ArduinoDigitalInPinSupervisor(const ArduinoDigitalInPinSupervisor& src);              // copy constructor
  ArduinoDigitalInPinSupervisor& operator = (const ArduinoDigitalInPinSupervisor& src); // assignment operator
};

#endif /* SRC_ARDUINODIGITALINPINSUPERVISOR_H_ */
