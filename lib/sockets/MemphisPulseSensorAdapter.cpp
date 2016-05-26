/*
 * MemphisPulseSensorAdapter.cpp
 *
 *  Created on: 15.05.2016
 *      Author: niklausd
 */

#include <Arduino.h>
#include <DbgTracePort.h>
#include <MemphisPulseSensorAdapter.h>
#include <MemphisWiFiClient.h>
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

MemphisPulseSensorAdapter::MemphisPulseSensorAdapter(MemphisWiFiClient* memphisWiFiClient, const unsigned long int channelNumber, const char* writeAPIKey)
: m_trPort(new DbgTrace_Port("pulse", DbgTrace_Level::debug))
, m_client(memphisWiFiClient)
, m_channelNumber(channelNumber)
, m_writeAPIKey(writeAPIKey)
{ }

MemphisPulseSensorAdapter::~MemphisPulseSensorAdapter()
{ }

void MemphisPulseSensorAdapter::notifyHeartBeatRate(unsigned int heartBeatRate)
{
  if (0 != m_client)
  {
    if (m_client->isConnected())
    {
      ThingSpeak.writeField(m_channelNumber, 1, static_cast<int>(heartBeatRate), m_writeAPIKey);
    }
    else
    {
      TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "client NOT connected");
    }
  }

  char heartBeatText[40];
  sprintf(heartBeatText, "Heart Beat Rate per minute: %d",heartBeatRate);
  TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, heartBeatText);
}
