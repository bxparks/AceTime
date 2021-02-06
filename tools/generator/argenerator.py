# Copyright 2018 Brian T. Park
#
# MIT License
"""
Generate the zone_info and zone_policies files for Arduino.
"""

import os
import logging
from typing import Dict
from typing import List
from typing import Optional
from typing import Tuple
from data_types.at_types import ZoneRuleRaw
from data_types.at_types import ZoneEraRaw
from data_types.at_types import ZonesMap
from data_types.at_types import PoliciesMap
from data_types.at_types import LinksMap
from data_types.at_types import CommentsMap
from data_types.at_types import IndexMap
from data_types.at_types import LettersPerPolicy
from data_types.at_types import ZoneInfoDatabase
from data_types.at_types import BufSizeMap
from transformer.transformer import normalize_name
from transformer.transformer import normalize_raw


class ArduinoGenerator:
    """Generate zone_infos and zone_policies files for Arduino/C++.
    """
    ZONE_INFOS_H_FILE_NAME = 'zone_infos.h'
    ZONE_INFOS_CPP_FILE_NAME = 'zone_infos.cpp'
    ZONE_POLICIES_H_FILE_NAME = 'zone_policies.h'
    ZONE_POLICIES_CPP_FILE_NAME = 'zone_policies.cpp'
    ZONE_REGISTRY_H_FILE_NAME = 'zone_registry.h'
    ZONE_REGISTRY_CPP_FILE_NAME = 'zone_registry.cpp'

    def __init__(
        self,
        invocation: str,
        db_namespace: str,
        zidb: ZoneInfoDatabase,
    ):
        # If I add a backslash (\) at the end of each line (which is needed if I
        # want to copy and paste the shell command), the C++ compiler spews out
        # warnings about "multi-line comment [-Wcomment]".
        wrapped_invocation = '\n//     --'.join(invocation.split(' --'))
        wrapped_tzfiles = '\n//   '.join(zidb['tz_files'])

        # Determine zonedb C++ namespace
        scope = zidb['scope']
        if not db_namespace:
            if scope == 'basic':
                db_namespace = 'zonedb'
            elif scope == 'extended':
                db_namespace = 'zonedbx'
            else:
                raise Exception(
                    f"db_namespace cannot be determined for scope '{scope}'"
                )

        self.zone_policies_generator = ZonePoliciesGenerator(
            invocation=wrapped_invocation,
            tz_files=wrapped_tzfiles,
            db_namespace=db_namespace,
            tz_version=zidb['tz_version'],
            scope=zidb['scope'],
            zones_map=zidb['zones_map'],
            policies_map=zidb['policies_map'],
            removed_zones=zidb['removed_zones'],
            removed_policies=zidb['removed_policies'],
            notable_zones=zidb['notable_zones'],
            notable_policies=zidb['notable_policies'],
            letters_per_policy=zidb['letters_per_policy'],
            letters_map=zidb['letters_map'],
        )
        self.zone_infos_generator = ZoneInfosGenerator(
            invocation=wrapped_invocation,
            tz_files=wrapped_tzfiles,
            db_namespace=db_namespace,
            tz_version=zidb['tz_version'],
            scope=zidb['scope'],
            start_year=zidb['start_year'],
            until_year=zidb['until_year'],
            zones_map=zidb['zones_map'],
            links_map=zidb['links_map'],
            policies_map=zidb['policies_map'],
            removed_zones=zidb['removed_zones'],
            removed_links=zidb['removed_links'],
            removed_policies=zidb['removed_policies'],
            notable_zones=zidb['notable_zones'],
            notable_links=zidb['notable_links'],
            notable_policies=zidb['notable_policies'],
            buf_sizes=zidb['buf_sizes'],
            zone_ids=zidb['zone_ids'],
            link_ids=zidb['link_ids'],
            formats_map=zidb['formats_map'],
            fragments_map=zidb['fragments_map'],
            compressed_names=zidb['compressed_names'],
        )
        self.zone_registry_generator = ZoneRegistryGenerator(
            invocation=wrapped_invocation,
            tz_files=wrapped_tzfiles,
            db_namespace=db_namespace,
            tz_version=zidb['tz_version'],
            scope=zidb['scope'],
            zones_map=zidb['zones_map'],
            links_map=zidb['links_map'],
            zone_ids=zidb['zone_ids'],
            link_ids=zidb['link_ids'],
        )

    def generate_files(self, output_dir: str) -> None:
        # zone_policies.*
        self._write_file(output_dir, self.ZONE_POLICIES_H_FILE_NAME,
                         self.zone_policies_generator.generate_policies_h())
        self._write_file(output_dir, self.ZONE_POLICIES_CPP_FILE_NAME,
                         self.zone_policies_generator.generate_policies_cpp())

        # zone_infos.*
        self._write_file(output_dir, self.ZONE_INFOS_H_FILE_NAME,
                         self.zone_infos_generator.generate_infos_h())
        self._write_file(output_dir, self.ZONE_INFOS_CPP_FILE_NAME,
                         self.zone_infos_generator.generate_infos_cpp())

        # zone_registry.*
        self._write_file(output_dir, self.ZONE_REGISTRY_H_FILE_NAME,
                         self.zone_registry_generator.generate_registry_h())
        self._write_file(output_dir, self.ZONE_REGISTRY_CPP_FILE_NAME,
                         self.zone_registry_generator.generate_registry_cpp())

    def _write_file(self, output_dir: str, filename: str, content: str) -> None:
        full_filename = os.path.join(output_dir, filename)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            print(content, end='', file=output_file)
        logging.info("Created %s", full_filename)


class ZonePoliciesGenerator:

    ZONE_POLICIES_H_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files
//
//   {tz_files}
//
// from https://github.com/eggert/tz/releases/tag/{tz_version}
//
// DO NOT EDIT

#ifndef ACE_TIME_{dbHeaderNamespace}_ZONE_POLICIES_H
#define ACE_TIME_{dbHeaderNamespace}_ZONE_POLICIES_H

#include <ace_time/internal/ZonePolicy.h>

namespace ace_time {{
namespace {dbNamespace} {{

//---------------------------------------------------------------------------
// Supported zone policies: {numPolicies}
//
{policyItems}

//---------------------------------------------------------------------------
// Unsupported zone policies: {numRemovedPolicies}
//
{removedPolicyItems}

// Notable zone policies: {numNotablePolicies}
//
{notablePolicyItems}

}}
}}

#endif
"""

    ZONE_POLICIES_CPP_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files
//
//   {tz_files}
//
// from https://github.com/eggert/tz/releases/tag/{tz_version}
//
// Policies: {numPolicies}
// Rules: {numRules}
// Letter Size (bytes): {letterSize}
// Total Memory 8-bit (bytes): {memory8}
// Total Memory 32-bit (bytes): {memory32}
//
// DO NOT EDIT

#include <ace_time/common/compat.h>
#include "zone_policies.h"

namespace ace_time {{
namespace {dbNamespace} {{

{policyItems}

}}
}}
"""

    ZONE_POLICIES_CPP_POLICY_ITEM = """\
//---------------------------------------------------------------------------
// Policy name: {policyName}
// Rules: {numRules}
// Memory (8-bit): {memory8}
// Memory (32-bit): {memory32}
//---------------------------------------------------------------------------

static const {scope}::ZoneRule kZoneRules{policyName}[] {progmem} = {{
{ruleItems}
}};

{letterArray}

const {scope}::ZonePolicy kPolicy{policyName} {progmem} = {{
  kZoneRules{policyName} /*rules*/,
  {letterArrayRef} /*letters*/,
  {numRules} /*numRules*/,
  {numLetters} /*numLetters*/,
}};

"""

    SIZEOF_ZONE_RULE_8 = 9
    SIZEOF_ZONE_RULE_32 = 12  # 9 rounded to 4-byte alignment
    SIZEOF_ZONE_POLICY_8 = 6
    SIZEOF_ZONE_POLICY_32 = 12  # 10 rounded to 4-byte alignment

    def __init__(
        self,
        invocation: str,
        tz_version: str,
        tz_files: str,
        scope: str,
        db_namespace: str,
        zones_map: ZonesMap,
        policies_map: PoliciesMap,
        removed_zones: CommentsMap,
        removed_policies: CommentsMap,
        notable_zones: CommentsMap,
        notable_policies: CommentsMap,
        letters_per_policy: LettersPerPolicy,
        letters_map: IndexMap,
    ):
        self.invocation = invocation
        self.tz_version = tz_version
        self.tz_files = tz_files
        self.scope = scope
        self.db_namespace = db_namespace
        self.zones_map = zones_map
        self.policies_map = policies_map
        self.removed_zones = removed_zones
        self.removed_policies = removed_policies
        self.notable_zones = notable_zones
        self.notable_policies = notable_policies
        self.letters_per_policy = letters_per_policy
        self.letters_map = letters_map

        self.db_header_namespace = self.db_namespace.upper()

    def generate_policies_h(self) -> str:
        ZONE_POLICIES_H_POLICY_ITEM = """\
extern const {scope}::ZonePolicy kPolicy{policyName};
"""
        policy_items = ''
        for name, rules in sorted(self.policies_map.items()):
            policy_items += ZONE_POLICIES_H_POLICY_ITEM.format(
                policyName=normalize_name(name),
                scope=self.scope)

        ZONE_POLICIES_H_REMOVED_POLICY_ITEM = """\
// kPolicy{policyName} ({policyReason})
"""
        removed_policy_items = ''
        for name, reasons in sorted(self.removed_policies.items()):
            removed_policy_items += ZONE_POLICIES_H_REMOVED_POLICY_ITEM.format(
                policyName=name,
                policyReason=', '.join(reasons))

        ZONE_POLICIES_H_NOTABLE_POLICY_ITEM = """\
// kPolicy{policyName} ({policyReason})
"""
        notable_policy_items = ''
        for name, reasons in sorted(self.notable_policies.items()):
            notable_policy_items += ZONE_POLICIES_H_NOTABLE_POLICY_ITEM.format(
                policyName=name,
                policyReason=', '.join(reasons))

        return self.ZONE_POLICIES_H_FILE.format(
            invocation=self.invocation,
            tz_files=self.tz_files,
            tz_version=self.tz_version,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numPolicies=len(self.policies_map),
            policyItems=policy_items,
            numRemovedPolicies=len(self.removed_policies),
            removedPolicyItems=removed_policy_items,
            numNotablePolicies=len(self.notable_policies),
            notablePolicyItems=notable_policy_items)

    def generate_policies_cpp(self) -> str:
        policy_items = ''
        memory8 = 0
        memory32 = 0
        num_rules = 0
        for name, rules in sorted(self.policies_map.items()):
            indexed_letters: Optional[IndexMap] = \
                self.letters_per_policy.get(name)
            num_rules += len(rules)
            policy_item, policy_memory8, policy_memory32 = \
                self._generate_policy_item(name, rules, indexed_letters)
            policy_items += policy_item
            memory8 += policy_memory8
            memory32 += policy_memory32

        num_policies = len(self.policies_map)
        letter_size = sum([
            len(letter) + 1 for letter in self.letters_map.keys()
        ])

        return self.ZONE_POLICIES_CPP_FILE.format(
            invocation=self.invocation,
            tz_files=self.tz_files,
            tz_version=self.tz_version,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numPolicies=num_policies,
            numRules=num_rules,
            letterSize=letter_size,
            memory8=memory8,
            memory32=memory32,
            policyItems=policy_items)

    def _generate_policy_item(
        self,
        name: str,
        rules: List[ZoneRuleRaw],
        indexed_letters: Optional[IndexMap],
    ) -> Tuple[str, int, int]:
        ZONE_POLICIES_CPP_RULE_ITEM = """\
  // {raw_line}
  {{
    {from_year_tiny} /*fromYearTiny*/,
    {to_year_tiny} /*toYearTiny*/,
    {in_month} /*inMonth*/,
    {on_day_of_week} /*onDayOfWeek*/,
    {on_day_of_month} /*onDayOfMonth*/,
    {at_time_code} /*atTimeCode*/,
    {at_time_modifier} /*atTimeModifier ({at_time_modifier_comment})*/,
    {delta_code} /*deltaCode ({delta_code_comment})*/,
    {letter} /*letter{letterComment}*/,
  }},
"""
        ZONE_POLICIES_LETTER_ARRAY = """\
static const char* const kLetters{policyName}[] {progmem} = {{
{letterItems}
}};
"""

        # Generate kZoneRules*[]
        rule_items = ''
        for rule in rules:
            at_time_code = rule['at_time_code']
            at_time_modifier = rule['at_time_modifier']
            at_time_modifier_comment = _get_time_modifier_comment(
                time_seconds=rule['at_seconds_truncated'],
                suffix=rule['at_time_suffix'],
            )
            delta_code = rule['delta_code_encoded']
            delta_code_comment = _get_rule_delta_code_comment(
                delta_seconds=rule['delta_seconds_truncated'],
                scope=self.scope,
            )
            from_year_tiny = rule['from_year_tiny']
            to_year_tiny = rule['to_year_tiny']

            # Single-character 'letter' values are represented as themselves
            # using the C++ 'char' type ('A'-'Z'). But some 'letter' fields hold
            # a multi-character string. We can encode these multi-character
            # strings as an index into an array of NUL-terminated strings.
            # ASCII codes less than 32 (space) are non-printable control
            # characters so they will not collide with the printable characters
            # 'A' - 'Z'. Therefore we can hold to up to 31 multi-character
            # strings per-zone. In practice, for a single zone, the maximum
            # number of multi-character strings that I've seen is 2.
            letter = rule['letter']
            if len(letter) == 1:
                letterComment = ''
                letter = f"'{letter}'"
            elif len(letter) > 1:
                letterComment = f' (index to "{letter}")'
                letter = str(rule['letter_index_per_policy'])
            else:
                raise Exception(
                    'len(%s) == 0; should not happen'
                    % rule['letter'])

            rule_items += ZONE_POLICIES_CPP_RULE_ITEM.format(
                raw_line=normalize_raw(rule['raw_line']),
                from_year_tiny=from_year_tiny,
                to_year_tiny=to_year_tiny,
                in_month=rule['in_month'],
                on_day_of_week=rule['on_day_of_week'],
                on_day_of_month=rule['on_day_of_month'],
                at_time_code=at_time_code,
                at_time_modifier=at_time_modifier,
                at_time_modifier_comment=at_time_modifier_comment,
                delta_code=delta_code,
                delta_code_comment=delta_code_comment,
                letter=letter,
                letterComment=letterComment)

        # Generate kLetters*[]
        policy_name = normalize_name(name)
        num_letters = len(indexed_letters) if indexed_letters else 0
        memory_letters8 = 0
        memory_letters32 = 0
        if num_letters:
            assert indexed_letters is not None
            letter_array_ref = f'kLetters{policy_name}'
            letterItems = ''
            for name, index in indexed_letters.items():
                letterItems += f'  /*{index}*/ "{name}",\n'
                memory_letters8 += len(name) + 1 + 2  # NUL terminated
                memory_letters32 += len(name) + 1 + 4  # NUL terminated
            letter_array = ZONE_POLICIES_LETTER_ARRAY.format(
                policyName=policy_name,
                letterItems=letterItems,
                progmem='ACE_TIME_PROGMEM')
        else:
            letter_array_ref = 'nullptr'
            letter_array = ''

        # Calculate the memory consumed by structs and arrays
        num_rules = len(rules)
        memory8 = (
            1 * self.SIZEOF_ZONE_POLICY_8
            + num_rules * self.SIZEOF_ZONE_RULE_8
            + memory_letters8)
        memory32 = (
            1 * self.SIZEOF_ZONE_POLICY_32
            + num_rules * self.SIZEOF_ZONE_RULE_32
            + memory_letters32)

        policy_item = self.ZONE_POLICIES_CPP_POLICY_ITEM.format(
            scope=self.scope,
            policyName=policy_name,
            numRules=num_rules,
            memory8=memory8,
            memory32=memory32,
            ruleItems=rule_items,
            numLetters=num_letters,
            letterArrayRef=letter_array_ref,
            letterArray=letter_array,
            progmem='ACE_TIME_PROGMEM')

        return (policy_item, memory8, memory32)


class ZoneInfosGenerator:
    ZONE_INFOS_H_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files
//
//   {tz_files}
//
// from https://github.com/eggert/tz/releases/tag/{tz_version}
//
// DO NOT EDIT

#ifndef ACE_TIME_{dbHeaderNamespace}_ZONE_INFOS_H
#define ACE_TIME_{dbHeaderNamespace}_ZONE_INFOS_H

#include <ace_time/internal/ZoneInfo.h>

namespace ace_time {{
namespace {dbNamespace} {{

//---------------------------------------------------------------------------
// ZoneContext (should not be in PROGMEM)
//---------------------------------------------------------------------------

// Version of the TZ Database which generated these files.
extern const char kTzDatabaseVersion[];

// Metadata about the zonedb files.
extern const internal::ZoneContext kZoneContext;

//---------------------------------------------------------------------------
// Supported zones: {numInfos}
//---------------------------------------------------------------------------

{infoItems}

{zoneIds}

//---------------------------------------------------------------------------
// Supported links: {numLinks}
//---------------------------------------------------------------------------

{linkItems}

{linkIds}

//---------------------------------------------------------------------------
// Estimated size of the Transition buffer in ExtendedZoneProcessor for each
// zone. Used only in the tests/validation/Extended*Test tests for
// ExtendedZoneProcessor. This used to be included in the ZoneInfo data struct
// above, but it is used only for tests, so pulling them out to these constants
// means that they take up no permanent storage space.
//---------------------------------------------------------------------------

{bufSizes}

//---------------------------------------------------------------------------
// Unsupported zones: {numRemovedInfos}
//---------------------------------------------------------------------------

{removedInfoItems}

//---------------------------------------------------------------------------
// Notable zones: {numNotableInfos}
//---------------------------------------------------------------------------

{notableInfoItems}

//---------------------------------------------------------------------------
// Unsupported links: {numRemovedLinks}
//---------------------------------------------------------------------------

{removedLinkItems}

//---------------------------------------------------------------------------
// Notable links: {numNotableLinks}
//---------------------------------------------------------------------------

{notableLinkItems}

}}
}}

#endif
"""

    ZONE_INFOS_CPP_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files
//
//   {tz_files}
//
// from https://github.com/eggert/tz/releases/tag/{tz_version}
//
// Zones: {numInfos}
// Links: {numLinks}
// kZoneRegistry sizes (bytes):
//   Names: {zoneStringSize} (originally {zoneStringOriginalSize})
//   Formats: {formatSize}
//   Fragments: {fragmentSize}
//   Memory (8-bit): {zoneMemory8}
//   Memory (32-bit): {zoneMemory32}
// kZoneAndLinkRegistry sizes (bytes):
//   Names: {zoneAndLinkStringSize} (originally {zoneAndLinkStringOriginalSize})
//   Formats: {formatSize}
//   Fragments: {fragmentSize}
//   Memory (8-bit): {zoneAndLinkMemory8}
//   Memory (32-bit): {zoneAndLinkMemory32}
//
// DO NOT EDIT

#include <ace_time/common/compat.h>
#include "zone_policies.h"
#include "zone_infos.h"

namespace ace_time {{
namespace {dbNamespace} {{

//---------------------------------------------------------------------------
// ZoneContext (should not be in PROGMEM)
//---------------------------------------------------------------------------

const char kTzDatabaseVersion[] = "{tz_version}";

const char* const kFragments[] = {{
{fragments}
}};

const internal::ZoneContext kZoneContext = {{
  {start_year} /*startYear*/,
  {until_year} /*untilYear*/,
  kTzDatabaseVersion /*tzVersion*/,
  {numFragments} /*numFragments*/,
  kFragments /*fragments*/,
}};

//---------------------------------------------------------------------------
// Zones: {numInfos}
//---------------------------------------------------------------------------

{infoItems}

//---------------------------------------------------------------------------
// Links: {numLinks}
//---------------------------------------------------------------------------

{linkItems}
}}
}}
"""

    ZONE_INFOS_CPP_INFO_ITEM = """\
//---------------------------------------------------------------------------
// Zone name: {zoneFullName}
// Zone Eras: {numEras}
// Strings (bytes): {stringSize} (originally {originalSize})
// Memory (8-bit): {memory8}
// Memory (32-bit): {memory32}
//---------------------------------------------------------------------------

static const {scope}::ZoneEra kZoneEra{zoneNormalizedName}[] {progmem} = {{
{eraItems}
}};

static const char kZoneName{zoneNormalizedName}[] {progmem} = \
{compressedName};

const {scope}::ZoneInfo kZone{zoneNormalizedName} {progmem} = {{
  kZoneName{zoneNormalizedName} /*name*/,
  0x{zoneId:08x} /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  {numEras} /*numEras*/,
  kZoneEra{zoneNormalizedName} /*eras*/,
}};

"""

    ZONE_INFOS_CPP_ERA_ITEM = """\
  // {raw_line}
  {{
    {zone_policy} /*zonePolicy*/,
    "{format}" /*format*/,
    {offset_code} /*offsetCode*/,
    {delta_code} /*deltaCode ({delta_code_comment})*/,
    {until_year_tiny} /*untilYearTiny*/,
    {until_month} /*untilMonth*/,
    {until_day} /*untilDay*/,
    {until_time_code} /*untilTimeCode*/,
    {until_time_modifier} /*untilTimeModifier ({until_time_modifier_comment})*/,
  }},
"""  # noqa

    SIZEOF_ZONE_ERA_8 = 11
    SIZEOF_ZONE_ERA_32 = 16  # 15 rounded to 4-byte alignment
    SIZEOF_ZONE_INFO_8 = 11
    SIZEOF_ZONE_INFO_32 = 20  # 18 rounded to 4-byte alignment

    def __init__(
        self,
        invocation: str,
        db_namespace: str,
        tz_version: str,
        tz_files: str,
        scope: str,
        start_year: int,
        until_year: int,
        zones_map: ZonesMap,
        links_map: LinksMap,
        policies_map: PoliciesMap,
        removed_zones: CommentsMap,
        removed_links: CommentsMap,
        removed_policies: CommentsMap,
        notable_zones: CommentsMap,
        notable_links: CommentsMap,
        notable_policies: CommentsMap,
        buf_sizes: BufSizeMap,
        zone_ids: Dict[str, int],
        link_ids: Dict[str, int],
        formats_map: IndexMap,
        fragments_map: IndexMap,
        compressed_names: Dict[str, str],
    ):
        self.invocation = invocation
        self.db_namespace = db_namespace
        self.tz_version = tz_version
        self.tz_files = tz_files
        self.scope = scope
        self.start_year = start_year
        self.until_year = until_year
        self.zones_map = zones_map
        self.links_map = links_map
        self.policies_map = policies_map
        self.removed_zones = removed_zones
        self.removed_links = removed_links
        self.removed_policies = removed_policies
        self.notable_zones = notable_zones
        self.notable_links = notable_links
        self.notable_policies = notable_policies
        self.buf_sizes = buf_sizes
        self.zone_ids = zone_ids
        self.link_ids = link_ids
        self.formats_map = formats_map
        self.fragments_map = fragments_map
        self.compressed_names = compressed_names

        self.db_header_namespace = self.db_namespace.upper()

    def generate_infos_h(self) -> str:
        ZONE_INFOS_H_INFO_ITEM = """\
extern const {scope}::ZoneInfo kZone{zoneNormalizedName}; // {zoneFullName}
"""
        ZONE_INFOS_H_INFO_ZONE_ID = """\
const uint32_t kZoneId{zoneNormalizedName} = 0x{zoneId:08x}; // {zoneFullName}
"""
        ZONE_INFOS_H_BUF_SIZE = """\
const uint8_t kZoneBufSize{zoneNormalizedName} = {bufSize};  // {zoneFullName}
"""
        info_items = ''
        info_zone_ids = ''
        info_buf_sizes = ''
        for zone_name, eras in sorted(self.zones_map.items()):
            normalized_name = normalize_name(zone_name)
            info_items += ZONE_INFOS_H_INFO_ITEM.format(
                scope=self.scope,
                zoneNormalizedName=normalized_name,
                zoneFullName=zone_name,
            )
            info_zone_ids += ZONE_INFOS_H_INFO_ZONE_ID.format(
                zoneNormalizedName=normalized_name,
                zoneFullName=zone_name,
                zoneId=self.zone_ids[zone_name],
            )
            info_buf_sizes += ZONE_INFOS_H_BUF_SIZE.format(
                zoneNormalizedName=normalized_name,
                zoneFullName=zone_name,
                bufSize=self.buf_sizes[zone_name],
            )

        ZONE_INFOS_H_LINK_ITEM = """\
extern const {scope}::ZoneInfo kZone{linkNormalizedName}; \
// {linkFullName} -> {zoneFullName}
"""
        ZONE_INFOS_H_LINK_ID = """\
const uint32_t kZoneId{linkNormalizedName} = 0x{linkId:08x}; // {linkFullName}
"""
        link_items = ''
        link_ids = ''
        for link_name, zone_name in sorted(self.links_map.items()):
            link_items += ZONE_INFOS_H_LINK_ITEM.format(
                scope=self.scope,
                linkNormalizedName=normalize_name(link_name),
                linkFullName=link_name,
                zoneFullName=zone_name)
            link_ids += ZONE_INFOS_H_LINK_ID.format(
                linkNormalizedName=normalize_name(link_name),
                linkFullName=link_name,
                linkId=self.link_ids[link_name],
            )

        ZONE_INFOS_H_REMOVED_INFO_ITEM = """\
// {zoneFullName} ({reason})
"""
        removed_info_items = ''
        for zone_name, reasons in sorted(self.removed_zones.items()):
            removed_info_items += ZONE_INFOS_H_REMOVED_INFO_ITEM.format(
                zoneFullName=zone_name, reason=', '.join(reasons))

        ZONE_INFOS_H_NOTABLE_INFO_ITEM = """\
// {zoneFullName} ({reason})
"""
        notable_info_items = ''
        for zone_name, reasons in sorted(self.notable_zones.items()):
            notable_info_items += ZONE_INFOS_H_NOTABLE_INFO_ITEM.format(
                zoneFullName=zone_name, reason=', '.join(reasons))

        ZONE_INFOS_H_REMOVED_LINK_ITEM = """\
// {linkFullName} ({reason})
"""
        removed_link_items = ''
        for link_name, reasons in sorted(self.removed_links.items()):
            removed_link_items += ZONE_INFOS_H_REMOVED_LINK_ITEM.format(
                linkFullName=link_name, reason=', '.join(reasons))

        ZONE_INFOS_H_NOTABLE_LINK_ITEM = """\
// {linkFullName} ({reason})
"""
        notable_link_items = ''
        for link_name, reasons in sorted(self.notable_links.items()):
            notable_link_items += ZONE_INFOS_H_NOTABLE_LINK_ITEM.format(
                linkFullName=link_name, reason=', '.join(reasons))

        return self.ZONE_INFOS_H_FILE.format(
            invocation=self.invocation,
            tz_files=self.tz_files,
            tz_version=self.tz_version,
            scope=self.scope,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numInfos=len(self.zones_map),
            infoItems=info_items,
            zoneIds=info_zone_ids,
            numLinks=len(self.links_map),
            linkItems=link_items,
            linkIds=link_ids,
            numRemovedInfos=len(self.removed_zones),
            removedInfoItems=removed_info_items,
            numNotableInfos=len(self.notable_zones),
            notableInfoItems=notable_info_items,
            numRemovedLinks=len(self.removed_links),
            removedLinkItems=removed_link_items,
            numNotableLinks=len(self.notable_links),
            notableLinkItems=notable_link_items,
            bufSizes=info_buf_sizes,
        )

    def generate_infos_cpp(self) -> str:
        # Generate the list of zone infos
        num_eras = 0
        info_items = ''
        for zone_name, eras in sorted(self.zones_map.items()):
            info_item = self._generate_info_item(zone_name, eras)
            info_items += info_item
            num_eras += len(eras)

        # Generate links references.
        link_items = ''
        for link_name, zone_name in sorted(self.links_map.items()):
            link_item = self._generate_link_item(link_name, zone_name)
            link_items += link_item

        # Generate fragments.
        num_fragments = len(self.fragments_map) + 1
        fragments = '/*\\x00*/ nullptr,\n'
        for fragment, index in self.fragments_map.items():
            fragments += f'/*\\x{index:02x}*/ "{fragment}",\n'

        # Estimate size of entire ZoneInfo database, factoring in deduping
        # of strings
        num_infos = len(self.zones_map)
        num_links = len(self.links_map)
        zone_string_original_size = sum([
            len(name) + 1 for name in self.zones_map.keys()
        ])
        zone_string_size = sum([
            len(self.compressed_names[name]) + 1
            for name in self.zones_map.keys()
        ])
        link_string_original_size = sum([
            len(name) + 1 for name in self.links_map.keys()
        ])
        link_string_size = sum([
            len(self.compressed_names[name]) + 1
            for name in self.links_map.keys()
        ])
        format_size = sum([len(s) + 1 for s in self.formats_map.keys()])
        fragment_size = sum([len(s) + 1 for s in self.fragments_map.keys()])

        zone_memory8 = (
            zone_string_size
            + format_size
            + fragment_size
            + num_eras * self.SIZEOF_ZONE_ERA_8
            + num_infos * self.SIZEOF_ZONE_INFO_8
            + num_infos * 2  # sizeof(kZoneRegistry)
            + num_fragments * 2
        )
        zone_memory32 = (
            zone_string_size
            + format_size
            + fragment_size
            + num_eras * self.SIZEOF_ZONE_ERA_32
            + num_infos * self.SIZEOF_ZONE_INFO_32
            + num_infos * 4  # sizeof(kZoneRegistry)
            + num_fragments * 2
        )
        zone_and_link_memory8 = (
            zone_memory8
            + link_string_size
            + num_links * self.SIZEOF_ZONE_INFO_8
            + num_links * 2  # sizeof(kZoneAndLinkRegistry)
        )
        zone_and_link_memory32 = (
            zone_memory32
            + link_string_size
            + num_links * self.SIZEOF_ZONE_INFO_32
            + num_links * 4  # sizeof(kZoneAndLinkRegistry)
        )
        zone_and_link_string_original_size = (
            zone_string_original_size + link_string_original_size
        )

        return self.ZONE_INFOS_CPP_FILE.format(
            invocation=self.invocation,
            tz_files=self.tz_files,
            tz_version=self.tz_version,
            scope=self.scope,
            start_year=self.start_year,
            until_year=self.until_year,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numInfos=num_infos,
            numLinks=num_links,
            numEras=num_eras,
            zoneStringSize=zone_string_size,
            zoneStringOriginalSize=zone_string_original_size,
            zoneMemory8=zone_memory8,
            zoneMemory32=zone_memory32,
            zoneAndLinkStringSize=(zone_string_size + link_string_size),
            zoneAndLinkStringOriginalSize=zone_and_link_string_original_size,
            zoneAndLinkMemory8=zone_and_link_memory8,
            zoneAndLinkMemory32=zone_and_link_memory32,
            formatSize=format_size,
            fragmentSize=fragment_size,
            infoItems=info_items,
            linkItems=link_items,
            numFragments=num_fragments,
            fragments=fragments,
        )

    def _generate_info_item(
        self,
        zone_name: str,
        eras: List[ZoneEraRaw],
    ) -> str:
        era_items = ''
        for era in eras:
            era_item = self._generate_era_item(zone_name, era)
            era_items += era_item

        compressed_name = self.compressed_names[zone_name]
        rendered_name = _compressed_name_to_c_string(compressed_name)

        # Calculate memory sizes
        zone_name_size = len(compressed_name) + 1
        format_size = 0
        for era in eras:
            format_size += len(era['format_short']) + 1
        num_eras = len(eras)
        data_size8 = (
            num_eras * self.SIZEOF_ZONE_ERA_8
            + self.SIZEOF_ZONE_INFO_8
        )
        data_size32 = (
            num_eras * self.SIZEOF_ZONE_ERA_32
            + self.SIZEOF_ZONE_INFO_32
        )

        string_size = zone_name_size + format_size
        original_size = len(zone_name) + 1 + format_size
        info_item = self.ZONE_INFOS_CPP_INFO_ITEM.format(
            scope=self.scope,
            zoneFullName=zone_name,
            zoneNormalizedName=normalize_name(zone_name),
            compressedName=rendered_name,
            zoneId=self.zone_ids[zone_name],
            numEras=num_eras,
            stringSize=string_size,
            originalSize=original_size,
            memory8=data_size8 + string_size,
            memory32=data_size32 + string_size,
            eraItems=era_items,
            progmem='ACE_TIME_PROGMEM')
        return info_item

    def _generate_era_item(
        self, zone_name: str, era: ZoneEraRaw
    ) -> str:
        rules_policy_name = era['rules']
        if rules_policy_name == '-' or rules_policy_name == ':':
            zone_policy = 'nullptr'
        else:
            zone_policy = f'&kPolicy{normalize_name(rules_policy_name)}'

        offset_code = era['offset_code']
        delta_code = era['delta_code_encoded']
        delta_code_comment = _get_era_delta_code_comment(
            offset_seconds=era['offset_seconds_truncated'],
            delta_seconds=era['rules_delta_seconds_truncated'],
            scope=self.scope,
        )
        until_year_tiny = era['until_year_tiny']
        until_month = era['until_month']
        until_day = era['until_day']
        until_time_code = era['until_time_code']
        until_time_modifier = era['until_time_modifier']
        until_time_modifier_comment = _get_time_modifier_comment(
            time_seconds=era['until_seconds_truncated'],
            suffix=era['until_time_suffix'],
        )
        format_short = era['format_short']

        era_item = self.ZONE_INFOS_CPP_ERA_ITEM.format(
            raw_line=normalize_raw(era['raw_line']),
            offset_code=offset_code,
            delta_code=delta_code,
            delta_code_comment=delta_code_comment,
            zone_policy=zone_policy,
            format=format_short,
            until_year_tiny=until_year_tiny,
            until_month=until_month,
            until_day=until_day,
            until_time_code=until_time_code,
            until_time_modifier=until_time_modifier,
            until_time_modifier_comment=until_time_modifier_comment,
        )

        return era_item

    def _generate_link_item(
        self, link_name: str, zone_name: str,
    ) -> str:
        """Return the Link item.
        """
        ZONE_INFOS_CPP_LINK_ITEM = """\
//---------------------------------------------------------------------------
// Link name: {linkFullName} -> {zoneFullName}
// Strings (bytes): {stringSize} (originally {originalSize})
// Memory (8-bit): {memory8}
// Memory (32-bit): {memory32}
//---------------------------------------------------------------------------

static const char kZoneName{linkNormalizedName}[] {progmem} = \
{compressedName};

const {scope}::ZoneInfo kZone{linkNormalizedName} {progmem} = {{
  kZoneName{linkNormalizedName} /*name*/,
  0x{linkId:08x} /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  {numEras} /*numEras*/,
  kZoneEra{zoneNormalizedName} /*eras*/,
}};

"""
        compressed_name = self.compressed_names[link_name]
        rendered_name = _compressed_name_to_c_string(compressed_name)

        link_name_size = len(compressed_name) + 1
        original_size = len(link_name) + 1
        memory8 = link_name_size + self.SIZEOF_ZONE_INFO_8
        memory32 = link_name_size + self.SIZEOF_ZONE_INFO_32
        link_item = ZONE_INFOS_CPP_LINK_ITEM.format(
            scope=self.scope,
            linkFullName=link_name,
            linkNormalizedName=normalize_name(link_name),
            compressedName=rendered_name,
            linkId=self.link_ids[link_name],
            zoneFullName=zone_name,
            zoneNormalizedName=normalize_name(zone_name),
            stringSize=link_name_size,
            originalSize=original_size,
            memory8=memory8,
            memory32=memory32,
            numEras=len(self.zones_map[zone_name]),
            progmem='ACE_TIME_PROGMEM',
        )

        return link_item


class ZoneRegistryGenerator:

    ZONE_REGISTRY_CPP_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files
//
//   {tz_files}
//
// from https://github.com/eggert/tz/releases/tag/{tz_version}
//
// DO NOT EDIT

#include <ace_time/common/compat.h>
#include "zone_infos.h"
#include "zone_registry.h"

namespace ace_time {{
namespace {dbNamespace} {{

//---------------------------------------------------------------------------
// Zone registry. Sorted by zoneId.
//---------------------------------------------------------------------------
const {scope}::ZoneInfo* const kZoneRegistry[{numZones}] {progmem} = {{
{zoneRegistryItems}
}};

//---------------------------------------------------------------------------
// Zone and Link registry. Sorted by zoneId.
//---------------------------------------------------------------------------
const {scope}::ZoneInfo* const kZoneAndLinkRegistry[{numZonesAndLinks}] \
{progmem} = {{
{zoneAndLinkRegistryItems}
}};

}}
}}
"""

    ZONE_REGISTRY_H_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files
//
//   {tz_files}
//
// from https://github.com/eggert/tz/releases/tag/{tz_version}
//
// DO NOT EDIT

#ifndef ACE_TIME_{dbHeaderNamespace}_ZONE_REGISTRY_H
#define ACE_TIME_{dbHeaderNamespace}_ZONE_REGISTRY_H

#include <ace_time/internal/ZoneInfo.h>

namespace ace_time {{
namespace {dbNamespace} {{

// Zones
const uint16_t kZoneRegistrySize = {numZones};
extern const {scope}::ZoneInfo* const kZoneRegistry[{numZones}];

// Zones and Links
const uint16_t kZoneAndLinkRegistrySize = {numZonesAndLinks};
extern const {scope}::ZoneInfo* const kZoneAndLinkRegistry[{numZonesAndLinks}];

}}
}}
#endif
"""

    def __init__(
        self,
        invocation: str,
        tz_version: str,
        tz_files: str,
        scope: str,
        db_namespace: str,
        zones_map: ZonesMap,
        links_map: LinksMap,
        zone_ids: Dict[str, int],
        link_ids: Dict[str, int],
    ):
        self.invocation = invocation
        self.tz_version = tz_version
        self.tz_files = tz_files
        self.scope = scope
        self.db_namespace = db_namespace
        self.zones_map = zones_map
        self.links_map = links_map
        self.zone_ids = zone_ids
        self.link_ids = link_ids

        self.db_header_namespace = self.db_namespace.upper()
        self.zones_and_links = list(zones_map.keys()) + list(links_map.keys())
        self.zone_and_link_ids = zone_ids.copy()
        self.zone_and_link_ids.update(link_ids)

    def generate_registry_cpp(self) -> str:
        # Generate only Zones, sorted by zoneId to enable
        # ZoneRegistrar::binarySearchById().
        zone_registry_items = ''
        for zone_name in sorted(
                self.zones_map.keys(),
                key=lambda x: self.zone_ids[x],
        ):
            normalized_name = normalize_name(zone_name)
            zone_id = self.zone_ids[zone_name]
            zone_registry_items += f"""\
  &kZone{normalized_name}, // 0x{zone_id:08x}, {zone_name}
"""

        # Generate Zones and Links, sorted by zoneId.
        zone_and_link_registry_items = ''
        num_zones_and_links = len(self.zones_and_links)
        for zone_name in sorted(
            self.zones_and_links,
            key=lambda x: self.zone_and_link_ids[x],
        ):
            normalized_name = normalize_name(zone_name)
            zone_id = self.zone_and_link_ids[zone_name]
            target_name = self.links_map.get(zone_name)
            if target_name:
                desc_name = f'{zone_name} -> {target_name}'
            else:
                desc_name = zone_name

            zone_and_link_registry_items += f"""\
  &kZone{normalized_name}, // 0x{zone_id:08x}, {desc_name}
"""

        return self.ZONE_REGISTRY_CPP_FILE.format(
            invocation=self.invocation,
            tz_files=self.tz_files,
            tz_version=self.tz_version,
            scope=self.scope,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numZones=len(self.zones_map),
            numZonesAndLinks=num_zones_and_links,
            zoneRegistryItems=zone_registry_items,
            zoneAndLinkRegistryItems=zone_and_link_registry_items,
            progmem='ACE_TIME_PROGMEM',
        )

    def generate_registry_h(self) -> str:
        return self.ZONE_REGISTRY_H_FILE.format(
            invocation=self.invocation,
            tz_files=self.tz_files,
            tz_version=self.tz_version,
            scope=self.scope,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numZones=len(self.zones_map),
            numZonesAndLinks=len(self.zones_and_links),
        )


def _get_time_modifier_comment(
    time_seconds: int,
    suffix: str,
) -> str:
    """Create the comment that explains how the until_time_code or at_time_code
    was calculated.
    """
    if suffix == 'w':
        comment = 'kSuffixW'
    elif suffix == 's':
        comment = 'kSuffixS'
    else:
        comment = 'kSuffixU'
    remaining_time_minutes = time_seconds % 900 // 60
    comment += f' + minute={remaining_time_minutes}'
    return comment


def _get_era_delta_code_comment(
    offset_seconds: int,
    delta_seconds: int,
    scope: str,
) -> str:
    """Create the comment that explains how the ZoneEra delta_code[_encoded] was
    calculated.
    """
    offset_minute = offset_seconds % 900 // 60
    delta_minutes = delta_seconds // 60
    if scope == 'extended':
        return (
            f"((offsetMinute={offset_minute}) << 4) + "
            f"((deltaMinutes={delta_minutes})/15 + 4)"
        )
    else:
        return f"(deltaMinutes={delta_minutes})/15"


def _get_rule_delta_code_comment(
    delta_seconds: int,
    scope: str,
) -> str:
    """Create the comment that explains how the ZoneRule delta_code[_encoded]
    was calculated.
    """
    delta_minutes = delta_seconds // 60
    if scope == 'extended':
        return f"(deltaMinutes={delta_minutes})/15 + 4"
    else:
        return f"(deltaMinutes={delta_minutes})/15"


def _compressed_name_to_c_string(compressed_name: str) -> str:
    """Convert a compressed name (with fragment references) to a string that
    the C++ compiler will accept. The primary reason for this function is
    because the hex escape sequence (\\xHH) in C/C++ has no length limit, so
    will happily run into the characters after the HH. So we have to break
    those references into separate strings. Example: converts ("\x01ab")
    into ("\x01" "ab").
    """
    rendered_string = ''
    in_normal_string = False
    for c in compressed_name:
        if ord(c) < 0x20:
            if in_normal_string:
                rendered_string += f'" "\\x{ord(c):02x}" '
                in_normal_string = False
            else:
                rendered_string += f'"\\x{ord(c):02x}" '
        else:
            if in_normal_string:
                rendered_string += c
            else:
                rendered_string += f'"{c}'
            in_normal_string = True
    if in_normal_string:
        rendered_string += '"'
    return rendered_string.strip()
