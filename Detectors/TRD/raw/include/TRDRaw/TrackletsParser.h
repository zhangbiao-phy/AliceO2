// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   TrackletsParser.h
/// @author Sean Murray
/// @brief  TRD parse tracklet o2 payoload and build tracklets.

#ifndef O2_TRD_TRACKLETPARSER
#define O2_TRD_TRACKLETPARSER

#include <fstream>
#include <vector>

#include "DataFormatsTRD/RawData.h"
#include "DataFormatsTRD/Tracklet64.h"
#include "DataFormatsTRD/TriggerRecord.h"

namespace o2::trd
{

class TrackletsParser
{
 public:
  TrackletsParser() = default;
  ~TrackletsParser() = default;
  void setData(std::vector<uint64_t>* data) { mData = data; }
  void setLinkLengths(std::array<uint32_t, 15>& lengths) { mCurrentHalfCRULinkLengths = lengths; };
  int Parse(); // presupposes you have set everything up already.
  int Parse(std::vector<uint64_t>* data, const std::array<uint32_t, 15>& lengths)
  {
    mData = data;
    mCurrentHalfCRULinkLengths = lengths;
    return Parse();
  };

  int getDataWordsParsed() { return mDataWordsParsed; }
  int getTrackletsFound() { return mTrackletsFound; }
  enum TrackletParserState { StateTrackletHCHeader, // always the start of a half chamber.
                             StateTrackletMCMHeader,
                             StateTrackletMCMData,
                             StatePadding };

 private:
  std::vector<uint64_t>* mData;
  std::vector<Tracklet64> mTracklets;
  int mState;
  int mDataWordsParsed; // count of data wordsin data that have been parsed in current call to parse.
  int mTrackletsFound;  // tracklets found in the data block, mostly used for debugging.
  int mBufferLocation;
  TrackletHCHeader* mTrackletHCHeader;
  TrackletMCMHeader* mTrackletMCMHeader;
  TrackletMCMData* mTrackletMCMData;

  uint16_t mCurrentLink; // current link within the halfcru we are parsing 0-14
  uint16_t mCRUEndpoint; // the upper or lower half of the currently parsed cru 0-14 or 15-29
  uint16_t mCRUID;
  uint16_t mHCID;
  uint16_t mFEEID;                         // current Fee ID working on
  uint32_t mCurrentLinkDataPosition256;    // count of data read for current link in units of 256 bits
  uint32_t mCurrentLinkDataPosition;       // count of data read for current link in units of 256 bits
  uint32_t mCurrentHalfCRUDataPosition256; //count of data read for this half cru.
  std::array<uint32_t, 15> mCurrentHalfCRULinkLengths;
  //  std::array<uint32_t, 16> mAverageNumTrackletsPerTrap; TODO come back to this stat.
};

} // namespace o2::trd

#endif // O2_TRD_TRACKLETPARSER
