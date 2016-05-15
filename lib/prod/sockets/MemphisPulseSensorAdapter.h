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

class MemphisPulseSensorAdapter: public PolarPulseAdapter
{
public:
  MemphisPulseSensorAdapter();
  virtual ~MemphisPulseSensorAdapter();

  void notifyHeartBeatRate(unsigned int heartBeatRate);

private:
  DbgTrace_Port* m_trPort;

private:  // forbidden functions
  MemphisPulseSensorAdapter(const MemphisPulseSensorAdapter& src);              // copy constructor
  MemphisPulseSensorAdapter& operator = (const MemphisPulseSensorAdapter& src); // assignment operator
};

#endif /* PROD_SOCKETS_MEMPHISPULSESENSORADAPTER_H_ */
