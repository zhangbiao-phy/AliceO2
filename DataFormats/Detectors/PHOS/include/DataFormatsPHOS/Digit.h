// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef ALICEO2_PHOS_DIGIT_H_
#define ALICEO2_PHOS_DIGIT_H_

#include <cmath>
#include "CommonDataFormat/TimeStamp.h"

namespace o2
{

namespace phos
{
/// \class PHOSDigit
/// \brief PHOS digit implementation
class Hit;

using DigitBase = o2::dataformats::TimeStamp<double>;
class Digit : public DigitBase
{

 public:
  static constexpr int kTimeGate = 25; // Time in ns between digits to be added as one signal.
                                       // Should it be readout time (6000 ns???): to be tested

  Digit() = default;

  /// \brief Main Digit constructor
  /// \param cell absId of a cell, amplitude energy deposited in a cell, time time measured in cell, label label of a
  /// particle in case of MC \return constructed Digit
  Digit(short cell, float amplitude, float time, int label);

  /// \brief Digit constructor from Hit
  /// \param PHOS Hit
  /// \return constructed Digit
  Digit(const Hit& hit, int label);

  ~Digit() = default; // override

  /// \brief Replace content of this digit with new one, from hit
  /// \param PHOS Hit
  /// \return
  void fillFromHit(const Hit& hit);

  /// \brief Comparison oparator, based on time and absId
  /// \param another PHOS Digit
  /// \return result of comparison: first time, if time same, then absId
  inline bool operator<(const Digit& other) const
  {
    if (fabs(getTimeStamp() - other.getTimeStamp()) < kTimeGate) {
      return getAbsId() < other.getAbsId();
    } else {
      return getTimeStamp() < other.getTimeStamp();
    }
  }

  /// \brief Comparison oparator, based on time and absId
  /// \param another PHOS Digit
  /// \return result of comparison: first time, if time same, then absId
  inline bool operator>(const Digit& other) const
  {
    if (fabs(getTimeStamp() - other.getTimeStamp()) <= kTimeGate) {
      return getAbsId() > other.getAbsId();
    } else {
      return getTimeStamp() > other.getTimeStamp();
    }
  }

  /// \brief Comparison oparator, based on time and absId
  /// \param another PHOS Digit
  /// \return result of comparison: first time, if time same, then absId
  inline bool operator==(const Digit& other) const
  {
    return ((fabs(getTimeStamp() - other.getTimeStamp()) <= kTimeGate) &&
            getAbsId() == other.getAbsId());
  }

  /// \brief Check, if one can add two digits
  /// \param another PHOS Digit
  /// \return true if time stamps are same and absId are same
  bool canAdd(const Digit other) const;
  /// \brief if addable, adds energy and list of primaries.
  /// \param another PHOS Digit
  /// \return digit with sum of energies and longer list of primaries
  Digit& operator+=(const Digit& other); //

  void addEnergyTime(float energy, float time);

  /// \brief Absolute sell id
  short getAbsId() const { return mAbsId; }
  void setAbsId(short cellId) { mAbsId = cellId; }

  /// \brief Energy deposited in a cell
  float getAmplitude() const { return mAmplitude; }
  void setAmplitude(float amplitude) { mAmplitude = amplitude; }

  /// \brief time measured in digit w.r.t. photon to PHOS arrival
  float getTime() const { return mTime; }
  void setTime(float time) { mTime = time; }

  /// \brief Checks if this digit is produced in High Gain or Low Gain channels
  bool isHighGain() const { return mIsHighGain; }
  void setHighGain(Bool_t isHG) { mIsHighGain = isHG; }

  /// \brief index of entry in MCLabels array
  /// \return ndex of entry in MCLabels array
  int getLabel() const { return mLabel; }
  void setLabel(int l) { mLabel = l; }

  void reset()
  {
    mIsHighGain = true;
    mAbsId = 0;
    mLabel = -1;
    mAmplitude = 0;
    mTime = 0;
  }

  void PrintStream(std::ostream& stream) const;

 private:
  // friend class boost::serialization::access;

  bool mIsHighGain = true; ///< High Gain or Low Gain channel (for calibration)
  short mAbsId = 0;        ///< cell index (absolute cell ID)
  int mLabel = -1;         ///< Index of the corresponding entry/entries in the MC label array
  float mAmplitude = 0;    ///< Amplitude
  float mTime = 0.;        ///< Time

  ClassDefNV(Digit, 1);
};

std::ostream& operator<<(std::ostream& stream, const Digit& dig);
} // namespace phos
} // namespace o2
#endif
