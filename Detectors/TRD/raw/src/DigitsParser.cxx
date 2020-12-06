// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   DigitsParser.h
/// @brief  TRD raw data parser for digits

#include "TRDRaw/DigitsParser.h"
#include "DataFormatsTRD/RawData.h"
#include "DataFormatsTRD/Tracklet64.h"
#include "TRDBase/Digit.h"

#include "fairlogger/Logger.h"

//TODO come back and figure which of below headers I actually need.
#include <cstring>
#include <string>
#include <vector>
#include <array>

namespace o2
{
namespace trd
{

uint32_t DigitsParser::Parse()
{
  //we are handed the buffer payload of an rdh and need to parse its contents. 
  //producing a vector of digits.
  LOG(debug) << "Digits Parser parse ";
  //mData holds a buffer of <8kb (the o2 payload) parse placing digits in the output vector.
  //mData holds 2048 digits.
  // due to the nature of the incoming data, there will *never* straggling digits or for that matter trap outputs spanning a boundary. 
  
  for(auto word : *mData){
      //loop over all the words ... duh
      //this is the payload sans the cruhalflinkheaders.
      
  }

  
  return 1;
}


} // namespace trd
} // namespace o2
