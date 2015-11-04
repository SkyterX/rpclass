#pragma once

#include <fstream>
#include <iostream>
#include <utility>

template <typename EdgeWeightProperty, typename BackInsertIterator>
int read_ddsg(BackInsertIterator backInserter, size_t& numOfNodes, size_t& numOfEdges, 
    const char* fileName) {
    using namespace std;
    ifstream input(fileName);
    if (!input.is_open()) {
        cerr << "File not found!" << endl;
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
            *backInserter++ = make_pair(make_pair(u, v), EdgeWeightProperty(w));
            *backInserter++ = make_pair(make_pair(v, u), EdgeWeightProperty(w));
            ++numOfEdges;
             break;
        case 1:
        case 2:
            *backInserter++ = make_pair(make_pair(v, u), EdgeWeightProperty(w));
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