#include "Arduino.h"

// PlatformIO libraries
#include <SerialCommand.h>
//#include <PubSubClient.h>
//#include <ThingSpeak.h>
//#include <Client.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// private libraries
#include <Timer.h>
//#include <DbgCliNode.h>
//#include <DbgCliTopic.h>
//#include <DbgTracePort.h>
//#include <DbgTraceContext.h>
//#include <DbgTraceOut.h>
//#include <DbgPrintConsole.h>
//#include <DbgTraceLevel.h>
//#include <MemphisWiFiClient.h>
#include <PolarPulse.h>
#include <MemphisPulseSensorAdapter.h>
#include <MemphisMatrixDisplay.h>
//#include <Battery.h>
//#include <MqttClient.h>
#include <ConnectivitySecrets.h>

//-----------------------------------------------------------------------------
// WiFi Client
//-----------------------------------------------------------------------------
#ifdef ESP8266
//MemphisWiFiClient* wifiClient = 0;
//MqttClient* mqttClient = 0;
#define MQTT_SERVER_IP  "iot.eclipse.org"
#define MQTT_PORT       1883
#endif

//-----------------------------------------------------------------------------
// Arduino Cmd I/F
//-----------------------------------------------------------------------------
SerialCommand* sCmd = 0;

//void dbgCliExecute()
//{
//  if ((0 != sCmd) && (0 != DbgCli_Node::RootNode()))
//  {
//    const unsigned int firstArgToHandle = 1;
//    const unsigned int maxArgCnt = 10;
//    char* args[maxArgCnt];
//    char* arg = const_cast<char*>("dbg");
//    unsigned int arg_cnt = 0;
//    while ((maxArgCnt > arg_cnt) && (0 != arg))
//    {
//      args[arg_cnt] = arg;
//      arg = sCmd->next();
//      arg_cnt++;
//    }
//    DbgCli_Node::RootNode()->execute(static_cast<unsigned int>(arg_cnt), const_cast<const char**>(args), firstArgToHandle);
//  }
//}

void sayHello()
{
  char *arg;
  if (0 != sCmd)
  {
    arg = sCmd->next();    // Get the next argument from the SerialCommand object buffer
  }
  else
  {
    arg = const_cast<char*>("");;
  }
  if (arg != NULL)         // As long as it exists, take it
  {
    Serial.print("Hello ");
    Serial.println(arg);
  }
  else
  {
    Serial.println("Hello, whoever you are");
  }
}

// This is the default handler, and gets called when no other command matches.
void unrecognized(const char *command)
{
  Serial.println("What?");
}

//-----------------------------------------------------------------------------
// Battery Surveillance
//-----------------------------------------------------------------------------
#define BAT_SENSE_PIN A0

//class MyBatteryAdapter : public BatteryAdapter
//{
//private:
//  Battery* m_battery;
//  MemphisMatrixDisplay* m_matrix;
//
//public:
//  MyBatteryAdapter(Battery* battery, MemphisMatrixDisplay* matrix)
//  : m_battery(battery)
//  , m_matrix(matrix)
//  { }
//
//  virtual float readBattVoltageSenseFactor()
//  {
//    return 9.239;
//  }
//
//  virtual unsigned int readRawBattSenseValue()
//  {
////    showBattVoltage();
//    unsigned int rawBattSenseValue = analogRead(BAT_SENSE_PIN);
//    return rawBattSenseValue;
//  }
//
//  virtual void notifyBattVoltageOk()
//  {
//    Serial.println("Battery Voltage OK");
//    showBattVoltage();
//    if (0 != m_matrix)
//    {
//      m_matrix->activateDisplay();
//    }
//  }
//
//  virtual void notifyBattVoltageBelowWarnThreshold()
//  {
//    Serial.println("Battery Voltage Below Warning Threshold");
//    showBattVoltage();
//    if (0 != m_matrix)
//    {
//      m_matrix->activateDisplay();
//    }
//  }
//
//  virtual void notifyBattVoltageBelowStopThreshold()
//  {
//    Serial.println("Battery Voltage Below Stop Threshold");
//    showBattVoltage();
//    if (0 != m_matrix)
//    {
//      m_matrix->activateDisplay();
//    }
//  }
//
//  virtual void notifyBattVoltageBelowShutdownThreshold()
//  {
//    Serial.println("Battery Voltage Below Shutdown Threshold");
//    showBattVoltage();
//    if (0 != m_matrix)
//    {
//      m_matrix->deactivateDisplay();
//    }
//  }
//
//private:
//  void showBattVoltage()
//  {
//    float battVoltage = 0.0;
//    if (0 != m_battery)
//    {
//      battVoltage = m_battery->getBatteryVoltage();
//    }
//    Serial.print("Battery Voltage: ");
//    Serial.print(battVoltage);
//    Serial.println(" V");
//  }
//};
//
//Battery* battery = 0;
//MyBatteryAdapter* batteryAdapter = 0;

//-----------------------------------------------------------------------------
// Free Heap Logger
//-----------------------------------------------------------------------------
const unsigned long c_freeHeapLogIntervalMillis = 10000;

#ifdef ESP8266
extern "C"
{
  #include "user_interface.h"
}
class FreeHeapLogTimerAdapter : public TimerAdapter
{
//private:
//  DbgTrace_Port* m_trPort;
public:
  FreeHeapLogTimerAdapter()
//  : m_trPort(new DbgTrace_Port("heap", DbgTrace_Level::info))
  { }

  void timeExpired()
  {
    Serial.print("Free Heap size: ");
    Serial.println(system_get_free_heap_size());
//    TR_PRINT_LONG(m_trPort, DbgTrace_Level::debug, system_get_free_heap_size());
  }
};
#else
class FreeHeapLogTimerAdapter : public TimerAdapter
{
//private:
//  DbgTrace_Port* m_trPort;
public:
  FreeHeapLogTimerAdapter()
//  : m_trPort(new DbgTrace_Port("heap", DbgTrace_Level::info))
  { }

  void timeExpired()
  {
    Serial.print("Free Heap size: ");
    Serial.println(RamUtils::getFreeRam());
//    TR_PRINT_LONG(m_trPort, DbgTrace_Level::debug, system_get_free_heap_size());
  }
};
#endif

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
  bool m_isModulationIncreasing;
public:
  PulseMockTimerAdapter(MemphisMatrixDisplay* matrix)
  : m_matrix(matrix)
  , m_hbr(60)
  , m_isModulationIncreasing(true)
  { }

  void timeExpired()
  {
    if (0 != m_matrix)
    {
      m_matrix->setHeartBeatRate(m_hbr);
    }

    if (m_isModulationIncreasing)
    {
      if (m_hbr < 91)
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
      if (m_hbr > 62)
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
  //-----------------------------------------------------------------------------
  // Serial Command Object for Debug CLI
  //-----------------------------------------------------------------------------
  Serial.begin(115200);
  sCmd = new SerialCommand();
//  DbgCli_Node::AssignRootNode(new DbgCli_Topic(0, "dbg", "Workforce2020 Controller Debug CLI Root Node."));

  // Setup callbacks for SerialCommand commands
  if (0 != sCmd)
  {
//    sCmd->addCommand("dbg", dbgCliExecute);
    sCmd->addCommand("hello", sayHello);        // Echos the string argument back
    sCmd->setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")
  }

  //---------------------------------------------------------------------------
  // Debug Trace
  //---------------------------------------------------------------------------
//  new DbgTrace_Context(new DbgCli_Topic(DbgCli_Node::RootNode(), "tr", "Modify debug trace"));
//  new DbgTrace_Out(DbgTrace_Context::getContext(), "trConOut", new DbgPrint_Console());

  Serial.println();
  Serial.println(F("---------------------------------------------"));
  Serial.println(F("Hello from Memphis Controller!"));
  Serial.println(F("---------------------------------------------"));
  Serial.println();

  //-----------------------------------------------------------------------------
  // Free Heap Logger
  //-----------------------------------------------------------------------------
  new Timer(new FreeHeapLogTimerAdapter(), Timer::IS_RECURRING, c_freeHeapLogIntervalMillis);

#ifdef ESP8266
  //-----------------------------------------------------------------------------
  // WiFi Connection
  //-----------------------------------------------------------------------------
//  wifiClient = new MemphisWiFiClient(WIFI_SSID, WIFI_PWD);
//  if (0 != wifiClient)
//  {
//    wifiClient->begin();
//
//    //-----------------------------------------------------------------------------
//    // ThingSpeak Client
//    //-----------------------------------------------------------------------------
//    ThingSpeak.begin(*wifiClient->getClient());
//  }

  //-----------------------------------------------------------------------------
  // MQTT Client
  //-----------------------------------------------------------------------------
//  mqttClient = new MqttClient(MQTT_SERVER_IP, MQTT_PORT, wifiClient);
//  if (0 != mqttClient)
//  {
//    mqttClient->setCallback(callback);
//    mqttClient->startupClient();
//  }
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
  new Timer(new PulseMockTimerAdapter(matrix), Timer::IS_RECURRING, c_PulseMockTimerModulateInterval);
//  pulseSensor = new PolarPulse(PolarPulse::PLS_NC, PULSE_IND_PIN, PolarPulse::IS_POS_LOGIC);
//  if (0 != pulseSensor)
//  {
//    pulseSensor->attachAdapter(new MemphisPulseSensorAdapter(PULSE_PIN, pulseSensor, wifiClient, cMyChannelNumber, cMyWriteAPIKey, matrix));
//  }
//  pulseSensor = new PolarPulse(PULSE_PIN, PULSE_IND_PIN, PolarPulse::IS_POS_LOGIC);
//  if (0 != pulseSensor)
//  {
//    pulseSensor->attachAdapter(new MemphisPulseSensorAdapter(PolarPulse::PLS_NC, pulseSensor, 0 /*wifiClient*/, cMyChannelNumber, cMyWriteAPIKey, matrix));
//  }

  //-----------------------------------------------------------------------------
  // Battery Voltage Surveillance
  //-----------------------------------------------------------------------------
//  battery = new Battery();
//  if (0 != battery)
//  {
//    batteryAdapter = new MyBatteryAdapter(battery, matrix);
//    battery->attachAdapter(batteryAdapter);
//  }
}

void loop()
{
  if (0 != sCmd)
  {
    sCmd->readSerial();     // process serial commands
  }
//  if (0 != mqttClient)
//  {
//    mqttClient->loop();     // process MQTT protocol
//  }
  yield();                  // process Timers
}
