#pragma once

#include <algorithm>
#include <vector>
#include <utility>

template <typename BackInsertIterator, typename SizeT = uint64_t>
void generate_list_graph( BackInsertIterator backInserter, SizeT n) {
    using namespace std;
    vector<SizeT> shuffle;
    shuffle.reserve(n);
    for (SizeT i = 0; i < n; ++i) shuffle.push_back(i);
    random_shuffle(shuffle.begin(), shuffle.end());
    for (SizeT i = 0; i < n; ++i) *backInserter = make_pair(i, shuffle[i]);
};