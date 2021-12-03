#ifndef BENCHMARK_HINNANT_DATE_H
#define BENCHMARK_HINNANT_DATE_H

#include <vector>
#include <string>

extern void installTzDb(
    const std::string& installDir,
    const std::string& tzVersion);

extern void benchmarkHinnantDate(
    const std::vector<std::string>& zones,
    int startYear,
    int untilYear);

#endif
