/*
 * MemphisPulseSensorAdapter.cpp
 *
 *  Created on: 15.05.2016
 *      Author: niklausd
 */

#include <Arduino.h>
#include <DbgTracePort.h>
#include <MemphisPulseSensorAdapter.h>

MemphisPulseSensorAdapter::MemphisPulseSensorAdapter()
: m_trPort(new DbgTrace_Port("pulse", "trConOut", DbgTrace_Level::debug))
{ }

MemphisPulseSensorAdapter::~MemphisPulseSensorAdapter()
{ }

void MemphisPulseSensorAdapter::notifyHeartBeatRate(unsigned int heartBeatRate)
{
  char heartBeatText[40];
  sprintf(heartBeatText, "Heart Beat Rate per minute: %d",heartBeatRate);
  TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, heartBeatText);
}
