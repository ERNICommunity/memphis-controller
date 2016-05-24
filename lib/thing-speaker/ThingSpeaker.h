/*
 * ThingSpeaker.h
 *
 *  Created on: 24.05.2016
 *      Author: kork
 */

#ifndef LIB_THING_SPEAKER_THINGSPEAKER_H_
#define LIB_THING_SPEAKER_THINGSPEAKER_H_

#include <vector>
#include <string>

using namespace std;

class ThingSpeaker
{
public:
  ThingSpeaker();
  virtual ~ThingSpeaker();

private:
  static const unsigned int s_numOfFields;
  vector<string>m_field;

private: // forbidden default functions
  ThingSpeaker& operator = (const ThingSpeaker& src); // assignment operator
  ThingSpeaker(const ThingSpeaker& src);              // copy constructor
  ThingSpeaker();                                     // default constructor
};

#endif /* LIB_THING_SPEAKER_THINGSPEAKER_H_ */
