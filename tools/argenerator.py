# Copyright 2018 Brian T. Park
#
# MIT License
"""
Generate the zone_info and zone_policies files for Arduino.
"""

import logging
import os
import transformer
from collections import OrderedDict
from transformer import div_to_zero
from transformer import normalize_name
from transformer import hash_name
from extractor import EPOCH_YEAR
from extractor import MAX_YEAR
from extractor import MAX_YEAR_TINY
from extractor import MIN_YEAR
from extractor import MIN_YEAR_TINY
from extractor import MAX_UNTIL_YEAR
from extractor import MAX_UNTIL_YEAR_TINY


class ArduinoGenerator:
    """Generate zone_infos and zone_policies files for Arduino/C++.
    """
    ZONE_INFOS_H_FILE_NAME = 'zone_infos.h'
    ZONE_INFOS_CPP_FILE_NAME = 'zone_infos.cpp'
    ZONE_POLICIES_H_FILE_NAME = 'zone_policies.h'
    ZONE_POLICIES_CPP_FILE_NAME = 'zone_policies.cpp'
    ZONE_REGISTRY_H_FILE_NAME = 'zone_registry.h'
    ZONE_REGISTRY_CPP_FILE_NAME = 'zone_registry.cpp'
    ZONE_STRINGS_CPP_FILE_NAME = 'zone_strings.cpp'
    ZONE_STRINGS_H_FILE_NAME = 'zone_strings.h'

    def __init__(self, invocation, tz_version, tz_files, scope, db_namespace,
                 generate_zone_strings, start_year, until_year,
                 zones_map, links_map, rules_map, removed_zones, removed_links,
                 removed_policies, notable_zones, notable_links,
                 notable_policies, format_strings, zone_strings, buf_sizes):
        self.scope = scope
        self.db_namespace = db_namespace
        self.generate_zone_strings = generate_zone_strings

        self.zone_policies_generator = ZonePoliciesGenerator(
            invocation=invocation,
            tz_version=tz_version,
            tz_files=tz_files,
            scope=scope,
            db_namespace=db_namespace,
            zones_map=zones_map,
            rules_map=rules_map,
            removed_zones=removed_zones,
            removed_policies=removed_policies,
            notable_zones=notable_zones,
            notable_policies=notable_policies)
        self.zone_infos_generator = ZoneInfosGenerator(
            invocation=invocation,
            tz_version=tz_version,
            tz_files=tz_files,
            scope=scope,
            db_namespace=db_namespace,
            start_year=start_year,
            until_year=until_year,
            zones_map=zones_map,
            links_map=links_map,
            rules_map=rules_map,
            removed_zones=removed_zones,
            removed_links=removed_links,
            removed_policies=removed_policies,
            notable_zones=notable_zones,
            notable_links=notable_links,
            notable_policies=notable_policies,
            buf_sizes=buf_sizes)
        self.zone_registry_generator = ZoneRegistryGenerator(
            invocation=invocation,
            tz_version=tz_version,
            tz_files=tz_files,
            scope=scope,
            db_namespace=db_namespace,
            zones_map=zones_map)

        if generate_zone_strings:
            self.zone_strings_generator = ZoneStringsGenerator(
                invocation=invocation,
                tz_version=tz_version,
                tz_files=tz_files,
                scope=scope,
                db_namespace=db_namespace,
                zones_map=zones_map,
                rules_map=rules_map,
                removed_zones=removed_zones,
                removed_policies=removed_policies,
                notable_zones=notable_zones,
                notable_policies=notable_policies,
                format_strings=format_strings,
                zone_strings=zone_strings)

    def generate_files(self, output_dir):
        # zone_policies.*
        if self.scope == 'extended':
            self.zone_policies_generator.collect_letter_strings()
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

        # zone_strings.*
        if self.generate_zone_strings:
            self._write_file(output_dir, self.ZONE_STRINGS_H_FILE_NAME,
                             self.zone_strings_generator.generate_strings_h())
            self._write_file(output_dir, self.ZONE_STRINGS_CPP_FILE_NAME,
                             self.zone_strings_generator.generate_strings_cpp())

    def _write_file(self, output_dir, filename, content):
        full_filename = os.path.join(output_dir, filename)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            print(content, end='', file=output_file)
        logging.info("Created %s", full_filename)


class ZonePoliciesGenerator:

    ZONE_POLICIES_H_FILE = """\
// This file was generated by the following script:
//
//  $ {invocation}
//
// using the TZ Database files
//
//  {tz_files}
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

// Inaccurate zone policies: {numNotablePolicies}
//
{notablePolicyItems}

}}
}}

#endif
"""

    ZONE_POLICIES_H_POLICY_ITEM = """\
extern const {scope}::ZonePolicy kPolicy{policyName};
"""

    ZONE_POLICIES_H_REMOVED_POLICY_ITEM = """\
// kPolicy{policyName} ({policyReason})
"""

    ZONE_POLICIES_H_NOTABLE_POLICY_ITEM = """\
// kPolicy{policyName} ({policyReason})
"""

    ZONE_POLICIES_CPP_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files from
//  https://github.com/eggert/tz/releases/tag/{tz_version}
//
// Policies: {numPolicies}
// Rules: {numRules}
// Memory (8-bit): {memory8}
// Memory (32-bit): {memory32}
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
  {letterArrayRef} /* letters */,
  {numRules} /*numRules*/,
  {numLetters} /* numLetters */,
}};

"""

    ZONE_POLICIES_LETTER_ARRAY = """\
static const char* const kLetters{policyName}[] {progmem} = {{
{letterItems}
}};
"""

    ZONE_POLICIES_CPP_RULE_ITEM = """\
  // {rawLine}
  {{
    {fromYearTiny} /*fromYearTiny*/,
    {toYearTiny} /*toYearTiny*/,
    {inMonth} /*inMonth*/,
    {onDayOfWeek} /*onDayOfWeek*/,
    {onDayOfMonth} /*onDayOfMonth*/,
    {atTimeCode} /*atTimeCode*/,
    {atTimeModifier} /*atTimeModifier*/,
    {deltaCode} /*deltaCode*/,
    {letter} /*letter{letterComment}*/,
  }},
"""

    SIZEOF_ZONE_RULE_8 = 9
    SIZEOF_ZONE_RULE_32 = 12
    SIZEOF_ZONE_POLICY_8 = 6
    SIZEOF_ZONE_POLICY_32 = 12

    def __init__(self, invocation, tz_version, tz_files, scope, db_namespace,
                 zones_map, rules_map,
                 removed_zones, removed_policies, notable_zones,
                 notable_policies):
        self.invocation = invocation
        self.tz_version = tz_version
        self.tz_files = tz_files
        self.scope = scope
        self.db_namespace = db_namespace
        self.zones_map = zones_map
        self.rules_map = rules_map
        self.removed_zones = removed_zones
        self.removed_policies = removed_policies
        self.notable_zones = notable_zones
        self.notable_policies = notable_policies

        self.letters_map = {}  # map{policy_name: map{letter: index}}
        self.db_header_namespace = self.db_namespace.upper()

    def collect_letter_strings(self):
        """Loop through all ZoneRules and collect the LETTERs which are
        more than one letter long into self.letters_map.
        """
        letters_map = {}
        for policy, rules in self.rules_map.items():
            letters = set()
            for rule in rules:
                if len(rule.letter) > 1:
                    letters.add(rule.letter)
            indexed_letters_map = OrderedDict()
            if letters:
                for letter in sorted(letters):
                    transformer.add_string(indexed_letters_map, letter)
                letters_map[policy] = indexed_letters_map
        self.letters_map = letters_map

    def generate_policies_h(self):
        policy_items = ''
        for name, rules in sorted(self.rules_map.items()):
            policy_items += self.ZONE_POLICIES_H_POLICY_ITEM.format(
                policyName=normalize_name(name),
                scope=self.scope)

        removed_policy_items = ''
        for name, reasons in sorted(self.removed_policies.items()):
            removed_policy_items += \
                self.ZONE_POLICIES_H_REMOVED_POLICY_ITEM.format(
                    policyName=name,
                    policyReason=', '.join(reasons))

        notable_policy_items = ''
        for name, reasons in sorted(self.notable_policies.items()):
            notable_policy_items += \
                self.ZONE_POLICIES_H_NOTABLE_POLICY_ITEM.format(
                    policyName=name,
                    policyReason=', '.join(reasons))

        return self.ZONE_POLICIES_H_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            tz_files=', '.join(self.tz_files),
            numPolicies=len(self.rules_map),
            policyItems=policy_items,
            numRemovedPolicies=len(self.removed_policies),
            removedPolicyItems=removed_policy_items,
            numNotablePolicies=len(self.notable_policies),
            notablePolicyItems=notable_policy_items)

    def generate_policies_cpp(self):
        policy_items = ''
        memory8 = 0
        memory32 = 32
        num_rules = 0
        for name, rules in sorted(self.rules_map.items()):
            indexed_letters = self.letters_map.get(name)
            num_rules += len(rules)
            policy_item, policy_memory8, policy_memory32 = \
                self._generate_policy_item(name, rules, indexed_letters)
            policy_items += policy_item
            memory8 += policy_memory8
            memory32 += policy_memory32

        num_policies = len(self.rules_map)

        return self.ZONE_POLICIES_CPP_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numPolicies=num_policies,
            numRules=num_rules,
            memory8=memory8,
            memory32=memory32,
            policyItems=policy_items)

    def _generate_policy_item(self, name, rules, indexed_letters):
        # Generate kZoneRules*[]
        rule_items = ''
        for rule in rules:
            at_time_code = div_to_zero(rule.atSecondsTruncated, 15 * 60)
            delta_code = div_to_zero(rule.deltaSecondsTruncated, 15 * 60)

            from_year = rule.fromYear
            from_year_tiny = to_tiny_year(from_year)
            to_year = rule.toYear
            to_year_tiny = to_tiny_year(to_year)

            if len(rule.letter) == 1:
                letter = "'%s'" % rule.letter
                letterComment = ''
            elif len(rule.letter) > 1:
                index = indexed_letters.get(rule.letter)
                if index == None:
                    raise Exception('Could not find index for letter (%s)'
                                    % rule.letter)
                if index >= 32:
                    raise Exception('Number of indexed letters >= 32')
                letter = index
                letterComment = ('; "%s"' % rule.letter)
            else:
                raise Exception('len(%s) == 0; should not happen' % rule.letter)

            rule_items += self.ZONE_POLICIES_CPP_RULE_ITEM.format(
                rawLine=normalize_raw(rule.rawLine),
                fromYearTiny=from_year_tiny,
                toYearTiny=to_year_tiny,
                inMonth=rule.inMonth,
                onDayOfWeek=rule.onDayOfWeek,
                onDayOfMonth=rule.onDayOfMonth,
                atTimeCode=at_time_code,
                atTimeModifier=to_modifier(rule.atTimeModifier, self.scope),
                deltaCode=delta_code,
                letter=letter,
                letterComment=letterComment)

        # Generate kLetters*[]
        policyName = normalize_name(name)
        numLetters = len(indexed_letters) if indexed_letters else 0
        memoryLetters8 = 0
        memoryLetters32 = 0
        if numLetters:
            letterArrayRef = 'kLetters%s' % policyName
            letterItems = ''
            for name, index in indexed_letters.items():
                letterItems += ('  /*%d*/ "%s",\n' % (index, name))
                memoryLetters8 += len(name) + 1 + 2  # NUL terminated
                memoryLetters32 += len(name) + 1 + 4  # NUL terminated
            letterArray = self.ZONE_POLICIES_LETTER_ARRAY.format(
                policyName=policyName,
                letterItems=letterItems,
                progmem='ACE_TIME_PROGMEM')
        else:
            letterArrayRef = 'nullptr'
            letterArray = ''

        # Calculate the memory consumed by structs and arrays
        num_rules = len(rules)
        memory8 = (1 * self.SIZEOF_ZONE_POLICY_8 +
                   num_rules * self.SIZEOF_ZONE_RULE_8 +
                   memoryLetters8)
        memory32 = (1 * self.SIZEOF_ZONE_POLICY_32 +
                    num_rules * self.SIZEOF_ZONE_RULE_32 +
                    memoryLetters32)

        policy_item = self.ZONE_POLICIES_CPP_POLICY_ITEM.format(
            scope=self.scope,
            policyName=policyName,
            numRules=num_rules,
            memory8=memory8,
            memory32=memory32,
            ruleItems=rule_items,
            numLetters=numLetters,
            letterArrayRef=letterArrayRef,
            letterArray=letterArray,
            progmem='ACE_TIME_PROGMEM')

        return (policy_item, memory8, memory32)


class ZoneInfosGenerator:
    ZONE_INFOS_H_FILE = """\
// This file was generated by the following script:
//
//  $ {invocation}
//
// using the TZ Database files
//
//  {tz_files}
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
extern const {scope}::ZoneContext kZoneContext;

//---------------------------------------------------------------------------
// Supported zones: {numInfos}
//---------------------------------------------------------------------------

{infoItems}

//---------------------------------------------------------------------------
// Supported links: {numLinks}
//---------------------------------------------------------------------------

{linkItems}

//---------------------------------------------------------------------------
// Unsupported zones: {numRemovedInfos}
//---------------------------------------------------------------------------

{removedInfoItems}

//---------------------------------------------------------------------------
// Inaccurate zones: {numNotableInfos}
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

    ZONE_INFOS_H_INFO_ITEM = """\
extern const {scope}::ZoneInfo kZone{zoneNormalizedName}; // {zoneFullName}
"""

    ZONE_INFOS_H_REMOVED_INFO_ITEM = """\
// {zoneFullName} ({reason})
"""

    ZONE_INFOS_H_NOTABLE_INFO_ITEM = """\
// {zoneFullName} ({reason})
"""

    ZONE_INFOS_H_LINK_ITEM = """\
extern const {scope}::ZoneInfo& kZone{linkNormalizedName}; // {linkFullName} -> {zoneFullName}
"""

    ZONE_INFOS_H_REMOVED_LINK_ITEM = """\
// {linkFullName} ({reason})
"""

    ZONE_INFOS_H_NOTABLE_LINK_ITEM = """\
// {linkFullName} ({reason})
"""

    ZONE_INFOS_CPP_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files from
// https://github.com/eggert/tz/releases/tag/{tz_version}
//
// Zones: {numInfos}
// Links: {numLinks}
// Strings (bytes): {stringLength}
// Memory (8-bit): {memory8}
// Memory (32-bit): {memory32}
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

const {scope}::ZoneContext kZoneContext = {{
  {startYear} /*startYear*/,
  {untilYear} /*untilYear*/,
  kTzDatabaseVersion /*tzVersion*/,
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
// Strings (bytes): {stringLength}
// Memory (8-bit): {memory8}
// Memory (32-bit): {memory32}
//---------------------------------------------------------------------------

static const {scope}::ZoneEra kZoneEra{zoneNormalizedName}[] {progmem} = {{
{eraItems}
}};

static const char kZoneName{zoneNormalizedName}[] {progmem} = "{zoneFullName}";

const {scope}::ZoneInfo kZone{zoneNormalizedName} {progmem} = {{
  kZoneName{zoneNormalizedName} /*name*/,
  0x{zoneNameHash:08x} /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  {transitionBufSize} /*transitionBufSize*/,
  {numEras} /*numEras*/,
  kZoneEra{zoneNormalizedName} /*eras*/,
}};

"""

    ZONE_INFOS_CPP_ERA_ITEM = """\
  // {rawLine}
  {{
    {zonePolicy} /*zonePolicy*/,
    "{format}" /*format*/,
    {offsetCode} /*offsetCode*/,
    {deltaCode} /*deltaCode*/,
    {untilYearTiny} /*untilYearTiny*/,
    {untilMonth} /*untilMonth*/,
    {untilDay} /*untilDay*/,
    {untilTimeCode} /*untilTimeCode*/,
    {untilTimeModifier} /*untilTimeModifier*/,
  }},
"""

    ZONE_INFOS_CPP_LINK_ITEM = """\
const {scope}::ZoneInfo& kZone{linkNormalizedName} = kZone{zoneNormalizedName};
"""

    SIZEOF_ZONE_ERA_8 = 11
    SIZEOF_ZONE_ERA_32 = 16
    SIZEOF_ZONE_INFO_8 = 12
    SIZEOF_ZONE_INFO_32 = 20

    def __init__(self, invocation, tz_version, tz_files, scope, db_namespace,
                 start_year, until_year, zones_map, links_map,
                 rules_map, removed_zones, removed_links, removed_policies,
                 notable_zones, notable_links, notable_policies, buf_sizes):
        self.invocation = invocation
        self.tz_version = tz_version
        self.tz_files = tz_files
        self.scope = scope
        self.db_namespace = db_namespace
        self.start_year = start_year
        self.until_year = until_year
        self.zones_map = zones_map
        self.links_map = links_map
        self.rules_map = rules_map
        self.removed_zones = removed_zones
        self.removed_links = removed_links
        self.removed_policies = removed_policies
        self.notable_zones = notable_zones
        self.notable_links = notable_links
        self.notable_policies = notable_policies
        self.buf_sizes = buf_sizes

        self.db_header_namespace = self.db_namespace.upper()

    def generate_infos_h(self):
        info_items = ''
        for zone_name, eras in sorted(self.zones_map.items()):
            info_items += self.ZONE_INFOS_H_INFO_ITEM.format(
                scope=self.scope,
                zoneNormalizedName=normalize_name(zone_name),
                zoneFullName=zone_name)

        removed_info_items = ''
        for zone_name, reasons in sorted(self.removed_zones.items()):
            removed_info_items += self.ZONE_INFOS_H_REMOVED_INFO_ITEM.format(
                zoneFullName=zone_name, reason=', '.join(reasons))

        notable_info_items = ''
        for zone_name, reasons in sorted(self.notable_zones.items()):
            notable_info_items += self.ZONE_INFOS_H_NOTABLE_INFO_ITEM.format(
                zoneFullName=zone_name, reason=', '.join(reasons))

        link_items = ''
        for link_name, zone_name in sorted(self.links_map.items()):
            link_items += self.ZONE_INFOS_H_LINK_ITEM.format(
                scope=self.scope,
                linkNormalizedName=normalize_name(link_name),
                linkFullName=link_name,
                zoneFullName=zone_name)

        removed_link_items = ''
        for link_name, reasons in sorted(self.removed_links.items()):
            removed_link_items += self.ZONE_INFOS_H_REMOVED_LINK_ITEM.format(
                linkFullName=link_name, reason=', '.join(reasons))

        notable_link_items = ''
        for link_name, reasons in sorted(self.notable_links.items()):
            notable_link_items += self.ZONE_INFOS_H_NOTABLE_LINK_ITEM.format(
                linkFullName=link_name, reason=', '.join(reasons))

        return self.ZONE_INFOS_H_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            scope=self.scope,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            tz_files=', '.join(self.tz_files),
            numInfos=len(self.zones_map),
            infoItems=info_items,
            numLinks=len(self.links_map),
            linkItems=link_items,
            numRemovedInfos=len(self.removed_zones),
            removedInfoItems=removed_info_items,
            numNotableInfos=len(self.notable_zones),
            notableInfoItems=notable_info_items,
            numRemovedLinks=len(self.removed_links),
            removedLinkItems=removed_link_items,
            numNotableLinks=len(self.notable_links),
            notableLinkItems=notable_link_items)

    def generate_infos_cpp(self):
        string_length = 0

        # Generate the list of zone infos
        info_items = ''
        num_eras = 0
        for zone_name, eras in sorted(self.zones_map.items()):
            (info_item, info_string_length) = self._generate_info_item(
                zone_name, eras)
            info_items += info_item
            string_length += info_string_length
            num_eras += len(eras)

        # Generate links references.
        link_items = ''
        for link_name, zone_name in sorted(self.links_map.items()):
            eras = self.zones_map[zone_name]
            link_items += self._generate_link_item(link_name, zone_name)

        # Estimate size of entire zone info database.
        num_infos = len(self.zones_map)
        num_links = len(self.links_map)
        memory8 = (string_length + num_eras * self.SIZEOF_ZONE_ERA_8 +
                   num_infos * self.SIZEOF_ZONE_INFO_8)
        memory32 = (string_length + num_eras * self.SIZEOF_ZONE_ERA_32 +
                    num_infos * self.SIZEOF_ZONE_INFO_32)

        return self.ZONE_INFOS_CPP_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            scope=self.scope,
            startYear=self.start_year,
            untilYear=self.until_year,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numInfos=num_infos,
            numLinks=num_links,
            numEras=num_eras,
            stringLength=string_length,
            memory8=memory8,
            memory32=memory32,
            infoItems=info_items,
            linkItems=link_items)

    def _generate_info_item(self, zone_name, eras):
        era_items = ''
        string_length = 0
        for era in eras:
            (era_item, length) = self._generate_era_item(zone_name, era)
            era_items += era_item
            string_length += length

        string_length += len(zone_name) + 1
        num_eras = len(eras)
        memory8 = (string_length + num_eras * self.SIZEOF_ZONE_ERA_8 +
                   1 * self.SIZEOF_ZONE_INFO_8)
        memory32 = (string_length + num_eras * self.SIZEOF_ZONE_ERA_32 +
                    1 * self.SIZEOF_ZONE_INFO_32)

        transition_buf_size = self.buf_sizes[zone_name]

        info_item = self.ZONE_INFOS_CPP_INFO_ITEM.format(
            scope=self.scope,
            zoneFullName=zone_name,
            zoneNormalizedName=normalize_name(zone_name),
            zoneNameHash=hash_name(zone_name),
            transitionBufSize=transition_buf_size,
            numEras=num_eras,
            stringLength=string_length,
            memory8=memory8,
            memory32=memory32,
            eraItems=era_items,
            progmem='ACE_TIME_PROGMEM')
        return (info_item, string_length)

    def _generate_era_item(self, zone_name, era):
        policy_name = era.rules
        if policy_name == '-' or policy_name == ':':
            zone_policy = 'nullptr'
            delta_code = div_to_zero(era.rulesDeltaSecondsTruncated, 15 * 60)
        else:
            zone_policy = '&kPolicy%s' % normalize_name(policy_name)
            delta_code = 0

        until_year = era.untilYear
        if until_year == MAX_UNTIL_YEAR:
            until_year_tiny = MAX_UNTIL_YEAR_TINY
        else:
            until_year_tiny = until_year - EPOCH_YEAR

        until_month = era.untilMonth
        if not until_month:
            until_month = 1

        until_day = era.untilDay
        if not until_day:
            until_day = 1

        until_time_code = div_to_zero(era.untilSecondsTruncated, 15 * 60)
        until_time_modifier = to_modifier(era.untilTimeModifier, self.scope)
        offset_code = div_to_zero(era.offsetSecondsTruncated, 15 * 60)

        # Replace %s with just a % for C++
        format = era.format.replace('%s', '%')
        string_length = len(format) + 1

        era_item = self.ZONE_INFOS_CPP_ERA_ITEM.format(
            rawLine=normalize_raw(era.rawLine),
            offsetCode=offset_code,
            deltaCode=delta_code,
            zonePolicy=zone_policy,
            format=format,
            untilYearTiny=until_year_tiny,
            untilMonth=until_month,
            untilDay=until_day,
            untilTimeCode=until_time_code,
            untilTimeModifier=until_time_modifier)

        return (era_item, string_length)

    def _generate_link_item(self, link_name, zone_name):
        return self.ZONE_INFOS_CPP_LINK_ITEM.format(
            scope=self.scope,
            linkFullName=link_name,
            linkNormalizedName=normalize_name(link_name),
            zoneFullName=zone_name,
            zoneNormalizedName=normalize_name(zone_name))

class ZoneStringsGenerator:

    ZONE_STRINGS_CPP_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files from
// https://github.com/eggert/tz/releases/tag/{tz_version}
//
// DO NOT EDIT

#include <ace_time/common/compat.h>
#include "zone_strings.h"

namespace ace_time {{
namespace {dbNamespace} {{

// numStrings: {numFormatStrings}
// memory: {formatStringsSize}
// memory original: {formatStringsOrigSize}
const char* const kFormats[] = {{
{formatStringItems}
}};

// numStrings: {numZoneStrings}
// memory: {zoneStringsSize}
// memory original: {zoneStringsOrigSize}
const char* const kZoneStrings[] = {{
{zoneStringItems}
}};

}}
}}
"""

    ZONE_STRINGS_ITEM = """\
  /*{index:3}*/ "{stringName}",
"""

    ZONE_STRINGS_H_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files from
// https://github.com/eggert/tz/releases/tag/{tz_version}
//
// DO NOT EDIT

#ifndef ACE_TIME_{dbHeaderNamespace}_ZONE_STRINGS_H
#define ACE_TIME_{dbHeaderNamespace}_ZONE_STRINGS_H

namespace ace_time {{
namespace {dbNamespace} {{

extern const char* const kFormats[];

extern const char* const kZoneStrings[];

}}
}}
#endif
"""

    def __init__(self, invocation, tz_version, tz_files, scope, db_namespace,
                 zones_map, rules_map,
                 removed_zones, removed_policies, notable_zones,
                 notable_policies, format_strings, zone_strings):
        self.invocation = invocation
        self.tz_version = tz_version
        self.tz_files = tz_files
        self.scope = scope
        self.db_namespace = db_namespace
        self.zones_map = zones_map
        self.rules_map = rules_map
        self.removed_zones = removed_zones
        self.removed_policies = removed_policies
        self.notable_zones = notable_zones
        self.notable_policies = notable_policies
        self.format_strings = format_strings
        self.zone_strings = zone_strings

        self.db_header_namespace = self.db_namespace.upper()

    def generate_strings_cpp(self):
        format_string_items = ''
        for name, index in self.format_strings.ordered_map.items():
            format_string_items += self.ZONE_STRINGS_ITEM.format(
                stringName=name,
                index=index)

        zone_string_items = ''
        for name, index in self.zone_strings.ordered_map.items():
            zone_string_items += self.ZONE_STRINGS_ITEM.format(
                stringName=name,
                index=index)

        return self.ZONE_STRINGS_CPP_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numFormatStrings=len(self.format_strings.ordered_map),
            formatStringsSize=self.format_strings.size,
            formatStringsOrigSize=self.format_strings.orig_size,
            formatStringItems=format_string_items,
            numZoneStrings=len(self.zone_strings.ordered_map),
            zoneStringsSize=self.zone_strings.size,
            zoneStringsOrigSize=self.zone_strings.orig_size,
            zoneStringItems=zone_string_items)

    def generate_strings_h(self):
        return self.ZONE_STRINGS_H_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace)


class ZoneRegistryGenerator:

    ZONE_REGISTRY_CPP_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files from
// https://github.com/eggert/tz/releases/tag/{tz_version}
//
// DO NOT EDIT

#include <ace_time/common/compat.h>
#include "zone_infos.h"
#include "zone_registry.h"

namespace ace_time {{
namespace {dbNamespace} {{

//---------------------------------------------------------------------------
// Zone registry. Sorted by zone name.
//---------------------------------------------------------------------------
const {scope}::ZoneInfo* const kZoneRegistry[{numZones}] {progmem} = {{
{zoneRegistryItems}
}};

}}
}}
"""

    ZONE_REGISTRY_H_FILE = """\
// This file was generated by the following script:
//
//   $ {invocation}
//
// using the TZ Database files from
// https://github.com/eggert/tz/releases/tag/{tz_version}
//
// DO NOT EDIT

#ifndef ACE_TIME_{dbHeaderNamespace}_ZONE_REGISTRY_H
#define ACE_TIME_{dbHeaderNamespace}_ZONE_REGISTRY_H

#include <ace_time/internal/ZoneInfo.h>

namespace ace_time {{
namespace {dbNamespace} {{

const uint16_t kZoneRegistrySize = {numZones};

extern const {scope}::ZoneInfo* const kZoneRegistry[{numZones}];

}}
}}
#endif
"""
    def __init__(self, invocation, tz_version, tz_files, scope, db_namespace,
                 zones_map):
        self.invocation = invocation
        self.tz_version = tz_version
        self.tz_files = tz_files
        self.scope = scope
        self.db_namespace = db_namespace
        self.zones_map = zones_map

        self.db_header_namespace = self.db_namespace.upper()

    def generate_registry_cpp(self):
        zone_registry_items = ''
        for zone_name, eras in sorted(self.zones_map.items()):
            name = normalize_name(zone_name)
            zone_registry_items += f'  &kZone{name}, // {zone_name}\n'
        return self.ZONE_REGISTRY_CPP_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            scope=self.scope,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numZones=len(self.zones_map),
            zoneRegistryItems=zone_registry_items,
            progmem='ACE_TIME_PROGMEM')

    def generate_registry_h(self):
        return self.ZONE_REGISTRY_H_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            scope=self.scope,
            dbNamespace=self.db_namespace,
            dbHeaderNamespace=self.db_header_namespace,
            numZones=len(self.zones_map))

def to_modifier(modifier, scope):
    """Return the C++ TIME_MODIFIER_{X} corresponding to the 'w', 's', and 'u'
    modifier character in the TZ database files.
    """
    if modifier == 'w':
        return f'{scope}::ZoneContext::TIME_MODIFIER_W'
    elif modifier == 's':
        return f'{scope}::ZoneContext::TIME_MODIFIER_S'
    elif modifier == 'u':
        return f'{scope}::ZoneContext::TIME_MODIFIER_U'
    else:
        raise Exception(f'Unknown modifier {modifier}')

def to_tiny_year(year):
    if year == MAX_YEAR:
        return MAX_YEAR_TINY
    elif year == MIN_YEAR:
        return MIN_YEAR_TINY
    else:
        return year - EPOCH_YEAR


def normalize_raw(raw_line):
    """Replace hard tabs with 4 spaces.
    """
    return raw_line.replace('\t', '    ')
