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
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#include "ThingSpeak.h"
#include <MemphisMatrixDisplay.h>
#include <String.h>
#include <PolarPulse.h>

//-----------------------------------------------------------------------------
// Pulse Pin ISR
//-----------------------------------------------------------------------------
volatile unsigned int pulseIsrCount = 0;
void pulseRIsr()
{
  noInterrupts();
  bool pulseState = digitalRead(MemphisPulseSensorAdapter::s_pulsePin);
  if (pulseState)
  {
    // rising edge
    pulseIsrCount++;
//    Serial.print("ISR, pulseIsrCount: ");
//    Serial.println (pulseIsrCount);
  }
  if (0 != MemphisPulseSensorAdapter::s_pulse)
  {
    // update LED indicator state
    MemphisPulseSensorAdapter::s_pulse->setIndicator(pulseState);
  }
  interrupts();
}

//-----------------------------------------------------------------------------

int MemphisPulseSensorAdapter::s_pulsePin = PolarPulse::PLS_NC;
PolarPulse* MemphisPulseSensorAdapter::s_pulse = 0;

MemphisPulseSensorAdapter::MemphisPulseSensorAdapter(int pulsePin, PolarPulse* polarPulse, MemphisWiFiClient* memphisWiFiClient, const unsigned long int channelNumber, const char* writeAPIKey, MemphisMatrixDisplay* matrix)
: m_trPort(new DbgTrace_Port("pulse", DbgTrace_Level::debug))
, m_client(memphisWiFiClient)
, m_channelNumber(channelNumber)
, m_writeAPIKey(writeAPIKey)
, m_matrix(matrix)
{
  s_pulsePin = pulsePin;
  s_pulse = polarPulse;
  if ((0 != s_pulse) && (PolarPulse::PLS_NC < s_pulsePin))
  {
    pinMode(s_pulsePin, INPUT);
    attachInterrupt(s_pulsePin, pulseRIsr, CHANGE);
  }
}

MemphisPulseSensorAdapter::~MemphisPulseSensorAdapter()
{
  detachInterrupt(s_pulsePin);

  delete m_trPort;
  m_trPort = 0;
}

unsigned int MemphisPulseSensorAdapter::getCount()
{
  unsigned int count = 0;
  noInterrupts();
  count = pulseIsrCount;
  pulseIsrCount = 0;
  interrupts();
  return count;
}

void MemphisPulseSensorAdapter::notifyHeartBeatRate(unsigned int* heartBeatRate, unsigned char numOfValues)
{
  if (0 != m_client)
  {
    if (m_client->isConnected())
    {
      for (unsigned char i = 0; i < numOfValues; i++)
      {
        ThingSpeak.setField(i+1, static_cast<int>(heartBeatRate[i]));
      }
      ThingSpeak.writeFields(m_channelNumber, m_writeAPIKey);
      Serial.println("notifyHeartBeatRate(): Reported to ThingSpeak");
      // TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "Reported to ThingSpeak");
    }
    else
    {
      Serial.println("notifyHeartBeatRate(): client NOT connected");
      // TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "client NOT connected");
    }
  }
}

void MemphisPulseSensorAdapter::notifyHeartBeatRate(unsigned int heartBeatRate)
{
  if (0 != m_matrix)
  {
    m_matrix->setHeartBeatRate(heartBeatRate);
  }

  char heartBeatText[40];
  Serial.print("Heart Beat Rate per minute: ");
  Serial.println(heartBeatRate);
//  TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, heartBeatText);
}
