/*
 * memphis_controller.cpp
 *
 *  Created on: 19.05.2016
 *      Author: niklausd
 */

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#endif
// PlatformIO libraries
#include <PubSubClient.h>   // pio lib install 89,   lib details see https://github.com/knolleary/PubSubClient
#include <SerialCommand.h>  // pio lib install 173,  lib details see https://github.com/kroimon/Arduino-SerialCommand
#include <ThingSpeak.h>     // pio lib install 550,  lib details see https://github.com/mathworks/thingspeak-arduino
#include <ArduinoJson.h>    // pio lib install 64,   lib details see https://github.com/bblanchon/ArduinoJson
#include <Timer.h>          // pio lib install 1699, lib details see https://github.com/dniklaus/wiring-timer
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Fonts/TomThumb.h>

// private libraries
#include <DbgCliNode.h>
#include <DbgCliTopic.h>
#include <DbgTracePort.h>
#include <DbgTraceContext.h>
#include <DbgTraceOut.h>
#include <DbgPrintConsole.h>
#include <DbgTraceLevel.h>
//#include <MqttClientController.h>
//#include <PubSubClientWrapper.h>
//#include <MqttClient.h>
//#include <MqttTopic.h>
#include <string.h>
#include <AppDebug.h>
#include <ProductDebug.h>
#include <PolarPulse.h>
#include <MemphisPulseSensorAdapter.h>
#include <MemphisMatrixDisplay.h>
#include <Battery.h>

//#define MQTT_SERVER "iot.eclipse.org"
#define MQTT_SERVER "test.mosquitto.org"
//#define MQTT_SERVER "broker.hivemq.com"

SerialCommand* sCmd = 0;

#ifdef ESP8266
WiFiClient* wifiClient = 0;

//-----------------------------------------------------------------------------
// ESP8266 WatchDog sitter
//-----------------------------------------------------------------------------
class WatchDogTimerAdapter : public TimerAdapter
{
public:
  void timeExpired()
  {
    wdt_reset();
  }
};
#endif

//-----------------------------------------------------------------------------
// Battery surveillance
//-----------------------------------------------------------------------------
#define BAT_SENSE_PIN A0

class MyBatteryAdapter : public BatteryAdapter
{
private:
  Battery* m_battery;
  MemphisMatrixDisplay* m_matrix;

public:
  MyBatteryAdapter(Battery* battery, MemphisMatrixDisplay* matrix)
  : m_battery(battery)
  , m_matrix(matrix)
  { }

  virtual ~MyBatteryAdapter()
  { }

  virtual float readBattVoltageSenseFactor()
  {
    return 9.239;
  }

  virtual unsigned int readRawBattSenseValue()
  {
//    showBattVoltage();
    unsigned int rawBattSenseValue = analogRead(BAT_SENSE_PIN);
    return rawBattSenseValue;
  }

  virtual void notifyBattVoltageOk()
  {
    Serial.println("Battery Voltage OK");
    showBattVoltage();
    if (0 != m_matrix)
    {
      m_matrix->activateDisplay();
    }
  }

  virtual void notifyBattVoltageBelowWarnThreshold()
  {
    Serial.println("Battery Voltage Below Warning Threshold");
    showBattVoltage();
    if (0 != m_matrix)
    {
      m_matrix->activateDisplay();
    }
  }

  virtual void notifyBattVoltageBelowStopThreshold()
  {
    Serial.println("Battery Voltage Below Stop Threshold");
    showBattVoltage();
    if (0 != m_matrix)
    {
      m_matrix->activateDisplay();
    }
  }

  virtual void notifyBattVoltageBelowShutdownThreshold()
  {
    Serial.println("Battery Voltage Below Shutdown Threshold");
    showBattVoltage();
    if (0 != m_matrix)
    {
      m_matrix->deactivateDisplay();
    }
  }

private:
  void showBattVoltage()
  {
    float battVoltage = 0.0;
    if (0 != m_battery)
    {
      battVoltage = m_battery->getBatteryVoltage();
    }
    Serial.print("Battery Voltage: ");
    Serial.print(battVoltage);
    Serial.println(" V");
  }
};

Battery* battery = 0;
MyBatteryAdapter* batteryAdapter = 0;

//-----------------------------------------------------------------------------
// Pulse Sensor
//-----------------------------------------------------------------------------
PolarPulse* pulseSensor = 0;
#define PULSE_PIN 13
#define PULSE_IND_PIN BUILTIN_LED

//const unsigned long c_PulseMockTimerModulateInterval = 2000;
//
//class PulseMockTimerAdapter : public TimerAdapter
//{
//private:
//  MemphisMatrixDisplay* m_matrix;
//  unsigned long int m_hbr;
//  bool m_isModulationIncreasing;
//public:
//  PulseMockTimerAdapter(MemphisMatrixDisplay* matrix)
//  : m_matrix(matrix)
//  , m_hbr(60)
//  , m_isModulationIncreasing(true)
//  { }
//
//  void timeExpired()
//  {
//    if (0 != m_matrix)
//    {
//      m_matrix->setHeartBeatRate(m_hbr);
//    }
//
//    if (m_isModulationIncreasing)
//    {
//      if (m_hbr < 91)
//      {
//        m_hbr++;
//      }
//      else
//      {
//        m_isModulationIncreasing = false;
//      }
//    }
//    else
//    {
//      if (m_hbr > 62)
//      {
//        m_hbr--;
//      }
//      else
//      {
//        m_isModulationIncreasing = true;
//      }
//    }
//  }
//};

//-----------------------------------------------------------------------------
// NEO Matrix
//-----------------------------------------------------------------------------
MemphisMatrixDisplay* matrix = 0;
#define NEO_PIN 12

//-----------------------------------------------------------------------------

void setup()
{
  pinMode(PULSE_IND_PIN, OUTPUT);
  digitalWrite(PULSE_IND_PIN, 1);

  setupProdDebugEnv();

#ifdef ESP8266
  //-----------------------------------------------------------------------------
  // ESP8266 WatchDog sitter
  //-----------------------------------------------------------------------------
  new Timer(new WatchDogTimerAdapter(), Timer::IS_RECURRING, 2000);

  //-----------------------------------------------------------------------------
  // ESP8266 WiFi Client
  //-----------------------------------------------------------------------------
  wifiClient = new WiFiClient();

  //-----------------------------------------------------------------------------
  // ThingSpeak Client
  //-----------------------------------------------------------------------------
  ThingSpeak.begin(*(wifiClient));
#endif

  //-----------------------------------------------------------------------------
  // NEO Matrix
  //-----------------------------------------------------------------------------
  matrix = new MemphisMatrixDisplay(NEO_PIN);
  if (0 != matrix)
  {
    matrix->activateDisplay();
  }

  //-----------------------------------------------------------------------------
  // Pulse Sensor
  //-----------------------------------------------------------------------------
  // new Timer(new PulseMockTimerAdapter(matrix), Timer::IS_RECURRING, c_PulseMockTimerModulateInterval);
  pulseSensor = new PolarPulse(PULSE_PIN, PULSE_IND_PIN, PolarPulse::IS_POS_LOGIC);
  if (0 != pulseSensor)
  {
    pulseSensor->attachAdapter(new MemphisPulseSensorAdapter(PolarPulse::PLS_NC, pulseSensor, matrix));
  }

  //-----------------------------------------------------------------------------
  // Battery Voltage Surveillance
  //-----------------------------------------------------------------------------
  battery = new Battery();
  if (0 != battery)
  {
    batteryAdapter = new MyBatteryAdapter(battery, matrix);
    battery->attachAdapter(batteryAdapter);
  }
}

void loop()
{
  if (0 != sCmd)
  {
    sCmd->readSerial();     // process serial commands
  }
//  MqttClient.loop();        // process MQTT Client
  yield();                  // process Timers
}
