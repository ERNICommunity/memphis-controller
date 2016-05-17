#include "Arduino.h"
#include <SerialCommand.h>
#include <Timer.h>
#include <DbgCliNode.h>
#include <DbgCliTopic.h>
#include <DbgTracePort.h>
#include <DbgTraceContext.h>
#include <DbgTraceOut.h>
#include <DbgPrintConsole.h>
#include <DbgTraceLevel.h>
#ifdef ESP8266
#include <MemphisWiFiClient.h>
#endif
#include <Adafruit_NeoPixel.h>
#include <PolarPulse.h>
#include <MemphisPulseSensorAdapter.h>

//-----------------------------------------------------------------------------
// WiFi Client
//-----------------------------------------------------------------------------
#ifdef ESP8266
MemphisWiFiClient* wifiClient = 0;
#define WIFI_SSID       "linksys"
#define WIFI_PWD        "jtv8a9r3"
#endif

//-----------------------------------------------------------------------------
// Arduino Cmd I/F
//-----------------------------------------------------------------------------
SerialCommand* sCmd = 0;

void dbgCliExecute()
{
  if ((0 != sCmd) && (0 != DbgCli_Node::RootNode()))
  {
    const unsigned int firstArgToHandle = 1;
    const unsigned int maxArgCnt = 10;
    char* args[maxArgCnt];
    char* arg = "dbg";
    unsigned int arg_cnt = 0;
    while ((maxArgCnt > arg_cnt) && (0 != arg))
    {
      args[arg_cnt] = arg;
      arg = sCmd->next();
      arg_cnt++;
    }
    DbgCli_Node::RootNode()->execute(static_cast<unsigned int>(arg_cnt), const_cast<const char**>(args), firstArgToHandle);
  }
}

void sayHello()
{
  char *arg;
  if (0 != sCmd)
  {
    arg = sCmd->next();    // Get the next argument from the SerialCommand object buffer
  }
  else
  {
    arg = "";
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
// Free Heap Logger
//-----------------------------------------------------------------------------
#ifdef ESP8266
extern "C"
{
  #include "user_interface.h"
}
const unsigned long c_freeHeapLogIntervalMillis = 10000;
class FreeHeapLogTimerAdapter : public TimerAdapter
{
private:
  DbgTrace_Port* m_trPort;
public:
  FreeHeapLogTimerAdapter()
  : m_trPort(new DbgTrace_Port("heap", "trConOut", DbgTrace_Level::info))
  { }

  void timeExpired()
  {
    TR_PRINT_LONG(m_trPort, DbgTrace_Level::debug, system_get_free_heap_size());
  }
};
#endif

//-----------------------------------------------------------------------------
// Pulse Sensor
//-----------------------------------------------------------------------------
PolarPulse* pulseSensor = 0;

//-----------------------------------------------------------------------------

void setup()
{
  //-----------------------------------------------------------------------------
  // Serial Command Object for Debug CLI
  //-----------------------------------------------------------------------------
  Serial.begin(115200);
  sCmd = new SerialCommand();
  DbgCli_Node::AssignRootNode(new DbgCli_Topic(0, "dbg", "Memphis Controller Debug CLI Root Node."));

  // Setup callbacks for SerialCommand commands
  if (0 != sCmd)
  {
    sCmd->addCommand("dbg", dbgCliExecute);
    sCmd->addCommand("hello", sayHello);        // Echos the string argument back
    sCmd->setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")
  }

  //---------------------------------------------------------------------------
  // Debug Trace
  //---------------------------------------------------------------------------
  new DbgTrace_Context(new DbgCli_Topic(DbgCli_Node::RootNode(), "tr", "Modify debug trace"));
  new DbgTrace_Out(DbgTrace_Context::getContext(), "trConOut", new DbgPrint_Console());

  Serial.println();
  Serial.println(F("---------------------------------------------"));
  Serial.println(F("Hello from Memphis Controller!"));
  Serial.println(F("---------------------------------------------"));
  Serial.println();

  //-----------------------------------------------------------------------------
  // Free Heap Logger
  //-----------------------------------------------------------------------------
#ifdef ESP8266
  new Timer(new FreeHeapLogTimerAdapter(), Timer::IS_RECURRING, c_freeHeapLogIntervalMillis);

  //-----------------------------------------------------------------------------
  // WiFi Connection
  //-----------------------------------------------------------------------------
  wifiClient = new MemphisWiFiClient(WIFI_SSID, WIFI_PWD);
  if (0 != wifiClient)
  {
    wifiClient->begin();
  }
#endif

  //-----------------------------------------------------------------------------
  // Pulse Sensor
  //-----------------------------------------------------------------------------
  pulseSensor = new PolarPulse(13, LED_BUILTIN, PolarPulse::IS_POS_LOGIC, new MemphisPulseSensorAdapter());
 }

void loop()
{
  if (0 != sCmd)
  {
    sCmd->readSerial();                       // process serial commands
  }
  yield();                                    // process Timers
}
