/*
 * ThingSpeakWrapper.h
 *
 *  Created on: 11.02.2018
 *      Author: niklaudi
 */

#ifndef LIB_SOCKETS_THINGSPEAKWRAPPER_H_
#define LIB_SOCKETS_THINGSPEAKWRAPPER_H_

class Timer;

class ThingSpeakWrapper
{
public:
  ThingSpeakWrapper();
  virtual ~ThingSpeakWrapper();

  void triggerWriteFields();

  void checkWriteFields();

private:
  DbgTrace_Port* m_trPort;
//  WiFiClient* m_wifiClient;
  Timer* m_writeFieldsTimer;
  const unsigned long int m_channelNumber;
  const char* m_writeAPIKey;
  bool m_writeFieldsFlag;

private:  // forbidden functions
  ThingSpeakWrapper(const ThingSpeakWrapper& src);              // copy constructor
  ThingSpeakWrapper& operator = (const ThingSpeakWrapper& src); // assignment operator
};

#endif /* LIB_SOCKETS_THINGSPEAKWRAPPER_H_ */
