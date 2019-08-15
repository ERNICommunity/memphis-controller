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
// PlatformIO libraries
#include <PubSubClient.h>   // pio lib install 89,   lib details see https://github.com/knolleary/PubSubClient
#include <ThingSpeak.h>     // pio lib install 550,  lib details see https://github.com/mathworks/thingspeak-arduino
#endif
// PlatformIO libraries
#include <SerialCommand.h>  // pio lib install 173,  lib details see https://github.com/kroimon/Arduino-SerialCommand
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
#include <string.h>
#include <AppDebug.h>
#include <ProductDebug.h>
#include <PolarPulse.h>
#include <MemphisPulseSensorAdapter.h>
#include <MemphisMatrixDisplay.h>
#include <Battery.h>
#include <ToggleButton.h>
#include <CmdSequence.h>

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
// Push Button
//-----------------------------------------------------------------------------
#define BUTTON_PIN 11
ToggleButton* button = 0;

class MyToggleButtonAdapter : public ToggleButtonAdapter
{
private:
  MemphisMatrixDisplay* m_matrix;

public:
  MyToggleButtonAdapter(MemphisMatrixDisplay* matrix)
  : m_matrix(matrix)
  { }

  void notifyStatusChanged(bool isActive)
  {
    if (0 != m_matrix)
    {
      if (m_matrix->imageSequence()->isRunning())
      {
        m_matrix->imageSequence()->stop();
      }
      else
      {
        m_matrix->imageSequence()->start();
      }
    }
  }
};

//-----------------------------------------------------------------------------
// Battery surveillance
//-----------------------------------------------------------------------------
#if defined (ARDUINO_ARCH_SAMD) && defined (__SAMD21G18A__) // Adafruit Feather M0
  #define BAT_SENSE_PIN A7
#else
  #define BAT_SENSE_PIN A0
#endif

class MyBatteryAdapter : public BatteryAdapter
{
private:
  MemphisMatrixDisplay* m_matrix;

public:
  static const BatteryThresholdConfig s_battCfg;

public:
  MyBatteryAdapter(MemphisMatrixDisplay* matrix)
  : m_matrix(matrix)
  { }

  virtual ~MyBatteryAdapter()
  { }

  virtual float readBattVoltageSenseFactor()
  {
    return 2.0;
  }

  virtual unsigned int readRawBattSenseValue()
  {
    unsigned int rawBattSenseValue = analogRead(BAT_SENSE_PIN);
    return rawBattSenseValue;
  }

  virtual void notifyBattVoltageOk()
  {
    Serial.println("Battery Voltage OK");
    showBattVoltage();
    if (0 != m_matrix)
    {
//      m_matrix->activateDisplay();
    }
  }

  virtual void notifyBattVoltageBelowWarnThreshold()
  {
    Serial.println("Battery Voltage Below Warning Threshold");
    showBattVoltage();
    if (0 != m_matrix)
    {
    }
  }

  virtual void notifyBattVoltageBelowStopThreshold()
  {
    Serial.println("Battery Voltage Below Stop Threshold");
    showBattVoltage();
    if (0 != m_matrix)
    {
    }
  }

  virtual void notifyBattVoltageBelowShutdownThreshold()
  {
    Serial.println("Battery Voltage Below Shutdown Threshold");
    showBattVoltage();
    if (0 != m_matrix)
    {
    }
  }

private:
  void showBattVoltage()
  {
    float battVoltage = 0.0;
    if (0 != battery())
    {
      battVoltage = battery()->getBatteryVoltage();
    }
    Serial.print("Battery Voltage: ");
    Serial.print(battVoltage);
    Serial.println(" V");
  }
};

const BatteryThresholdConfig MyBatteryAdapter::s_battCfg = { 3.6, // BATT_WARN_THRSHD [V]
                                                             3.4, // BATT_STOP_THRSHD [V]
                                                             3.2, // BATT_SHUT_THRSHD [V]
                                                             0.1  // BATT_HYST        [V]
                                                           };
Battery* battery = 0;

//-----------------------------------------------------------------------------
// Pulse Sensor
//-----------------------------------------------------------------------------
PolarPulse* pulseSensor = 0;
#define PULSE_PIN 13
#define PULSE_IND_PIN LED_BUILTIN

const unsigned long c_PulseMockTimerModulateInterval = 2000;

class PulseMockTimerAdapter : public TimerAdapter
{
private:
  MemphisMatrixDisplay* m_matrix;
  unsigned long int m_hbr;
  const static unsigned long int c_hbr = 20;
  const static unsigned long int c_minHbr = c_hbr;
  const static unsigned long int c_maxHbr = c_hbr;
  bool m_isModulationIncreasing;

public:
  PulseMockTimerAdapter(MemphisMatrixDisplay* matrix)
  : m_matrix(matrix)
  , m_hbr(c_hbr)
  , m_isModulationIncreasing(true)
  { }

  void timeExpired()
  {
    if (0 != m_matrix)
    {
      m_matrix->setHeartBeatRate(m_hbr);
//      if (0 != battery)
//      {
//        unsigned int battV = static_cast<unsigned int>(10 * battery->getBatteryVoltage());
//        m_matrix->setHeartBeatRate(battV);
//      }
    }

    if (m_isModulationIncreasing)
    {
      if (m_hbr < c_maxHbr)
      {
        m_hbr++;
      }
      else
      {
        m_isModulationIncreasing = false;
      }
    }
    else
    {
      if (m_hbr > c_minHbr)
      {
        m_hbr--;
      }
      else
      {
        m_isModulationIncreasing = true;
      }
    }
  }
};

//-----------------------------------------------------------------------------
// NEO Matrix
//-----------------------------------------------------------------------------
MemphisMatrixDisplay* matrix = 0;
#define NEO_PIN 12

//-----------------------------------------------------------------------------

void setup()
{
  pinMode(PULSE_IND_PIN, OUTPUT);
  digitalWrite(PULSE_IND_PIN, 0);

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
    matrix->deactivateDisplay();
  }

  //-----------------------------------------------------------------------------
  // Push Button
  //-----------------------------------------------------------------------------
  button = new ToggleButton(BUTTON_PIN, ToggleButton::IND_NC, ToggleButton::IS_NEG_LOGIC, new MyToggleButtonAdapter(matrix));

  //-----------------------------------------------------------------------------
  // Pulse Sensor
  //-----------------------------------------------------------------------------
   new Timer(new PulseMockTimerAdapter(matrix), Timer::IS_RECURRING, c_PulseMockTimerModulateInterval);
//  pulseSensor = new PolarPulse(PULSE_PIN, PULSE_IND_PIN, PolarPulse::IS_POS_LOGIC);
//  if (0 != pulseSensor)
//  {
//    pulseSensor->attachAdapter(new MemphisPulseSensorAdapter(PolarPulse::PLS_NC, pulseSensor, matrix));
//  }

  //-----------------------------------------------------------------------------
  // Battery Voltage Surveillance
  //-----------------------------------------------------------------------------
  battery = new Battery(new MyBatteryAdapter(matrix), MyBatteryAdapter::s_battCfg);
}

void loop()
{
  if (0 != sCmd)
  {
    sCmd->readSerial();     // process serial commands
  }
//  MqttClient.loop();      // process MQTT Client
#ifdef ESP8266
  scheduleTimers();
#else
  yield();
#endif
}
