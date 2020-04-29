/*
MIT License

Copyright (c) 2020 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ACE_TIME_CSTR_PRINT_H
#define ACE_TIME_CSTR_PRINT_H

#include <stddef.h> // size_t
#include <Print.h>

namespace ace_time {

/**
 * Base class for all the CstrPrint<SIZE> classes. This was created to make sure
 * that there is only one copy of the core code across all possible template
 * instances. Otherwise, the same code gets duplicated for each template
 * instance since CstrPrint<10> is a different class than CstrPrint<20>.
 *
 * Here are the actual numbers. I modified tests/CommonTest.ino program to use
 * 2 template instances, CstrPrint<10> and CstrPrint<20> instead of just
 * CstrPrint<10>. Without this base class optimization, the sketch uses:
 *
 *    * 10030 bytes (32%) of program storage space,
 *    * 710 bytes (34%) of dynamic memory on an Arduino Nano.
 *
 * After inserting this CstrPrintBase class in the class hierarchy, the same
 * sketch uses:
 *
 *    * 9936 bytes (32%) of program storage space,
 *    * 698 bytes (34%) of dynamic memory,
 *
 * So we save 94 bytes of flash memory, and 12 bytes of static RAM. And even
 * better, the program and RAM size was the same as using 2 CstrPrint<10>
 * instances. In other words, the amount of flash and static RAM remains
 * constant no matter how many template instances we create.
 */
class CstrPrintBase: public Print {
  public:
    CstrPrintBase(uint8_t size, char* buf):
        mSize(size),
        mBuf(buf) {}

    size_t write(uint8_t c) override {
      if (mIndex < mSize - 1) {
        mBuf[mIndex] = c;
        mIndex++;
        return 1;
      } else {
        return 0;
      }
    }

    size_t write(const uint8_t *buffer, size_t size) override {
      if (buffer == nullptr) return 0;

      while (size > 0 && mIndex < mSize - 1) {
        write(*buffer++);
        size--;
      }
      return size;
    }

// ESP32 version of Print class does not define a virtual flush() method.
#ifdef ESP32
    void flush() {
      mIndex = 0;
    }
#else
    void flush() override {
      mIndex = 0;
    }
#endif

    /**
     * Return the NUL terminated c-string buffer. After the buffer is no longer
     * needed, the flush() method should be called to reset the internal buffer
     * index to 0.
     */
    const char* getCstr() const {
      mBuf[mIndex] = '\0';
      return mBuf;
    }

  private:
    uint8_t const mSize;
    uint8_t mIndex = 0;

    // This is the pointer to mActualBuf which is defined in the subclasses.
    // Instead of storing it (and taking up precious RAM), maybe there's a way
    // to calculate this pointer by simply extending the pointer from the last
    // element of this object (i.e. &mIndex), and thereby saving some memory.
    // But I am not convinced that I have the knowledge do that properly
    // without triggering UB (undefined behavior) in the C++ language. Do I
    // define mBuf[0] here and will it point exactly where mActualBuf[] is
    // allocated? Or do I use [&CstrPrintBase + sizeof(CstrPrintBase)] to
    // calculate the pointer to mActualBuf. I just don't know what's actually
    // allowed in the language spec versus something that works by pure luck on
    // a particular microcontroller and compiler. So I'll pay the cost of the 2
    // extra bytes (8-bit) or 4 extra bytes (32-bit processors) of RAM and
    // store the pointer to mActualBuf explicitly here in the base class.
    char* const mBuf;
};

/**
 * An implementation of `Print` that writes to an in-memory buffer. The
 * NUL-terminated c-string representation can be retrieved using `getCstr()`.
 * This object is intended to allow a c-string to be extract from various Date
 * and DateTime classes which provide a printTo(Print&) method.
 *
 * This object is expected to be created on the stack. The desired information
 * will be written into it, and then extracted using the `getCstr()`. The
 * object will be destroyed automatically when the stack is unwound after
 * returning from the function where this is used. It is possible to create an
 * instance of this object statically and reused across different calling
 * sites, but the programmer must handle the memory management properly.
 *
 * Warning: The contents of `getCstr()` is valid only as long as the CstrPrint
 * object is alive. It should never be passed to another part of the program if
 * the getCstr() pointer outlives its underlying CstrPrint object.
 *
 * Usage:
 *
 * @verbatim
 * #include <AceTime.h>
 *
 * using namespace acetime;
 *
 * {
 *   TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHours(-8));
 *   ZonedDateTime dt = ZonedDateTime::forComponents(
 *       2018, 3, 11, 1, 59, 59, tz);
 *
 *   CstrPrint<32> cstrPrint;
 *   dt.printTo(cstrPrint);
 *   const char* cstr = cstrPrint.getCstr();
 *
 *   // do stuff with cstr
 *   // ...
 *
 *   cstrPrint.flush(); // needed only if this will be used again
 * }
 * @endverbatim
 *
 * @tparam SIZE size of internal string buffer including the NUL terminator
 *         character
 */
template <uint8_t SIZE>
class CstrPrint: public CstrPrintBase {
  public:
    CstrPrint(): CstrPrintBase(SIZE, mActualBuf) {}

  private:
    char mActualBuf[SIZE];
};

}

#endif
