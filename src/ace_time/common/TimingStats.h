/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TIMING_STATS_H
#define ACE_TIME_TIMING_STATS_H

#include <stdint.h>

class Print;

namespace ace_time {
namespace common {

/**
 * Helper class to collect timing statistics such as min, max, average.
 */
class TimingStats {
  public:
    /** Constructor. Default copy-constructor and assignment operator ok. */
    TimingStats(): mCounter(0) {
      reset();
    }

    void reset() {
      mExpDecayAvg = 0;
      mMin = UINT16_MAX;
      mMax = 0;
      mSum = 0;
      mCount = 0;
    }

    uint16_t getMax() const { return mMax; }

    uint16_t getMin() const { return mMin; }

    uint16_t getAvg() const { return (mCount > 0) ? mSum / mCount : 0; }

    /** An exponential decay average. */
    uint16_t getExpDecayAvg() const { return mExpDecayAvg; }

    /** Number of times update() was called since last reset. */
    uint16_t getCount() const { return mCount; }

    /**
     * Number of times update() was called from the beginning of time. Never
     * reset. This is useful to determining how many times update() was called
     * since it was last checked from the client code.
     */
    uint16_t getCounter() const { return mCounter; }

    void update(uint16_t duration) {
      mCount++;
      mCounter++;
      mSum += duration;
      if (duration < mMin) {
        mMin = duration;
      }
      if (duration > mMax) {
        mMax = duration;
      }
      mExpDecayAvg = (mExpDecayAvg + duration) / 2;
    }

  private:
    uint16_t mExpDecayAvg;
    uint16_t mMin;
    uint16_t mMax;
    uint32_t mSum;
    uint16_t mCount;
    uint16_t mCounter;
};

}
}

#endif
