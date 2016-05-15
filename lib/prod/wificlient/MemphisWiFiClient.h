/*
 * WiFiClient.h
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

class MemphisWiFiClient
{
public:
  MemphisWiFiClient(char* wifi_ssid, char* wifi_pw);
  virtual ~MemphisWiFiClient();

  void begin();
  void printWiFiStatusChanged(wl_status_t& old_wlStatus);
  wl_status_t getStatus();
  inline WiFiClient* getClient() { return m_client; };
  const char* getMacAddress() const;

private:
  Timer* m_wifiConnectTimer;
  WiFiClient* m_client;
  DbgTrace_Port* m_trPort;
  char* m_WiFi_ssid;
  char* m_WiFi_pw;
  static const unsigned long s_connectInterval_ms;

private:  // forbidden functions
  MemphisWiFiClient(const MemphisWiFiClient& src);              // copy constructor
  MemphisWiFiClient& operator = (const MemphisWiFiClient& src); // assignment operator
};

#endif /* PROD_SOCKETS_WIFICLIENT_H_ */
