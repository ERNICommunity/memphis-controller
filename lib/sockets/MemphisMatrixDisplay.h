/*
 * MemphisMatrixDisplay.h
 *
 *  Created on: 29.05.2016
 *      Author: niklausd
 */

#ifndef LIB_SOCKETS_MEMPHISMATRIXDISPLAY_H_
#define LIB_SOCKETS_MEMPHISMATRIXDISPLAY_H_

#include <Adafruit_NeoMatrix.h>

class Timer;

class MemphisMatrixDisplay : public Adafruit_NeoMatrix
{
public:
  MemphisMatrixDisplay(uint8_t pin);
  virtual ~MemphisMatrixDisplay();

  void setHeartBeatRate(unsigned int heartBeatRate);
  void showNextFrame();

  void activateDisplay();
  void deactivateDisplay();


private:
  uint16_t drawRGB24toRGB565(byte r, byte g, byte b);
  void updateText();
  void updateHeart();
  void updateDisplay();

private:
  unsigned int m_heartBeatRate;
  Timer* m_heartAnimationTimer;
  unsigned int m_frame;
  bool m_isDisplayActive;

private:  // forbidden functions
  MemphisMatrixDisplay();                                             // default constructor
  MemphisMatrixDisplay(const MemphisMatrixDisplay& src);              // copy constructor
  MemphisMatrixDisplay& operator = (const MemphisMatrixDisplay& src); // assignment operator
};

#endif /* LIB_SOCKETS_MEMPHISMATRIXDISPLAY_H_ */
