/*
 * MemphisPulseSensorAdapter.cpp
 *
 *  Created on: 15.05.2016
 *      Author: niklausd
 */

#include <Arduino.h>
#include <DbgTracePort.h>
#include <MemphisPulseSensorAdapter.h>
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

MemphisPulseSensorAdapter::MemphisPulseSensorAdapter(int pulsePin, PolarPulse* polarPulse, const unsigned long int channelNumber, const char* writeAPIKey, MemphisMatrixDisplay* matrix)
: m_trPort(new DbgTrace_Port("pulse", DbgTrace_Level::error))
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
  bool isWiFiConnected = false;
#ifdef ESP8266
  WiFiClient wifiClient;
  isWiFiConnected = wifiClient.connected();
#endif
  if (isWiFiConnected)
  {
    for (unsigned char i = 0; i < numOfValues; i++)
    {
      ThingSpeak.setField(i+1, static_cast<int>(heartBeatRate[i]));
    }
    ThingSpeak.writeFields(m_channelNumber, m_writeAPIKey);
    TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "Reported to ThingSpeak");
  }
  else
  {
    TR_PRINT_STR(m_trPort, DbgTrace_Level::error, "client NOT connected");
  }
}

void MemphisPulseSensorAdapter::notifyHeartBeatRate(unsigned int heartBeatRate)
{
  if (0 != m_matrix)
  {
    m_matrix->setHeartBeatRate(heartBeatRate);
  }

  char heartBeatText[40];
  sprintf(heartBeatText, "Heart Beat Rate per minute: %d", heartBeatRate);
  TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, heartBeatText);
}
