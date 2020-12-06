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
/// @brief  TRD cru output data to tracklet task

#ifndef O2_TRD_TRACKLETPARSER
#define O2_TRD_TRACKLETPARSER

#include <fstream>
#include <vector>

namespace o2
{
namespace trd
{
class Tracklet64;

class TrackletsParser 
{
 public:
  TrackletsParser() = default;
  ~TrackletsParser() = default;
  void setData(std::vector<uint64_t> * data){mData=data;}
  int Parse();
  int Parse(std::vector<uint64_t> *data){mData=data;return Parse();};

  int getDataWordsParsed(){return mDataParsed;}
  int getTrackletsFound(){return mTrackletsFound;}
 private:
  std::vector<uint64_t> *mData;
  std::vector<Tracklet64> mTracklets;
  int mDataParsed;   // count of data wordsin data that have been parsed in current call to parse.
  int mTrackletsFound; // used for debugging.
  
};

} // namespace trd
} // namespace o2

#endif // O2_TRD_CRU2TRACKLETTASK
