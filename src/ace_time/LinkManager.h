/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#ifndef ACE_TIME_LINK_MANAGER_H
#define ACE_TIME_LINK_MANAGER_H

#include "internal/LinkRegistrar.h"
#include "internal/BasicBrokers.h"
#include "internal/ExtendedBrokers.h"

namespace ace_time {

/**
 * Base class for BasicLinkManager and ExtendedLinkManager mostly for backwards
 * compatibility so that kInvalidZoneId can be referenced as
 * `LinkManager::kInvalidZoneId`. No virtual methods are used to reduce flash
 * memory usage, especially on 8-bit processors.
 */
class LinkManager {
  public:
    /**
     * ZoneId which is not valid to indicates an error or not found. The
     * AceTimeTools project will never generate a zoneId of 0.
     */
    static const uint32_t kInvalidZoneId = 0;
};

/**
 * An implementation of the LinkManager which uses a registry of basic::ZoneInfo
 * records.
 */
class BasicLinkManager: public LinkManager {
  public:
    /**
     * Constructor.
     * @param linkRegistrySize number of LinkEntry entries in linkRegistry
     * @param linkRegistry an array of LinkEntry entries
     */
    BasicLinkManager(
        uint16_t linkRegistrySize,
        const basic::LinkEntry* linkRegistry
    ) :
        mLinkRegistrar(linkRegistrySize, linkRegistry)
    {}

    uint32_t zoneIdForLinkId(uint32_t linkId) const {
      const basic::LinkEntry* linkEntry =
          mLinkRegistrar.getLinkEntryForId(linkId);
      if (! linkEntry) return kInvalidZoneId;
      return basic::LinkEntryBroker(linkEntry).zoneId();
    }

    uint16_t linkRegistrySize() const {
      return mLinkRegistrar.linkRegistrySize();
    }

  private:
    // disable copy constructor and assignment operator
    BasicLinkManager(const BasicLinkManager&) = delete;
    BasicLinkManager& operator=(const BasicLinkManager&) = delete;

    const basic::LinkRegistrar mLinkRegistrar;
};

/**
 * An implementation of the LinkManager which uses a registry of
 * extended::ZoneInfo records.
 */
class ExtendedLinkManager: public LinkManager {
  public:
    /**
     * Constructor.
     * @param linkRegistrySize number of LinkEntry entries in linkRegistry
     * @param linkRegistry an array of LinkEntry entries
     */
    ExtendedLinkManager(
        uint16_t linkRegistrySize,
        const extended::LinkEntry* linkRegistry
    ) :
        mLinkRegistrar(linkRegistrySize, linkRegistry)
    {}

    uint32_t zoneIdForLinkId(uint32_t linkId) const {
      const extended::LinkEntry* linkEntry =
          mLinkRegistrar.getLinkEntryForId(linkId);
      if (! linkEntry) return kInvalidZoneId;
      return extended::LinkEntryBroker(linkEntry).zoneId();
    }

    uint16_t linkRegistrySize() const {
      return mLinkRegistrar.linkRegistrySize();
    }

  private:
    // disable copy constructor and assignment operator
    ExtendedLinkManager(const ExtendedLinkManager&) = delete;
    ExtendedLinkManager& operator=(const ExtendedLinkManager&) = delete;

    const extended::LinkRegistrar mLinkRegistrar;
};

}

#endif
