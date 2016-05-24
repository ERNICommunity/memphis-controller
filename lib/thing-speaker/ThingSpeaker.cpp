/*
 * ThingSpeaker.cpp
 *
 *  Created on: 24.05.2016
 *      Author: kork
 */

#include "ThingSpeaker.h"

const unsigned int ThingSpeaker::s_numOfFields = 8;

ThingSpeaker::ThingSpeaker()
{
  for (unsigned int i = 0; i < s_numOfFields; i++)
  {
    m_field.push_back("");
  }
}

ThingSpeaker::~ThingSpeaker()
{ }

