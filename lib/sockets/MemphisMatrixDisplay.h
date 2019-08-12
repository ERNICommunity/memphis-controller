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
class DbgCli_Command;
class CmdSequence;

class MemphisMatrixDisplay
{
public:
  MemphisMatrixDisplay(uint8_t pin);
  virtual ~MemphisMatrixDisplay();

  CmdSequence* imageSequence();

  void setHeartBeatRate(unsigned int heartBeatRate);
  void showFirstFrame();
  void showNextFrame();

  void activateDisplay();
  void deactivateDisplay();

  void selectImage(unsigned int frame);
  unsigned int selectedImage();

  bool isPrintText();
  void setIstPrintText(bool isPrintText);

  DbgCli_Topic* getCliTopicMatrix() { return m_dbgCliTopicMatrix; }

  void blankDisplay();

private:
  uint16_t drawRGB24toRGB565(byte r, byte g, byte b);
  void updateText();
  void updateHeart();
  void updateDisplay();

  void prepareImageSequence();

public:
  const static unsigned char s_matrixEdgeLength = 16;
  const static int c_displayBlankPin = 10;

private:
  Adafruit_NeoMatrix* m_neoMatrix;
  unsigned int m_heartBeatRate;
  Timer* m_heartAnimationTimer;
  unsigned int m_frame;
  bool m_isDisplayActive;
  unsigned int m_selectedImage;  /// 0: blank display, 1: Arkathon heart, 2: ERNI heart, 3: SBB heart
  CmdSequence* m_imageSequence;
  bool m_printText;
  DbgCli_Topic* m_dbgCliTopicMatrix;
  DbgCli_Command* m_dbgCliSelImageCmd;
  DbgCli_Command* m_dbgCliRunSequenceCmd;
  DbgCli_Command* m_dbgCliTxtEnaCmd;

private:  // forbidden functions
  MemphisMatrixDisplay();                                             // default constructor
  MemphisMatrixDisplay(const MemphisMatrixDisplay& src);              // copy constructor
  MemphisMatrixDisplay& operator = (const MemphisMatrixDisplay& src); // assignment operator
};

#endif /* LIB_SOCKETS_MEMPHISMATRIXDISPLAY_H_ */
