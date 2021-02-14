/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#ifndef ACE_TIME_LINK_REGISTRAR_H
#define ACE_TIME_LINK_REGISTRAR_H

#include <stdint.h>
#include <AceCommon.h> // KString, binarySearchByKey(), isSortedByKey()
#include "../common/compat.h" // ACE_TIME_USE_PROGMEM
#include "LinkEntry.h"
#include "BasicBrokers.h"
#include "ExtendedBrokers.h"

class LinkRegistrarTest_isSorted;

namespace ace_time {
namespace internal {

/**
 * Class that allows looking up the LinkEntry (LE) from its LinkRegistry (LRGB)
 * using its linkId.
 *
 * @tparam LE LinkEntry type (e.g. basic::LinkEntry)
 * @tparam LEB LinkEntryBroker type (e.g. basic::LinkEntryBroker)
 * @tparam LRGB LinkRegistryBroker type (e.g. basic::LinkRegistryBroker)
 */
template<typename LE, typename LEB, typename LRGB>
class LinkRegistrarTemplate {
  public:
    /** Invalid index to indicate error or not found. */
    static const uint16_t kInvalidIndex = 0xffff;

    /** Constructor. */
    LinkRegistrarTemplate(
        uint16_t linkRegistrySize = 0,
        const LE* linkRegistry = nullptr
    ):
        mLinkRegistrySize(linkRegistrySize),
        mLinkRegistry(linkRegistry),
        mIsSorted(isSorted(linkRegistry, linkRegistrySize))
    {}

    /** Return the number of (thin) links. */
    uint16_t linkRegistrySize() const { return mLinkRegistrySize; }

    /** Return the LinkEntry at index i. Return nullptr if i is out of range. */
    const LE* getLinkEntryForIndex(uint16_t i) const {
      return (mLinkRegistry && i < mLinkRegistrySize)
          ? LRBG(mLinkRegistry).linkEntry(i)
          : nullptr;
    }

    /** Return the LinkEntry using the linkId. Return nullptr if not found. */
    const LE* getLinkEntryForId(uint32_t linkId) const {
      uint16_t index = findIndexForId(linkId);
      if (index == kInvalidIndex) return nullptr;
      return LRGB(mLinkRegistry).linkEntry(index);
    }

    /** Find the index for linkId. Return kInvalidIndex if not found. */
    uint16_t findIndexForId(uint32_t linkId) const {
      if (mIsSorted && mLinkRegistrySize >= kBinarySearchThreshold) {
        return binarySearchById(mLinkRegistry, mLinkRegistrySize, linkId);
      } else {
        return linearSearchById(mLinkRegistry, mLinkRegistrySize, linkId);
      }
    }

  protected:
    friend class ::LinkRegistrarTest_isSorted;

    /** Use binarySearchById() if linkRegistrySize >= threshold. */
    static const uint8_t kBinarySearchThreshold = 8;

    /** Determine if the given link registry is sorted by id. */
    static bool isSorted(const LE* registry, uint16_t registrySize) {
      const LRGB linkRegistry(registry);
      return ace_common::isSortedByKey(
          (size_t) registrySize,
          [&linkRegistry](size_t i) {
            return LEB(linkRegistry.linkEntry((uint16_t) i)).linkId();
          }
      );
    }

    /**
     * Find the registry index corresponding to linkId using linear search.
     * Returns kInvalidIndex if not found.
     */
    static uint16_t linearSearchById(const LE* registry,
        uint16_t registrySize, uint32_t linkId) {
      const LRGB linkRegistry(registry);
      for (uint16_t i = 0; i < registrySize; ++i) {
        const LE* linkEntry = linkRegistry.linkEntry(i);
        if (linkId == LEB(linkEntry).linkId()) {
          return i;
        }
      }
      return kInvalidIndex;
    }

    /**
     * Find the registry index corresponding to linkId using a binary search.
     * Returns kInvalidIndex if not found.
     *
     * The largest registrySize is UINT16_MAX so the largest valid index is
     * UINT16_MAX - 1. This allows us to set kInvalidIndex to UINT16_MAX to
     * indicate "Not Found".
     */
    static uint16_t binarySearchById(const LE* registry,
        uint16_t registrySize, uint32_t linkId) {
      const LRGB linkRegistry(registry);
      return (uint16_t) ace_common::binarySearchByKey(
          (size_t) registrySize,
          linkId,
          [&linkRegistry](size_t i) -> uint32_t {
            const LE* linkEntry = linkRegistry.linkEntry(i);
            return LEB(linkEntry).linkId();
          } // lambda expression returns linkId at index i
      );
    }

    /** Exposed only for benchmarking purposes. */
    uint16_t findIndexForIdLinear(uint32_t linkId) const {
      return linearSearchById(mLinkRegistry, mLinkRegistrySize, linkId);
    }

    /** Exposed only for benchmarking purposes. */
    uint16_t findIndexForIdBinary(uint32_t linkId) const {
      return binarySearchById(mLinkRegistry, mLinkRegistrySize, linkId);
    }

  private:
    // Ordering of fields optimized for 32-bit alignment.
    uint16_t const mLinkRegistrySize;
    const LE* const mLinkRegistry; // nullable
    bool const mIsSorted;
};

} // internal

namespace basic {

#if 1

/**
 * Concrete template instantiation of LinkRegistrarTemplate for
 * basic::LinkEntry.
 */
class LinkRegistrar: public internal::LinkRegistrarTemplate<
    basic::LinkEntry,
    basic::LinkEntryBroker,
    basic::LinkRegistryBroker
> {
  public:
    LinkRegistrar(
        uint16_t linkRegistrySize,
        const basic::LinkEntry* linkRegistry
    ) :
        internal::LinkRegistrarTemplate<
            basic::LinkEntry,
            basic::LinkEntryBroker,
            basic::LinkRegistryBroker
        >(linkRegistrySize, linkRegistry)
    {}
};

#else

// Use subclassing instead of template typedef so that error messages are
// understandable. The compiler seems to optimize away the subclass overhead.

typedef internal::LinkRegistrarTemplate<
    basic::LinkEntry,
    basic::LinkRegistryBroker,
    basic::LinkEntryBroker
>
    LinkRegistrar;

#endif

} // basic

namespace extended {

#if 1

/**
 * Concrete template instantiation of LinkRegistrarTemplate for
 * extended::LinkEntry.
 */
class LinkRegistrar: public internal::LinkRegistrarTemplate<
    extended::LinkEntry,
    extended::LinkEntryBroker,
    extended::LinkRegistryBroker
> {
  public:
    LinkRegistrar(
        uint16_t linkRegistrySize,
        const extended::LinkEntry* linkRegistry
    ) :
        internal::LinkRegistrarTemplate<
            extended::LinkEntry,
            extended::LinkEntryBroker,
            extended::LinkRegistryBroker
        >(linkRegistrySize, linkRegistry)
    {}
};

#else

typedef internal::LinkRegistrarTemplate<
    extended::LinkEntry,
    extended::LinkRegistryBroker,
    extended::LinkEntryBroker
>
    LinkRegistrar;

#endif

} // extended

} // ace_time

#endif
