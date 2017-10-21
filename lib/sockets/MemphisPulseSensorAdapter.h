/*
 * MemphisPulseSensorAdapter.h
 *
 *  Created on: 15.05.2016
 *      Author: niklausd
 */

#ifndef PROD_SOCKETS_MEMPHISPULSESENSORADAPTER_H_
#define PROD_SOCKETS_MEMPHISPULSESENSORADAPTER_H_

#include <PolarPulse.h>

class DbgTrace_Port;
class MemphisMatrixDisplay;

class MemphisPulseSensorAdapter: public PolarPulseAdapter
{
public:
  MemphisPulseSensorAdapter(int pulsePin, PolarPulse* polarPulse, const unsigned long int channelNumber, const char* writeAPIKey, MemphisMatrixDisplay* matrix);
  virtual ~MemphisPulseSensorAdapter();

  unsigned int getCount();
  void notifyHeartBeatRate(unsigned int* heartBeatRate, unsigned char numOfValues);
  void notifyHeartBeatRate(unsigned int heartBeatRate);

public:
  static int s_pulsePin;
  static PolarPulse* s_pulse;
private:
  DbgTrace_Port* m_trPort;
  const unsigned long int m_channelNumber;
  const char* m_writeAPIKey;
  MemphisMatrixDisplay* m_matrix;

private:  // forbidden functions
  MemphisPulseSensorAdapter();                                                  // default constructor
  MemphisPulseSensorAdapter(const MemphisPulseSensorAdapter& src);              // copy constructor
  MemphisPulseSensorAdapter& operator = (const MemphisPulseSensorAdapter& src); // assignment operator
};

#endif /* PROD_SOCKETS_MEMPHISPULSESENSORADAPTER_H_ */
