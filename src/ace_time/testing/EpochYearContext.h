/*
 * MIT License
 * Copyright (c) 2022 Brian T. Park
 */

#ifndef ACE_TIME_EPOCH_YEAR_CONTEXT_H
#define ACE_TIME_EPOCH_YEAR_CONTEXT_H

#include <stdint.h> // int16_t
#include "../Epoch.h"

namespace ace_time {
namespace testing {

/**
 * A helper class that saves the current epoch year, then switches the epoch
 * year to a different year. The new epoch year is set when an instance of this
 * class is created through the constructor. When the object goes out of scope,
 * the destructor is automatically called which switches the epoch year to the
 * previous value. This uses the RAII (Resource Acquisition Is Initialization)
 * pattern. The advantage of using this helper class instead of invoking
 * `Epoch::currentEpochYear()` manually is that the compiler guarantees that
 * the destructor is always called, so that previous epoch year is always
 * restored.
 */
class EpochYearContext {
  public:
    /**
     * Construtor. Saves the current epoch year to an internal member variable,
     * then calls `Epoch::currentEpochYear(year)` to set the new epoch year.
     */
    EpochYearContext(int16_t year) {
      mSavedEpochYear = Epoch::currentEpochYear();
      Epoch::currentEpochYear(year);
    }

    /**
     * Destructor. Calls `Epoch::currentEpochYear(savedYear)` to restore
     * the previous epoch year.
     */
    ~EpochYearContext() {
      Epoch::currentEpochYear(mSavedEpochYear);
    }

  private:
    int16_t mSavedEpochYear;
};

}
}

#endif
