#pragma once
#include <string>
#include <ostream>

namespace util{
namespace statistics {

enum class GeneralKeys : char {
    graph,
    algorithm,
    phase, 
    metric, 
    vertices,
    edges,
    time,
    space
};

std::ostream& operator<<(std::ostream& osm, const GeneralKeys& arg) {
    switch (arg) {
    case GeneralKeys::algorithm:
        osm << "algorithm";
        break;
    case GeneralKeys::edges:
        osm << "edges";
        break;
    case GeneralKeys::graph:
        osm << "graph";
        break;
    case GeneralKeys::metric:
        osm << "metric";
        break;
    case GeneralKeys::phase:
        osm << "phase";
        break;
    case GeneralKeys::space:
        osm << "space";
        break;
    case GeneralKeys::time:
        osm << "time";
        break;
    case GeneralKeys::vertices:
        osm << "vertices";
        break;
    default:
        osm << "Unknown column";
        break;
    };
    return osm;
};


enum class Algorithm: char {
    bfs,
    dijkstra,
    dijkstraPtoP,
    biDijkstra,
    arcFlags
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
    case Algorithm::arcFlags:
        osm << "arcFlags";
        break;
    default:
        osm << "Unknown algorithm";
        break;
    };
    return osm;
};
enum class Phase : char {
    topology,
    metric,
    query
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

enum class Metric : char {
    time
};

std::ostream& operator<<(std::ostream& osm, const Metric& arg) {
    switch (arg) {
    case Metric::time:
        osm << "time";
        break;
    default:
        osm << "unknown metric";
        break;
    };
return osm;
};


template <typename Key, typename Value>
struct StatisticsField {
    StatisticsField(Key key, const Value& value)
    :key(key),value(value){};
    Key key;
    Value value;
};

template <typename Key, typename Value>
std::ostream& operator<<(std::ostream& osm, const StatisticsField<Key, Value>& arg) {
    osm << arg.key << ":" << arg.value;
    return osm;
};


struct GeneralStatistics {
    StatisticsField<GeneralKeys,std::string> graph;
    StatisticsField<GeneralKeys,Algorithm> algorithm;
    StatisticsField<GeneralKeys,Phase> phase;
    StatisticsField<GeneralKeys,Metric> metric;
    StatisticsField<GeneralKeys,uint64_t> vertices;
    StatisticsField<GeneralKeys,uint64_t> edges;
    StatisticsField<GeneralKeys,uint64_t> time;
    StatisticsField<GeneralKeys,uint64_t> space;
    GeneralStatistics(const std::string& graph,
        const Algorithm& algorithm, const Phase& phase, const Metric& metric,
        const uint64_t& vertices, uint64_t edges, const uint64_t& time,
        const uint64_t& space)
        :graph(GeneralKeys::graph,graph),
        algorithm(GeneralKeys::algorithm,algorithm),
        phase(GeneralKeys::phase,phase),
        metric(GeneralKeys::metric, metric),
        vertices(GeneralKeys::vertices,vertices), 
        edges(GeneralKeys::edges,edges),
        time(GeneralKeys::time,time),
        space(GeneralKeys::space,space) {};
};

std::ostream& operator<<(std::ostream& osm, const GeneralStatistics& arg) {
    osm << arg.graph << '\t' << arg.algorithm << '\t' << arg.phase << '\t' << arg.metric << '\t' <<
        arg.vertices << '\t'<< arg.edges << '\t'<< arg.time << '\t' << arg.space;
return osm;
};

}; //statistics
}; //util