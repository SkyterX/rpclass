#pragma once

#include <iostream>
#include <utility>
#include <graph/properties.hpp>
#include <graph/io/FileReader.hpp>

template <typename EdgeWeightProperty, typename BackInsertIterator>
int read_ddsg(BackInsertIterator backInserter, size_t& numOfNodes, size_t& numOfEdges, 
    const char* fileName) {
    using namespace std;
    graphIO::FileReader input;
    if (!input.Open(fileName)) {
        cerr << "File "<<fileName<<" not found!" << endl;
        return 1;
    }
    char ch = input.NextChar();
    if (ch != 'd') {
        std::cerr << "Wrong file format" << endl;
        input.Close();
        return 1;
    }
    numOfNodes = input.NextUnsignedInt();
    numOfEdges = input.NextUnsignedInt();
    size_t u, v, d;
    typename EdgeWeightProperty::value_type w;
    while (input.HasNext()) {
        u = input.NextUnsignedInt();
        v = input.NextUnsignedInt();
        w = input.NextUnsignedInt();
        d = input.NextUnsignedInt();
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
            input.Close();
            return 1;
        }
    }
   
    input.Close();
    return 0;
};

template <typename EdgeWeightProperty, typename EdgeDirectionProperty, typename BackInsertIterator>
int read_ddsg(BackInsertIterator backInserter, size_t& numOfNodes, size_t& numOfEdges,
    const char* fileName) {
    using namespace std;
    graphIO::FileReader input;
    if (!input.Open(fileName)) {
        cerr << "File " << fileName << " not found!" << endl;
        return 1;
    }
    char ch = input.NextChar();
    if (ch != 'd') {
        std::cerr << "Wrong file format" << endl;
        input.Close();
        return 1;
    }
    numOfNodes = input.NextUnsignedInt();
    numOfEdges = input.NextUnsignedInt();
    size_t u, v, d;
    typename EdgeWeightProperty::value_type w;
    using DirectionBit = typename EdgeDirectionProperty::value_type;
    while (input.HasNext()) {
        u = input.NextUnsignedInt();
        v = input.NextUnsignedInt();
        w = input.NextUnsignedInt();
        d = input.NextUnsignedInt();
        switch (d) {
        case 0:
        case 3:
			*backInserter++ = make_pair(make_pair(u, v), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(1))));
			*backInserter++ = make_pair(make_pair(v, u), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(2))));
			*backInserter++ = make_pair(make_pair(u, v), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(2))));
			*backInserter++ = make_pair(make_pair(v, u), graph::make_properties(EdgeWeightProperty(w), EdgeDirectionProperty(static_cast<DirectionBit>(1))));
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
            input.Close();
            return 1;
        }
    };

    input.Close();
    return 0;
};