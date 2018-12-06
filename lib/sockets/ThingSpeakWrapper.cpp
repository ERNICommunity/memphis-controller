/*
 * ThingSpeakWrapper.cpp
 *
 *  Created on: 11.02.2018
 *      Author: dniklaus
 */

#include <ESP8266WiFi.h>
#include <ESP.h>
#include <ThingSpeak.h>
#include <ConnectivitySecrets.h>
#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include <Timer.h>

#include <ThingSpeakWrapper.h>

class WriteFieldsTimerAdapter : public TimerAdapter
{
private:
  ThingSpeakWrapper* m_thingSpeakWrapper;

public:
  WriteFieldsTimerAdapter(ThingSpeakWrapper* thingSpeakWrapper)
  : m_thingSpeakWrapper(thingSpeakWrapper)
  { }

  void timeExpired()
  {
    if (0 != m_thingSpeakWrapper)
    {
      m_thingSpeakWrapper->checkWriteFields();
    }
  }

private:  // forbidden functions
  WriteFieldsTimerAdapter();                                                // default constructor
  WriteFieldsTimerAdapter(const WriteFieldsTimerAdapter& src);              // copy constructor
  WriteFieldsTimerAdapter& operator = (const WriteFieldsTimerAdapter& src); // assignment operator
};

//-----------------------------------------------------------------------------

ThingSpeakWrapper::ThingSpeakWrapper()
: m_trPort(new DbgTrace_Port("thngwr", DbgTrace_Level::debug))
, m_writeFieldsTimer(new Timer(new WriteFieldsTimerAdapter(this), Timer::IS_RECURRING, 500))
, m_channelNumber(cMyChannelNumber)
, m_writeAPIKey(cMyWriteAPIKey)
, m_writeFieldsFlag(false)
{ }

ThingSpeakWrapper::~ThingSpeakWrapper()
{
  delete m_writeFieldsTimer->adapter();
  m_writeFieldsTimer->attachAdapter(0);

  delete m_writeFieldsTimer;
  m_writeFieldsTimer = 0;
}

void ThingSpeakWrapper::triggerWriteFields()
{
  m_writeFieldsFlag = true;
  TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "Fields to be sent to ThingSpeak ready.")
}

void ThingSpeakWrapper::checkWriteFields()
{
  bool isLanConnected = false;
#ifdef ESP8266
  isLanConnected = WiFi.isConnected();
#endif
  if (m_writeFieldsFlag)
  {
    if (isLanConnected)
    {
      m_writeFieldsFlag = false;
      ThingSpeak.writeFields(m_channelNumber, m_writeAPIKey);
      wdt_reset();
      TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "Fields reported to ThingSpeak");
    }
    else
    {
      TR_PRINT_STR(m_trPort, DbgTrace_Level::error, "WiFi client NOT connected");
    }
  }
}
