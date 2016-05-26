#include "Arduino.h"

// PlatformIO libraries
#include <SerialCommand.h>
#include <PubSubClient.h>
#include <ThingSpeak.h>
#include <Client.h>

// private libraries
#include <Timer.h>
#include <DbgCliNode.h>
#include <DbgCliTopic.h>
#include <DbgTracePort.h>
#include <DbgTraceContext.h>
#include <DbgTraceOut.h>
#include <DbgPrintConsole.h>
#include <DbgTraceLevel.h>
#include <MemphisWiFiClient.h>
#include <PolarPulse.h>
#include <MemphisPulseSensorAdapter.h>
#include <MqttClient.h>
#include <ConnectivitySecrets.h>

//-----------------------------------------------------------------------------
// WiFi Client
//-----------------------------------------------------------------------------
#ifdef ESP8266
MemphisWiFiClient* wifiClient = 0;
MqttClient* mqttClient = 0;
#define MQTT_SERVER_IP  "iot.eclipse.org"
#define MQTT_PORT       1883
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
  : m_trPort(new DbgTrace_Port("heap", DbgTrace_Level::info))
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
#define PULSE_PIN 13

//-----------------------------------------------------------------------------
// NEO Matrix
//-----------------------------------------------------------------------------
#define NEO_PIN    6
#define NEO_SIZE  16

// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)


// Example for NeoPixel Shield.  In this application we'd like to use it
// as a 5x8 tall matrix, with the USB port positioned at the top of the
// Arduino.  When held that way, the first pixel is at the top right, and
// lines are arranged in columns, progressive order.  The shield uses
// 800 KHz (v2) pixels that expect GRB color data.
//Adafruit_NeoMatrix* matrix = new Adafruit_NeoMatrix(NEO_SIZE, NEO_SIZE, NEO_PIN,
//  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
//  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
//  NEO_GRB            + NEO_KHZ800);
//
//const uint16_t colors[] = {
//  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };
//
//int x = matrix.width();
//int pass = 0;
//int counter = 1;

//-----------------------------------------------------------------------------

void setup()
{
  //-----------------------------------------------------------------------------
  // Serial Command Object for Debug CLI
  //-----------------------------------------------------------------------------
  Serial.begin(115200);
  sCmd = new SerialCommand();
  DbgCli_Node::AssignRootNode(new DbgCli_Topic(0, "dbg", "Workforce2020 Controller Debug CLI Root Node."));

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

    //-----------------------------------------------------------------------------
    // ThingSpeak Client
    //-----------------------------------------------------------------------------
    ThingSpeak.begin(*wifiClient->getClient());
  }

  //-----------------------------------------------------------------------------
  // MQTT Client
  //-----------------------------------------------------------------------------
  mqttClient = new MqttClient(MQTT_SERVER_IP, MQTT_PORT, wifiClient);
  if (0 != mqttClient)
  {
//    mqttClient->setCallback(callback);
    mqttClient->startupClient();
  }
#endif

  //-----------------------------------------------------------------------------
  // Pulse Sensor
  //-----------------------------------------------------------------------------
  pulseSensor = new PolarPulse(PULSE_PIN, LED_BUILTIN, PolarPulse::IS_POS_LOGIC, new MemphisPulseSensorAdapter(wifiClient, cMyChannelNumber, cMyWriteAPIKey));

  //-----------------------------------------------------------------------------
  // NEO Matrix
  //-----------------------------------------------------------------------------
//  matrix.begin();
//  matrix.setTextWrap(false);
//  matrix.setBrightness(10);
//  matrix.setTextColor(colors[0]);
//  matrix.setTextSize(1);
}

void loop()
{
  if (0 != sCmd)
  {
    sCmd->readSerial();     // process serial commands
  }
  if (0 != mqttClient)
  {
    mqttClient->loop();     // process MQTT protocol
  }
  yield();                  // process Timers
}
