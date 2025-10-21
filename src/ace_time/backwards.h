/*
 * MIT License
 * Copyright (c) 2025 Brian T. Park
 */

/**
 * @file backwards.h
 *
 * Provide backwards compatibility macros for the renaming of:
 *
 *  - LocalDate -> PlainDate
 *  - LocalTime -> PlainTime
 *  - LocalDateTime -> PlainDateTime
 *
 * Various methods within classes have backwards compatible shims that forward
 * to the new versions:
 *
 *  - ZonedDateTime::localDateTime()
 *  - ZonedDateTime::forLocalDateTime()
 *  - ZonedExtra::forLocalDateTime()
 *  - OffsetDateTime::localDateTime()
 *  - OffsetDateTime::localDate()
 *  - OffsetDateTime::localTime()
 *  - PlainDateTime::localDate()
 *  - PlainDateTime::localTime()
 *
 * In theory, the `zonedb` database and namespace should have been renamed to
 * `zonedb2000` (for symmetry with `zonedb2025`). But I could not figure out a
 * way provide a backwards compatible shim. So we have live with 'zonedb`.
 */

#ifndef ACE_TIME_BACKWARDS_H
#define ACE_TIME_BACKWARDS_H

#define LocalDate PlainDate
#define LocalDateTime PlainDateTime
#define LocalTime PlainTime

#endif
