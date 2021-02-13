/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stdint.h>

/**
 * A LINK to ZONE mapping, using the zoneId/linkId hash key.
 */
struct LinkEntry {
  /** Hash id of the source LINK name (e.g. "US/Pacific"). */
  uint32_t const linkId;

  /** Hash id of the target ZONE name (e.g. "America/Los_Angeles"). */
  uint32_t const zoneId;
};
