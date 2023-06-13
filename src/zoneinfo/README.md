# ZoneInfo Data Files

These are internal representations of the zoneinfo  data from the [IANA TZ
database](https://www.iana.org/time-zones). Library users should treat the
data objects in the various `basic::`, `extended::`, `and `complete::`
namespaces as *opaque*. They should be referenced only by the specific
`basic::kZone*` or `extended::kZone*` identifiers defined in the `src/zonedb`,
`src/zonedbx`, `src/zonedbc` directories. The internal structure of these data
types will change when the internal implementations change.

There are 5 data types that describe the storage format of the database:
`ZoneContext`, `ZoneRule`, `ZonePolicy`, `ZoneEra` and `ZoneInfo`. These data
types are analogous to tables in a relational database. These objects come in 3
slightly different implementations, placed in 3 different namespaces:

* `ZoneInfoLow.h`
    * low resolution persistence format
    * `zoneinfolow::ZoneContext<>`
    * `zoneinfolow::ZoneRule<>`
    * `zoneinfolow::ZonePolicy<>`
    * `zoneinfolow::ZoneEra<>`
    * `zoneinfolow::ZoneInfo<>`
* `ZoneInfoMid.h`
    * medium resolution persistence format
    * `zoneinfomid::ZoneContext<>`
    * `zoneinfomid::ZoneRule<>`
    * `zoneinfomid::ZonePolicy<>`
    * `zoneinfomid::ZoneEra<>`
    * `zoneinfomid::ZoneInfo<>`
* `ZoneInfoHigh.h`
    * high resolution persistence format
    * `zoneinfohigh::ZoneContext<>`
    * `zoneinfohigh::ZoneRule<>`
    * `zoneinfohigh::ZonePolicy<>`
    * `zoneinfohigh::ZoneEra<>`
    * `zoneinfohigh::ZoneInfo<>`

The `infos.h` file provide 3 specific template instantiations of the above set
of classes:

* `basic::ZoneXxx`
* `extended::ZoneXxx`
* `complete::ZoneXxx`

Wrapping each of these low-level persistent classes are the "broker" layer
classes. They convert the low-level storage formats into a consistent set
of fields with identical types and integer sizes, so that they can be used by
the code in the `src/ace_time` layer to determine the time zone DST transition
information.

* `BrokersLow.h`
    * `zoneinfolow::ZoneContextBroker<>`
    * `zoneinfolow::ZoneRuleBroker<>`
    * `zoneinfolow::ZonePolicyBroker<>`
    * `zoneinfolow::ZoneEraBroker<>`
    * `zoneinfolow::ZoneInfoBroker<>`
* `BrokersMid.h`
    * `zoneinfomid::ZoneContextBroker<>`
    * `zoneinfomid::ZoneRuleBroker<>`
    * `zoneinfomid::ZonePolicyBroker<>`
    * `zoneinfomid::ZoneEraBroker<>`
    * `zoneinfomid::ZoneInfoBroker<>`
* `BrokersHigh.h`
    * `zoneinfohigh::ZoneContextBroker<>`
    * `zoneinfohigh::ZoneRuleBroker<>`
    * `zoneinfohigh::ZonePolicyBroker<>`
    * `zoneinfohigh::ZoneEraBroker<>`
    * `zoneinfohigh::ZoneInfoBroker<>`

Similar to `infos.h`, the `brokers.h` provide 3 specific template instanations
of the above classes:

* `basic::ZoneXxxBroker`
* `extended::ZoneXxxBroker`
* `complete::ZoneXxxBroker`

Each of the namespaces correspond to the 3 implementations of the
`ZoneProcessor`:

* `BasicZoneProcessor.h`
    * bound to `basic::` classes
* `ExtendedZoneProcessor.h`
    * bound to `extended::` classes
* `CompleteZoneProcessor.h`
    * bound to `complete::` classes

(TODO: Add some blurb about how the broker layer abstracts away the
implementation detail of specific `zoneinfoXXX` data records. It is
theoretically possible for any `ZoneProcessor` to use any of the `ZoneXxxBroker`
objects in any namespace. However, to keep things manageable, the 3 low/mid/high
persistence layers are mapped to the corresponding Basic/Extended/Complete
ZoneProcessors.)
