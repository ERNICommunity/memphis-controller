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
class MemphisWiFiClient;

class MemphisPulseSensorAdapter: public PolarPulseAdapter
{
public:
  MemphisPulseSensorAdapter(MemphisWiFiClient* memphisWiFiClient, const unsigned long int channelNumber, const char* writeAPIKey);
  virtual ~MemphisPulseSensorAdapter();

  void notifyHeartBeatRate(unsigned int heartBeatRate);

private:
  DbgTrace_Port* m_trPort;
  MemphisWiFiClient* m_client;
  const unsigned long int m_channelNumber;
  const char* m_writeAPIKey;

private:  // forbidden functions
  MemphisPulseSensorAdapter();                                                  // default constructor
  MemphisPulseSensorAdapter(const MemphisPulseSensorAdapter& src);              // copy constructor
  MemphisPulseSensorAdapter& operator = (const MemphisPulseSensorAdapter& src); // assignment operator
};

#endif /* PROD_SOCKETS_MEMPHISPULSESENSORADAPTER_H_ */
