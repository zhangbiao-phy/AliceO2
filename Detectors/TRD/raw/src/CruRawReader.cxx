// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   CruRawReader.h
/// @brief  TRD raw data translator

#include "DetectorsRaw/RDHUtils.h"
//#include "Headers/RAWDataHeader.h"
#include "Headers/RDHAny.h"
#include "TRDRaw/CruRawReader.h"
#include "DataFormatsTRD/RawData.h"
#include "DataFormatsTRD/Tracklet64.h"
#include "TRDRaw/DigitsParser.h"
#include "TRDRaw/TrackletsParser.h"

#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <numeric>

namespace o2
{
namespace trd
{

uint32_t CruRawReader::processHBFs()
{

  LOG(debug) << "PROCESS HBF starting at " << (void*)mDataPointer;

  mDataRDH = reinterpret_cast<const o2::header::RDHAny*>(mDataPointer);
  //mEncoderRDH = reinterpret_cast<o2::header::RDHAny*>(mEncoderPointer);
  auto rdh = mDataRDH;
  /* loop until RDH stop header */
  while (!o2::raw::RDHUtils::getStop(rdh)) { // carry on till the end of the event.
    LOG(debug) << "--- RDH open/continue detected";
    o2::raw::RDHUtils::printRDH(rdh);

    auto headerSize = o2::raw::RDHUtils::getHeaderSize(rdh);
    auto memorySize = o2::raw::RDHUtils::getMemorySize(rdh);
    auto offsetToNext = o2::raw::RDHUtils::getOffsetToNext(rdh);
    auto dataPayload = memorySize - headerSize;
    mFEEID = o2::raw::RDHUtils::getFEEID(rdh);            //TODO change this and just carry around the curreht RDH
    mCRUEndpoint = o2::raw::RDHUtils::getEndPointID(rdh); // the upper or lower half of the currently parsed cru 0-14 or 15-29
    mCRUID = o2::raw::RDHUtils::getCRUID(rdh);
    int packetCount = o2::raw::RDHUtils::getPacketCounter(rdh);
    LOG(debug) << "FEEID : " << mFEEID << " Packet: " << packetCount << " sizes : header" << headerSize << " memorysize:" << memorySize << " offsettonext:" << offsetToNext << " datapayload:" << dataPayload;
    //   we will parse on the fly with a basic state machine.
    LOGF(debug, "rdh ptr is %p\n", (void*)rdh);
    mDataPointer = (uint32_t*)((char*)rdh + headerSize);
    LOGF(debug, "mDataPointer is %p  ?= %p", (void*)mDataPointer, (void*)rdh);
    mDataEndPointer = (const uint32_t*)((char*)rdh + offsetToNext);
    LOGF(debug, "mDataEndPointer is %p\n ", (void*)mDataEndPointer);
    while ((void*)mDataPointer < (void*)mDataEndPointer) { // loop to handle the case where a halfcru ends/begins within the rdh data block
      mEventCounter++;
      LOGF(debug, "Process halfcru starting at : mDataPointer is %p  ?= %p  0x%x", (void*)mDataPointer, (void*)rdh, ((uint32_t*)mDataPointer)[0]);
      if (memorySize == 96 && ((uint32_t*)mDataPointer)[0] == 0xeeeeeeee) {
        //header with only padding word (blank event), ignore and move on.
        LOG(info) << "A MFEEID : " << std::hex << mFEEID << " rdh is at : " << (void*)rdh;
        mDataPointer = mDataEndPointer; //dump the rest
      } else {

        if (processHalfCRU()) {
          // process a halfcru
          // or continue with the remainder of an rdh o2 payload if we got to the end of cru
          // or continue with a new rdh payload if we are not finished with the last cru payload.
          if (mState == CRUStateHalfCRUHeader) {
            //ergo mCRUPayLoad holds the whole links payload, so parse it.
            // tracklet or digit ??
            switch (DataBufferFormatIs()) {
              case TrackletsDataFormat:
                LOG(info) << "Now to parse for Tracklets with a buffer length of " << mHalfCRUPayLoadRead;
                mTrackletsParser.Parse(&mCRUPayLoad, mCurrentHalfCRULinkLengths);
                break;
              case DigitsDataFormat:
                LOG(info) << "Now to parse for Digits";
                mDigitsParser.Parse();
                break;
              case ConfigEventDataFormat:
                LOG(info) << "Now to parse for a ConfigEvent";
                break; //mConfigEventParser.Parse();break;
              case TestPatternDataFormat:
                LOG(info) << "Now to parse for a TestPatter unimplemented yet";
                break; //mTestPatternParser.Parse();break;
              default:
                LOG(warn) << " we cant parse what we dont understand, Error in determining format of databuffer in CRU payload.";
            }
          } else {
            LOG(info) << "H";
            LOG(debug) << "Processed an rdh, no halfcru finished, looping around again to find an end";
          }
          //break; // end of CRU
          LOG(info) << "II";
        }
      }
      mState = CRUStateHalfChamber;
    }
    LOG(info) << "J";
    // move to next RDH
    LOG(info) << "moving rdh from " << (void*)rdh;
    rdh = (o2::header::RDHAny*)((char*)(rdh) + offsetToNext);
    LOG(info) << "BK";
    LOG(info) << " rdh is now at 0x" << (void*)rdh << " offset to next : " << offsetToNext;
  }

  if (o2::raw::RDHUtils::getStop(rdh)) {
    if (mDataPointer != (const uint32_t*)((char*)rdh + o2::raw::RDHUtils::getOffsetToNext(rdh))) {
      LOG(warn) << " at end of parsing loop and mDataPointer is on next rdh";
    }
    mDataPointer = (const uint32_t*)((char*)rdh + o2::raw::RDHUtils::getOffsetToNext(rdh));
    // make sure mDataPointer is in the correct place.
  } else
    mDataPointer = (const uint32_t*)((char*)rdh);
  LOGF(debug, " at exiting processHBF after advancing to next rdh mDataPointer is %p  ?= %p", (void*)mDataPointer, (void*)mDataEndPointer);
  o2::raw::RDHUtils::printRDH(rdh);

  LOG(debug) << "--- RDH close detected";

  LOG(debug) << "--- END PROCESS HBF";

  /* move to next RDH */
  // mDataPointer = (uint32_t*)((char*)(rdh) + o2::raw::RDHUtils::getOffsetToNext(rdh));

  /* otherwise return */

  return mDataEndPointer - mDataPointer;
}

int CruRawReader::DataBufferFormatIs()
{
  return TrackletsDataFormat;
}

bool CruRawReader::buildCRUPayLoad()
{
  // copy data for the current half cru, and when we eventually get to the end of the payload return 1
  // to say we are done.
  int cruid = 0;
  int additionalBytes = -1;
  int crudatasize = -1;
  LOG(debug) << "--- Build CRU Payload, added " << additionalBytes << " bytes to CRU "
             << cruid << " with new size " << crudatasize;
  return true;
}

bool CruRawReader::processHalfCRU()
{
  //given an rdh payload, read the halfcruheaders find the datablock related to the link.
  /* process a FeeID/halfcru, 15 links */
  LOG(debug) << "--- PROCESS HalfCRU FeeID:" << mFEEID;
  if (mState == CRUStateHalfCRUHeader) {
    // well then read the halfcruheader.
    memcpy(&mCurrentHalfCRUHeader, (void*)(mDataPointer), sizeof(mCurrentHalfCRUHeader)); //TODO remove the copy just use pointer dereferencing, doubt it will improve the speed much though.
    //mEncoderRDH = reinterpret_cast<o2::header::RDHAny*>(mEncoderPointer);)
    mDataPointer += sizeof(mCurrentHalfCRUHeader);
    o2::trd::getlinkdatasizes(mCurrentHalfCRUHeader, mCurrentHalfCRULinkLengths);
    o2::trd::getlinkerrorflags(mCurrentHalfCRUHeader, mCurrentHalfCRULinkErrorFlags);
    mTotalHalfCRUDataLength256 = std::accumulate(mCurrentHalfCRULinkLengths.begin(),
                                                 mCurrentHalfCRULinkLengths.end(),
                                                 decltype(mCurrentHalfCRULinkLengths)::value_type(0));
    mTotalHalfCRUDataLength = mTotalHalfCRUDataLength256 * 32; //convert to bytes.

    // we will always have at least a length of 1 fully padded for each link.
    //TODO Sanity check,1. each link is >=1, 2. link is < ?? what is the maximum link length.3. header values are sane. define sane?
    //size sanity check.
    if (mTotalHalfCRUDataLength > mMaxCRUBufferSize) {
      LOG(fatal) << "Cru wont fit in the allocated buffer  " << mTotalHalfCRUDataLength << " > " << mMaxCRUBufferSize;
    }
    LOG(debug) << "Found  a HalfCRUHeader : ";
    LOG(debug) << mCurrentHalfCRUHeader << " with payload total size of : " << mTotalHalfCRUDataLength;
    mState = CRUStateHalfChamber; // we expect a halfchamber header now
    //TODO maybe change name to something more generic, this is will have to change to handle other data types config/adcdata.
    int rdhpayloadleft = mDataEndPointer - mDataPointer;
    mHalfCRUPayLoadRead = 0;
    //now sort out if we copy the remainderof the rdh payload or only a portion of it (tillthe end off the current halfcruheader's body
    if (mTotalHalfCRUDataLength < rdhpayloadleft) {
      LOG(debug) << "read a halfcruheader at the top of the rdh payload, and it fits with in the rdh payload : " << mTotalHalfCRUDataLength << " < " << rdhpayloadleft;
      LOG(debug) << "copying from " << mDataPointer << " to " << mDataPointer + mTotalHalfCRUDataLength;
      memcpy(&mCRUPayLoad[0], (void*)(mDataPointer), sizeof(mTotalHalfCRUDataLength)); //0 as we have just read the header.
      //advance pointer to next halfcruheader.
      mDataPointer += mTotalHalfCRUDataLength; // this cru half chamber is contained with in the a single rdh payload.
      mHalfCRUPayLoadRead = mDataEndPointer - mDataPointer;
      mState = CRUStateHalfCRUHeader; // now back on a halfcruheader with in the current rdh payload.
    } else {
      //otherwise we copy till the end of the rdh payload, and place mDataPointer on the next rdh header.
      memcpy(&mCRUPayLoad[0], (void*)(mDataPointer), mDataEndPointer - mDataPointer); //0 as we have just read the header.
      mHalfCRUPayLoadRead = mDataEndPointer - mDataPointer;
      mDataPointer = mDataEndPointer;
      mState = CRUStateHalfChamber;
    }
  } // end of cruhalfchamber header at the top of rdh.
  else {
    if (mState == CRUStateHalfChamber) {
      //we are still busy inside a halfcruchamber
      //copy the remainder of the halfcruchamber or the entire rdh payload, which ever is smaller.
      //mCurrentLinkDataPosition;
      //mCurrentHalfCRULinkHeaderPosition;

      int remainderofrdhpayload = mDataEndPointer - mDataPointer;
      int remainderofcrudatablock = mTotalHalfCRUDataLength - mHalfCRUPayLoadRead;
      if (remainderofcrudatablock > remainderofrdhpayload) {
        // the halfchamber block extends past the end of this rdh we are currently on.
        // copy the data from where we are to the end into the crupayload buffer.

        int remainderofrdhpayload = mTotalHalfCRUDataLength - mHalfCRUPayLoadRead;
        LOG(debug) << "in state CRUStateHalfChamber with remainderofrdhpayload (" << remainderofrdhpayload << ") = " << mTotalHalfCRUDataLength << "-" << mHalfCRUPayLoadRead;
        memcpy(&mCRUPayLoad[mHalfCRUPayLoadRead], (void*)(mDataPointer), remainderofrdhpayload); //0 as we have just read the header.
        mDataPointer = mDataEndPointer;
        mState = CRUStateHalfChamber; // state stays the same, written here to be explicit.
      } else {
        // the current cru payload we are on finishes before the end of the current rdh block we are in.
        int remainderofrdhpayloadthatwewant = mTotalHalfCRUDataLength - mHalfCRUPayLoadRead;
        //sanity check :
        if (remainderofrdhpayloadthatwewant < (mDataEndPointer - mDataPointer)) {
          LOG(warn) << " something odd we are supposed to have the cruhalfchamber ending with in the current rdh however : remainder of the rdh payload we want is : " << remainderofrdhpayloadthatwewant << " yet the rdh block only has " << mDataEndPointer - mDataPointer << " data left";
        }
        memcpy(&mCRUPayLoad[mHalfCRUPayLoadRead], (void*)(mDataPointer), remainderofrdhpayloadthatwewant); //0 as we have just read the header.
        mDataPointer += remainderofrdhpayloadthatwewant;
        mState = CRUStateHalfCRUHeader;
      }
    } else {
      LOG(warn) << "huh unknown CRUstate of " << mState;
    }
  }

  LOG(debug) << "--- END PROCESS HalfCRU with state: " << mState;

  return true;
}

bool CruRawReader::processCRULink()
{
  /* process a CRU Link 15 per half cru */
  //  checkFeeID(); // check the link we are working with corresponds with the FeeID we have in the current rdh.
  //  uint32_t slotId = GET_TRMDATAHEADER_SLOTID(*mDataPointer);
  return false;
}

void CruRawReader::resetCounters()
{
  mEventCounter = 0;
  mFatalCounter = 0;
  mErrorCounter = 0;
}

void CruRawReader::checkSummary()
{
  char chname[2] = {'a', 'b'};

  LOG(info) << "--- SUMMARY COUNTERS: " << mEventCounter << " events "
            << " | " << mFatalCounter << " decode fatals "
            << " | " << mErrorCounter << " decode errors ";
}

} // namespace trd
} // namespace o2
