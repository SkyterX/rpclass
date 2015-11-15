#pragma once
#include <util/statistics.h>

using util::statistics::GeneralStatistics;


namespace util {
namespace statistics {

enum class GraphKeys {
    source,
    target,
    distance
};

std::ostream& operator<<(std::ostream& osm, const GraphKeys& arg) {
    switch (arg) {
    case GraphKeys::source:
        osm << "source";
        break;
    case GraphKeys::target:
        osm << "target";
        break;
    case GraphKeys::distance:
        osm << "distance";
        break;
    default:
        osm << "Unknown column";
        break;
    };
    return osm;
};


struct  DijkstraOneToAllSPStatistics : GeneralStatistics {
    DijkstraOneToAllSPStatistics(const GeneralStatistics& base, size_t source)
        :GeneralStatistics(base), source(GraphKeys::source,source) {};
    StatisticsField<GraphKeys,size_t> source;
};

struct DijkstraSSSPStatistics : GeneralStatistics {
    DijkstraSSSPStatistics(const GeneralStatistics& base,
        size_t source, size_t target, size_t distance)
        :GeneralStatistics(base), source(GraphKeys::source,source), target(GraphKeys::target,target),distance(GraphKeys::distance,distance)  {};
    StatisticsField<GraphKeys,size_t> source;
    StatisticsField<GraphKeys,size_t> target;
    StatisticsField<GraphKeys,size_t> distance;
};

struct BFSStatistics : GeneralStatistics {
    using GeneralStatistics::GeneralStatistics;
};

std::ostream& operator<<(std::ostream& osm, const DijkstraOneToAllSPStatistics& arg) {
    osm << static_cast<GeneralStatistics>(arg) << '\t' << arg.source;
    return osm;
};

std::ostream& operator<<(std::ostream& osm, const DijkstraSSSPStatistics& arg) {
    osm << static_cast<GeneralStatistics>(arg) << '\t' << arg.source << '\t' << arg.target << '\t' << arg.distance;
    return osm;
};

} //statistic
} //util
