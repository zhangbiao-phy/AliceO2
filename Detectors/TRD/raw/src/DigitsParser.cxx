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

int DigitsParser::Parse()
{

  //we are handed the buffer payload of an rdh and need to parse its contents.
  //producing a vector of digits.
  LOG(debug) << "Digits Parser parse of data sitting at :" << std::hex << (void*)mData;
  //mData holds a buffer containing tracklets parse placing tracklets in the output vector.
  //mData holds 2048 digits.
  // due to the nature of the incoming data, there will *never* straggling digits or for that matter trap outputs spanning a boundary.
  mCurrentLinkDataPosition = 0;
  mCurrentLink = 0;
  mBufferLocation = 0;
  int currentLinkStart = 0;

  for (auto word : *mData) { // loop over the entire cru payload.
    //loop over all the words ... duh
    //this is the payload sans the cruhalflinkheaders.
    if (mBufferLocation == currentLinkStart + mCurrentHalfCRULinkLengths[mCurrentLink]) {
      // we are changing a link.
      currentLinkStart += mCurrentHalfCRULinkLengths[mCurrentLink];
      //increment the link we are on, change relevant other data.
      mCurrentLink++;
      //sanity check
      if (mCurrentLink == 15) {
        LOG(warn) << "link count during parsing is 15, should end at 14 ...";
      }
      mState = StateDigitHCHeader; // we are the start of another link.
    }
//failing here.
    if (mState == StateDigitHCHeader && (mBufferLocation == currentLinkStart + mCurrentHalfCRULinkLengths[mCurrentLink])) {
      LOG(warn) << " Parsing state is StateDigitHCHeader, yet according to the lengths we are not at the beginning of a half chamber. " << mBufferLocation << " != " << currentLinkStart << "+" << mCurrentHalfCRULinkLengths[mCurrentLink];
    }
    // we are changing a link.
    if (mState == StateDigitMCMHeader) {
      LOG(debug) << "mDigitMCMHeader is has value 0x" << std::hex << word;
      //read the header OR padding of 0xeeee;
      if (word != 0xeeeeeeee) {
        //we actually have an header word.
        mDigitHCHeader = (DigitHCHeader*)&word;
        LOG(debug) << "state mcmheader and word : 0x" << std::hex << word;
        //sanity check of trackletheader ??  Still to implement.
        //        if (!digitMCMHeaderSanityCheck(*mDigitMCMHeader)) {
        //          LOG(warn) << "Sanity check Failure MCMHeader : " << mDigitMCMHeader;
        //        };
        mBufferLocation++;
        mCurrentLinkDataPosition++;
        mState = StateDigitMCMData;
      } else { // this is the case of a first padding word for a "noncomplete" tracklet i.e. not all 3 tracklets.
               //        LOG(debug) << "C";
        mState = StatePadding;
        mBufferLocation++;
        mCurrentLinkDataPosition++;
        //    TRDStatCounters.LinkPadWordCounts[mHCID]++; // keep track off all the padding words.
      }
    } else {
      if (mState == StatePadding) {
        LOG(debug) << "state padding and word : 0x" << std::hex << word;
        if (word == 0xeeeeeeee) {
          //another pointer with padding.
          mBufferLocation++;
          mCurrentLinkDataPosition++;
          //TRDStatCounters.LinkPadWordCounts[mHCID]++; // keep track off all the padding words.
          if (word & 0x1) {
            //mcmheader
            //        LOG(debug) << "changing state from padding to mcmheader as next datais 0x" << std::hex << mDataPointer[0];
            mState = StateDigitMCMHeader;
          } else if (word != 0xeeeeeeee) {
            //        LOG(debug) << "changing statefrom padding to mcmdata as next datais 0x" << std::hex << mDataPointer[0];
            mState = StateDigitMCMData;
          }
        } else {
          LOG(debug) << "some went wrong we are in state padding, but not a pad word. 0x" << word;
        }
      }
      if (mState == StateDigitMCMData) {
        LOG(debug) << "mDigitMCMData is at " << mBufferLocation << " had value 0x" << std::hex << word;
        //tracklet data;
        // build tracklet.
        //for the case of on flp build a vector of tracklets, then pack them into a data stream with a header.
        //for dpl build a vector and connect it with a triggerrecord.
        mDigitMCMData = (DigitMCMData*)&word;
        mBufferLocation++;
        mCurrentLinkDataPosition++;
        if (word == 0xeeeeeeee) {
          mState = StatePadding;
          //  LOG(debug) <<"changing to padding from mcmdata" ;
        } else {
          if (word & 0x1) {
            mState = StateDigitMCMHeader; // we have more tracklet data;
            LOG(debug) << "changing from MCMData to MCMHeader";
          } else {
            mState = StateDigitMCMData;
            LOG(debug) << "continuing with mcmdata";
          }
        }
        // Digit64 trackletsetQ0(o2::trd::getDigitQ0());
      }

      //accounting ....
      // mCurrentLinkDataPosition256++;
      // mCurrentHalfCRUDataPosition256++;
      // mTotalHalfCRUDataLength++;
      //end of data so
    }
  }
  return 1;
}

} // namespace trd
} // namespace o2
