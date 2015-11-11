#pragma once
#include <string>
#include <ostream>

namespace util{
namespace statistics {

enum class Algorithm: char {
    bfs = 0,
    dijkstra = 1,
    dijkstraPtoP = 2,
    biDijkstra = 3
};

enum class Metric : char {
    time = 0
};

enum class Phase : char {
    topology = 0,
    metric = 1,
    query = 2
};

std::ostream& operator<<(std::ostream& osm, const Algorithm& arg) {
    switch (arg) {
    case Algorithm::bfs:
        osm << "bfs";
        break;
    case Algorithm::dijkstra: 
        osm << "dijkstra";
        break;
    case Algorithm::dijkstraPtoP:
        osm << "dijkstraPtP";
        break;
    case Algorithm::biDijkstra:
        osm << "biDijkstra";
        break;
    default:
        osm << "Unknown algorithm";
        break;
    };
return osm;
};


std::ostream& operator<<(std::ostream& osm, const Metric& arg) {
    switch (arg) {
    case Metric::time:
        osm << "time";
        break;
    default:
        osm << "unkonwn metric";
        break;
    };
return osm;
};

std::ostream& operator<<(std::ostream& osm, const Phase& arg) {
    switch (arg) {
    case Phase::topology:
        osm << "topology";
        break;
    case Phase::metric:
        osm << "metric";
        break;
    case Phase::query:
        osm << "query";
        break;
    default:
        osm << "unkonwn phase";
        break;
    };
    return osm;;
};

struct GeneralStatistics {
    std::string graphName;
    Algorithm algorithm;
    Phase phase;
    Metric metric;
    uint64_t time,
             space;
    bool turns = false;
    GeneralStatistics(const std::string& graphName,
        const Algorithm& algorithm, const Phase& phase, const Metric& metric,
        const uint64_t& time, const uint64_t& space, bool turns)
        :graphName(graphName),
        algorithm(algorithm),
        phase(phase),
        metric(metric),
        time(time),
        space(space),
        turns(turns) {};
};

std::ostream& operator<<(std::ostream& osm, const GeneralStatistics& arg) {
    osm << arg.graphName << ',' << arg.algorithm << ',' << arg.phase << ',' << arg.metric << ',' <<
        arg.time << ',' << arg.space << ',' << arg.turns;
return osm;
};

}; //statistics
}; //util