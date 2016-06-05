/*
 * MemphisWiFiClient.h
 *
 *  Created on: 08.04.2016
 *      Author: scan
 */

#ifndef PROD_SOCKETS_WIFICLIENT_H_
#define PROD_SOCKETS_WIFICLIENT_H_

#include <WiFiClient.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

class Timer;
class DbgTrace_Port;

typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;


class MemphisWiFiClient
{
public:
  MemphisWiFiClient(const char* wifi_ssid, const char* wifi_pw);
  virtual ~MemphisWiFiClient();

  void begin();
  void printWiFiStatusChanged(wl_status_t& old_wlStatus);
  wl_status_t getStatus();
  bool isConnected();
  inline WiFiClient* getClient() { return m_client; };
  const char* getMacAddress() const;

private:
  Timer* m_wifiConnectTimer;
  WiFiClient* m_client;
  DbgTrace_Port* m_trPort;
  const char* m_WiFi_ssid;
  const char* m_WiFi_pw;
  static const unsigned long s_connectInterval_ms;

private:  // forbidden functions
  MemphisWiFiClient(const MemphisWiFiClient& src);              // copy constructor
  MemphisWiFiClient& operator = (const MemphisWiFiClient& src); // assignment operator
};

#endif /* PROD_SOCKETS_WIFICLIENT_H_ */
