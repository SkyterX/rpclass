#pragma once
#include <util/statistics.h>

using util::statistics::GeneralStatistics;


namespace util {
namespace statistics {
struct  DijkstraOneToAllSPStatistics : GeneralStatistics {
    DijkstraOneToAllSPStatistics(const GeneralStatistics& base, size_t source)
        :GeneralStatistics(base), source(source) {};
    size_t source;
};

struct DijkstraSSSPStatistics : GeneralStatistics {
    DijkstraSSSPStatistics(const GeneralStatistics& base,
        size_t source, size_t target, size_t distance)
        :GeneralStatistics(base), source(source), target(target),distance(distance)  {};
    size_t source;
    size_t target;
    size_t distance;
};

struct BFSStatistics : GeneralStatistics {
    using GeneralStatistics::GeneralStatistics;
};

std::ostream& operator<<(std::ostream& osm, const DijkstraOneToAllSPStatistics& arg) {
    osm << static_cast<GeneralStatistics>(arg) << 
        ",source=" << '\"' << arg.source << '\"';
    return osm;
};

std::ostream& operator<<(std::ostream& osm, const DijkstraSSSPStatistics& arg) {
    osm << static_cast<GeneralStatistics>(arg) << ",source=" << '\"' << arg.source << '\"' <<
        "\ttarget=" << '\"' << arg.target << '\"' << "\tdistance=" << '\"' << arg.distance << '\"';
    return osm;
};

} //statistic
} //util
