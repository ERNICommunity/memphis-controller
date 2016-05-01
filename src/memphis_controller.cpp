#include "memphis_controller.h"

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


//-----------------------------------------------------------------------------
// NeoPixel Matrix
//-----------------------------------------------------------------------------

Adafruit_NeoPixel* pixels = 0;
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            12
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      256
int delayval = 1; // delay for half a second

uint32_t Wheel(byte WheelPos);

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<pixels->numPixels(); i++) {
    pixels->setPixelColor(i, c);
    pixels->show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<pixels->numPixels(); i++) {
      pixels->setPixelColor(i, Wheel((i+j) & 255));
    }
    pixels->show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< pixels->numPixels(); i++) {
      pixels->setPixelColor(i, Wheel(((i * 256 / pixels->numPixels()) + j) & 255));
    }
    pixels->show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels->numPixels(); i=i+3) {
        pixels->setPixelColor(i+q, c);    //turn every third pixel on
      }
      pixels->show();

      delay(wait);

      for (uint16_t i=0; i < pixels->numPixels(); i=i+3) {
        pixels->setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels->numPixels(); i=i+3) {
        pixels->setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      pixels->show();

      delay(wait);

      for (uint16_t i=0; i < pixels->numPixels(); i=i+3) {
        pixels->setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels->Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels->Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels->Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

class MatrixDemoTimerAdapter : public TimerAdapter
{
public:
  void timeExpired()
  {
    if (0 != pixels)
    {
      // Some example procedures showing how to display to the pixels:
      colorWipe(pixels->Color(255, 0, 0), 50); // Red
      colorWipe(pixels->Color(0, 255, 0), 50); // Green
      colorWipe(pixels->Color(0, 0, 255), 50); // Blue
    //colorWipe(pixels->Color(0, 0, 0, 255), 50); // White RGBW
      // Send a theater pixel chase in...
      theaterChase(pixels->Color(127, 127, 127), 50); // White
      theaterChase(pixels->Color(127, 0, 0), 50); // Red
      theaterChase(pixels->Color(0, 0, 127), 50); // Blue

      rainbow(20);
      rainbowCycle(20);
      theaterChaseRainbow(50);
    }
  }
};


//-----------------------------------------------------------------------------
// WiFi Client
//-----------------------------------------------------------------------------
#ifdef ESP8266
MemphisWiFiClient* wifiClient = 0;
// Update these with values suitable for your network.
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

void setup()
{
  //-----------------------------------------------------------------------------
  // Serial Command Object for Debug CLI
  //-----------------------------------------------------------------------------
  Serial.begin(115200);
  sCmd = new SerialCommand();
  DbgCli_Node::AssignRootNode(new DbgCli_Topic(0, "dbg", "Internet of Fish Aquarium Controller Debug CLI Root Node."));

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
  // NeoPixel Matrix
  //-----------------------------------------------------------------------------

  // When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
  // Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
  // example for more information on possible values.
  pixels = new Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//  new Timer(new MatrixDemoTimerAdapter(), Timer::IS_RECURRING, delayval);
  pixels->begin();
  pixels->show();
 }

void loop()
{
  if (0 != sCmd)
  {
    sCmd->readSerial();                       // process serial commands
  }
  if (0 != pixels)
  {
    // Some example procedures showing how to display to the pixels:
    colorWipe(pixels->Color(255, 0, 0), 50); // Red
    colorWipe(pixels->Color(0, 255, 0), 50); // Green
    colorWipe(pixels->Color(0, 0, 255), 50); // Blue
  //colorWipe(pixels->Color(0, 0, 0, 255), 50); // White RGBW
    // Send a theater pixel chase in...
    theaterChase(pixels->Color(127, 127, 127), 50); // White
    theaterChase(pixels->Color(127, 0, 0), 50); // Red
    theaterChase(pixels->Color(0, 0, 127), 50); // Blue

    rainbow(20);
    rainbowCycle(20);
    theaterChaseRainbow(50);
  }

  yield();                                    // process Timers
}
