#pragma once
#include <util/statistics.h>

using util::statistics::GeneralStatistics;


namespace util {
namespace statistics {

enum class ArcFlagsNames {
    source,
    target,
    distance,
    num_parts,
    filtering
};

std::ostream& operator<<(std::ostream& osm, const ArcFlagsNames& arg) {
    switch (arg) {
    case ArcFlagsNames::source:
        osm << "source";
        break;
    case ArcFlagsNames::target:
        osm << "target";
        break;
    case ArcFlagsNames::distance:
        osm << "distance";
        break;
    case ArcFlagsNames::num_parts:
        osm << "num_parts";
        break;
    case ArcFlagsNames::filtering:
        osm << "filtering";
        break;
    default:
        osm << "Unknown column";
        break;
    };
    return osm;
};


struct ArcFlagsMetricStatistics : GeneralStatistics {
    ArcFlagsMetricStatistics(const GeneralStatistics& base, size_t num_parts, double filtering)
        :GeneralStatistics(base), num_parts(ArcFlagsNames::num_parts, num_parts),
        filtering(ArcFlagsNames::filtering, filtering) {};
    StatisticsField<ArcFlagsNames, size_t> num_parts;
    StatisticsField<ArcFlagsNames, double> filtering;
};


std::ostream& operator<<(std::ostream& osm, const ArcFlagsMetricStatistics& arg) {
    osm << static_cast<GeneralStatistics>(arg) << '\t' << arg.num_parts << '\t' << 
        arg.filtering;
    return osm;
};

struct ArcFlagsQueryStatistic : ArcFlagsMetricStatistics {
    ArcFlagsQueryStatistic(const ArcFlagsMetricStatistics& base,
        size_t source, size_t target, size_t distance)
        :ArcFlagsMetricStatistics(base), source(ArcFlagsNames::source,source),
        target(ArcFlagsNames::target,target),distance(ArcFlagsNames::distance,distance)  {};
    StatisticsField<ArcFlagsNames,size_t> source;
    StatisticsField<ArcFlagsNames,size_t> target;
    StatisticsField<ArcFlagsNames,size_t> distance;
};


std::ostream& operator<<(std::ostream& osm, const ArcFlagsQueryStatistic& arg) {
    osm << static_cast<ArcFlagsMetricStatistics>(arg) << '\t' << arg.source << '\t' << arg.target << '\t' << arg.distance;
    return osm;
};



} //statistic
} //util
