#pragma once
#include <util/statistics.h>
#include <ch/contraction_hierarchy.hpp>

using util::statistics::GeneralStatistics;


namespace util {
namespace statistics {

enum class CHNames:char {
    source,
    target,
    distance,
    dij_limit,
    priority,
    stalling
};

std::ostream& operator<<(std::ostream& osm, const CHNames& arg) {
    switch (arg) {
    case CHNames::source:
        osm << "source";
        break;
    case CHNames::target:
        osm << "target";
        break;
    case CHNames::distance:
        osm << "distance";
        break;
    case CHNames::dij_limit:
        osm << "dij_limit";
        break;
    case CHNames::priority:
        osm << "priority";
        break;
    case CHNames::stalling:
        osm << "stalling";
        break;

    default:
        osm << "Unknown column";
        break;
    };
    return osm;
};

enum class CHPriority:char {
    shortcut,
    HL
};

std::ostream& operator<<(std::ostream& osm, const CHPriority& arg) {
    switch (arg) {
    case CHPriority::HL:
        osm << "hub_labeling";
        break;
    case CHPriority::shortcut:
        osm << "shortcut";
        break;
    default:
        osm << "Unknown priority";
        break;
    };
    return osm;
}

struct CHMetricStatistics : GeneralStatistics {
    CHMetricStatistics(const GeneralStatistics& base, size_t dij_limit, CHPriority priority)
        :GeneralStatistics(base), dij_limit(CHNames::dij_limit, dij_limit),
        priority(CHNames::priority, priority) {};
    StatisticsField<CHNames, size_t> dij_limit;
    StatisticsField<CHNames, CHPriority> priority;
};


std::ostream& operator<<(std::ostream& osm, const CHMetricStatistics& arg) {
    osm << static_cast<GeneralStatistics>(arg) << '\t' << arg.dij_limit << '\t' << 
        arg.priority;
    return osm;
};

struct CHQueryStatistic : CHMetricStatistics {
    CHQueryStatistic(const CHMetricStatistics& base,
        size_t source, size_t target, size_t distance, bool stalling)
        :CHMetricStatistics(base), source(CHNames::source,source),
        target(CHNames::target,target),distance(CHNames::distance,distance),
        stalling(CHNames::stalling,stalling) {};
    StatisticsField<CHNames,size_t> source;
    StatisticsField<CHNames,size_t> target;
    StatisticsField<CHNames,size_t> distance;
    StatisticsField<CHNames, bool> stalling;
};


std::ostream& operator<<(std::ostream& osm, const CHQueryStatistic& arg) {
    osm << static_cast<CHMetricStatistics>(arg) << '\t' << arg.source << '\t' <<
        arg.target << '\t' << arg.distance << '\t' << arg.stalling;
    return osm;
};



} //statistic
} //util
