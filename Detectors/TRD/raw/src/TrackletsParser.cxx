// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   TrackletParser.h
/// @brief  TRD raw data parser for Tracklet dat format

#include "TRDRaw/TrackletsParser.h"
#include "DataFormatsTRD/RawData.h"
#include "DataFormatsTRD/Tracklet64.h"

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

int TrackletsParser::Parse()
{
  //we are handed the buffer payload of an rdh and need to parse its contents. 
  //producing a vector of digits.
  LOG(debug) << "Tracklet Parser parse of data sitting at :" << std::hex << (void*) mData;
  //mData holds a buffer of <8kb (the o2 payload) parse placing tracklets in the output vector.
  //mData holds 2048 digits.
  // due to the nature of the incoming data, there will *never* straggling digits or for that matter trap outputs spanning a boundary. 
  for(auto word : *mData){
      //loop over all the words ... duh
      //this is the payload sans the cruhalflinkheaders.
      //
      //
      //
      //
      //
  mCurrentLinkDataPosition = 0;
  mCurrentHalfCRULinkHeaderPoisition=0;
  mCurrentLink = 0;

  while (mDataPointer != mDataEndPointer && mCurrentLinkDataPosition < mTotalHalfCRUDataLength * 16) { // while we are stil in the rdh block and with in the current link
    LOG(debug) << "in while loop with state of :" << mState;
      LOGF(debug, "mDataPointer: %p != mDataEndPointer: %p, mCurrentLinkDataPosition=%d != mTotalHalfCRUDataLenght=%d\n", (void*)mDataPointer, (void*)mDataPointer,mCurrentLinkDataPosition,mTotalHalfCRUDataLength*16);
    if (mState == CRUStateHalfChamber) {
      // read in the halfchamber header.
      LOGF(debug, "mTrackletHCHeader is at %p had value 0x%08x", (void*)mDataPointer, mDataPointer[0]);
      mTrackletHCHeader = (TrackletHCHeader*)mDataPointer;
      mDataPointer += 16; //sizeof(mTrackletHCHeader)/4;
      mHCID = getHCIDFromTrackletHCHeader(mTrackletHCHeader->word);
      LOG(info)<< "HCID set to  : " << mHCID;
      //     LOGF(info,"mDataPointer after advancing past TrackletHCHeader is at %p has value 0x%08x",(void*)mDataPointer,mDataPointer[0]);
      //if(debugparsing){
      //     printHalfChamber(*mTrackletHCHeader);
      // }
      mState = CRUStateTrackletMCMHeader; //now we expect a TrackletMCMHeader or some padding.
    }
    if (mState == CRUStateTrackletMCMHeader) {
      LOGF(debug, "mTrackletMCMHeader is at %p had value 0x%08x", (void*)mDataPointer, mDataPointer[0]);
      if (debugparsing) {
        //           LOG(debug) << " state is : " << mState << " about to read TrackletMCMHeader";
      }
      //read the header OR padding of 0xeeee;
      if (mDataPointer[0] != 0xeeeeeeee) {
        //we actually have an header word.
        mTrackletHCHeader = (TrackletHCHeader*)mDataPointer;
        LOG(debug) << "state mcmheader and word : 0x" << std::hex << mDataPointer[0];
        mDataPointer++;
        mCurrentLinkDataPosition++;
        if (debugparsing) {
          //       printTrackletMCMHeader(*mTrackletHCHeader);
        }
        mState = CRUStateTrackletMCMData;
      } else { // this is the case of a first padding word for a "noncomplete" tracklet i.e. not all 3 tracklets.
               //        LOG(debug) << "C";
        mState = CRUStatePadding;
        mDataPointer++;
        mCurrentLinkDataPosition++;
        TRDStatCounters.LinkPadWordCounts[mHCID]++; // keep track off all the padding words.
        if (debugparsing) {
          //       printTrackletMCMHeader(*mTrackletHCHeader);
        }
      }
    }
    if (mState == CRUStatePadding) {
      LOGF(debug, "Padding is at %p had value 0x%08x", (void*)mDataPointer, mDataPointer[0]);
      LOG(debug) << "state padding and word : 0x" << std::hex << mDataPointer[0];
      if (mDataPointer[0] == 0xeeeeeeee) {
        //another pointer with padding.
        mDataPointer++;
        mCurrentLinkDataPosition++;
        TRDStatCounters.LinkPadWordCounts[mHCID]++; // keep track off all the padding words.
        if (mDataPointer[0] & 0x1) {
          //mcmheader
          //        LOG(debug) << "changing state from padding to mcmheader as next datais 0x" << std::hex << mDataPointer[0];
          mState = CRUStateTrackletMCMHeader;
        } else if (mDataPointer[0] != 0xeeeeeeee) {
          //        LOG(debug) << "changing statefrom padding to mcmdata as next datais 0x" << std::hex << mDataPointer[0];
          mState = CRUStateTrackletMCMData;
        }
      } else {
        LOG(debug) << "some went wrong we are in state padding, but not a pad word. 0x" << (void*)mDataPointer;
      }
    }
    if (mState == CRUStateTrackletMCMData) {
      LOGF(debug, "mTrackletMCMData is at %p had value 0x%08x", (void*)mDataPointer, mDataPointer[0]);
      //tracklet data;
      // build tracklet.
      //for the case of on flp build a vector of tracklets, then pack them into a data stream with a header.
      //for dpl build a vector and connect it with a triggerrecord.
      mTrackletMCMData = (TrackletMCMData*)mDataPointer;
      mDataPointer++;
      mCurrentLinkDataPosition++;
      if (mDataPointer[0] == 0xeeeeeeee) {
        mState = CRUStatePadding;
        //  LOG(debug) <<"changing to padding from mcmdata" ;
      } else {
        if (mDataPointer[0] & 0x1) {
          mState = CRUStateTrackletMCMHeader; // we have more tracklet data;
          LOG(debug) << "changing from MCMData to MCMHeader";
        } else {
          mState = CRUStateTrackletMCMData;
          LOG(debug) << "continuing with mcmdata";
        }
      }
      // Tracklet64 trackletsetQ0(o2::trd::getTrackletQ0());
    }
    //accounting ....
    // mCurrentLinkDataPosition256++;
    // mCurrentHalfCRUDataPosition256++;
    // mTotalHalfCRUDataLength++;
    LOG(debug) << mDataPointer << ":" << mDataEndPointer << " &&  " << mCurrentLinkDataPosition << " != " << mTotalHalfCRUDataLength * 16;
  }
  //end of data so

  }
  return 1;
}


} // namespace trd
} // namespace o2
