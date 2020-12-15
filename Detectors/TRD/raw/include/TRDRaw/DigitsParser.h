// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   EpnRawReaderTask.h
/// @author Sean Murray
/// @brief  TRD cru output data to tracklet task

#ifndef O2_TRD_DIGITSPARSER
#define O2_TRD_DIGITSPARSER

#include "Framework/Task.h"
#include "Framework/DataProcessorSpec.h"
#include <fstream>
#include "TRDBase/Digit.h"
#include "DataFormatsTRD/RawData.h"
#include "DataFormatsTRD/TriggerRecord.h"

using namespace o2::framework;

namespace o2::trd
{

class DigitsParser
{

 public:
  DigitsParser() = default;
  ~DigitsParser() = default;
  void setData(std::array<uint32_t,1048576>* data) { mData = data; }
  void setLinkLengths(std::array<uint32_t, 15>& lengths) { mCurrentHalfCRULinkLengths = lengths; };
  int Parse(); // presupposes you have set everything up already.
  int Parse(std::array<uint32_t,1048576> *data, std::array<uint32_t, 15>& lengths)
  {
    setData(data);
    setLinkLengths(lengths);
    return Parse();
  };
  enum DigitParserState { StateDigitHCHeader, // always the start of a half chamber.
                             StateDigitMCMHeader,
                             StateDigitMCMData,
                             StatePadding };



 private:
  int mState;
  int mDataWordsParsed; // count of data wordsin data that have been parsed in current call to parse.
  int mDigitsFound;  // tracklets found in the data block, mostly used for debugging.
  int mBufferLocation;
  std::array<uint32_t,1048576>* mData = nullptr;     // parsed in vector of raw data to parse.
  std::vector<Digit> mDigits;                 // outgoing parsed digits
  std::vector<TriggerRecord> mTriggerRecords; // trigger records to index into the digits vector.
  int mParsedWords{0};                        // words parsed in data vector, last complete bit is not parsed, and left for another round of data update.
  DigitHCHeader* mDigitHCHeader;
  DigitMCMHeader* mDigitMCMHeader;
  DigitMCMData* mDigitMCMData;
  
  uint16_t mCurrentLink; // current link within the halfcru we are parsing 0-14
  uint16_t mCRUEndpoint; // the upper or lower half of the currently parsed cru 0-14 or 15-29
  uint16_t mCRUID;
  uint16_t mHCID;
  uint16_t mFEEID;                         // current Fee ID working on
  uint32_t mCurrentLinkDataPosition256;    // count of data read for current link in units of 256 bits
  uint32_t mCurrentLinkDataPosition;       // count of data read for current link in units of 256 bits
  uint32_t mCurrentHalfCRUDataPosition256; //count of data read for this half cru.
  std::array<uint32_t, 15> mCurrentHalfCRULinkLengths; // not in units of 256 bits or 32 bytes or 8 words
};

} // namespace o2::trd

#endif // O2_TRD_DIGITSPARSER
