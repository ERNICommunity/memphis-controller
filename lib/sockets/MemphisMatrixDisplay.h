/*
 * MemphisMatrixDisplay.h
 *
 *  Created on: 29.05.2016
 *      Author: niklausd
 */

#ifndef LIB_SOCKETS_MEMPHISMATRIXDISPLAY_H_
#define LIB_SOCKETS_MEMPHISMATRIXDISPLAY_H_

class Timer;
class Adafruit_NeoMatrix;

class MemphisMatrixDisplay
{
public:
  MemphisMatrixDisplay(uint8_t pin);
  virtual ~MemphisMatrixDisplay();

  void setHeartBeatRate(unsigned int heartBeatRate);
  void showNextFrame();

  void activateDisplay();
  void deactivateDisplay();

  void selectFrame2(bool isFrame2);


private:
  uint16_t drawRGB24toRGB565(byte r, byte g, byte b);
  void updateText();
  void updateHeart();
  void updateDisplay();

public:
  const static unsigned char s_matrixEdgeLength = 16;

private:
  Adafruit_NeoMatrix* m_neoMatrix;
  unsigned int m_heartBeatRate;
  Timer* m_heartAnimationTimer;
  unsigned int m_frame;
  bool m_isDisplayActive;
  bool m_showHeart2;
  bool m_showHeart3;
  bool m_printText;

private:  // forbidden functions
  MemphisMatrixDisplay();                                             // default constructor
  MemphisMatrixDisplay(const MemphisMatrixDisplay& src);              // copy constructor
  MemphisMatrixDisplay& operator = (const MemphisMatrixDisplay& src); // assignment operator
};

#endif /* LIB_SOCKETS_MEMPHISMATRIXDISPLAY_H_ */
