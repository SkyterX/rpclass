#pragma once

#include <fstream>
#include <iostream>
#include <utility>
#include <graph/properties.hpp>

template <typename EdgeWeightProperty, typename BackInsertIterator>
int read_ddsg(BackInsertIterator backInserter, size_t& numOfNodes, size_t& numOfEdges, 
    const char* fileName) {
    using namespace std;
    ifstream input(fileName);
    if (!input.is_open()) {
        cerr << "File "<<fileName<<" not found!" << endl;
        return 1;
    }
    char ch;
    input >> ch;
    if (ch != 'd') {
        std::cerr << "Wrong file format" << endl;
        input.close();
        return 1;
    };
    input >> numOfNodes >> numOfEdges;
    size_t u, v, d;
    typename EdgeWeightProperty::value_type w;
    while (input >> u >> v >> w >> d) {
        switch (d) {
        case 0:
        case 3: 
            *backInserter++ = make_pair(make_pair(u, v), graph::make_properties(EdgeWeightProperty(w)));
            *backInserter++ = make_pair(make_pair(v, u), graph::make_properties(EdgeWeightProperty(w)));
            ++numOfEdges;
             break;
        case 1:
            *backInserter++ = make_pair(make_pair(u, v), graph::make_properties(EdgeWeightProperty(w)));
            break;
        case 2:
            *backInserter++ = make_pair(make_pair(v, u), graph::make_properties(EdgeWeightProperty(w)));
             break;
        default:
            std::cerr << "Wrong file format" << endl;
            input.close();
            return 1;
        }
    };
   
    input.close();
    return 0;
};

template <typename EdgeWeightProperty, typename EdgeDirectionProperty, typename BackInsertIterator>
int read_ddsg(BackInsertIterator backInserter, size_t& numOfNodes, size_t& numOfEdges,
    const char* fileName) {
    using namespace std;
    ifstream input(fileName);
    if (!input.is_open()) {
        cerr << "File " << fileName << " not found!" << endl;
        return 1;
    }
    char ch;
    input >> ch;
    if (ch != 'd') {
        std::cerr << "Wrong file format" << endl;
        input.close();
        return 1;
    };
    input >> numOfNodes >> numOfEdges;
    size_t u, v, d;
    typename EdgeWeightProperty::value_type w;
    using DirectionBit = typename EdgeDirectionProperty::value_type;
    while (input >> u >> v >> w >> d) {
        switch (d) {
        case 0:
        case 3:
            *backInserter++ = make_pair(make_pair(u, v), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(0))));
            *backInserter++ = make_pair(make_pair(v, u), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(0))));
            ++numOfEdges;
            break;
        case 1:
            *backInserter++ = make_pair(make_pair(u, v), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(1))));
            *backInserter++ = make_pair(make_pair(v, u), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(2))));
            break;
        case 2:
            *backInserter++ = make_pair(make_pair(u, v), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(2))));
            *backInserter++ = make_pair(make_pair(v, u), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(1))));
            break;
        default:
            std::cerr << "Wrong file format" << endl;
            input.close();
            return 1;
        }
    };

    input.close();
    return 0;
};