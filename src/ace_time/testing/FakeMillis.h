/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_FAKE_MILLIS_H
#define ACE_TIME_FAKE_MILLIS_H

namespace ace_time {
namespace testing {

/** A class that allows injection of the millis() function. */
class FakeMillis {
  public:
    unsigned long millis() const { return mMillis; }

    void millis(unsigned long millis) { mMillis = millis; }

  private:
    unsigned long mMillis = 0;
};

}
}

#endif
