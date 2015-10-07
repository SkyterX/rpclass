#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <graph/static_graph.hpp>
#include <graph/GraphStatistics.hpp>
#include <graph/FileReader.hpp>
#include <ctime>

using namespace std;
using namespace graph;


int main() {
	double time = clock();

	FileReader fileReader;
//	fileReader.Open("input.txt");
	fileReader.Open("osm-bawu.gr");
	//fileReader.Open("osm-ger.gr");

	auto g = ReadGraphFrom(fileReader);

	fileReader.Close();

	printf("Max Degree: %d\n", GraphStatistics::GetMaximalVertexDegree(*g));
	printf("Min Degree: %d\n", GraphStatistics::GetMinimalVertexDegree(*g));
	printf("Average Degree: %.3lf\n", GraphStatistics::GetAverageVertexDegree(*g));
	printf("Single oriented edges count: %d\n", GraphStatistics::GetSingleOrientedEdgesCount(*g));

	printf("Elapsed time: %.3lf\n", (clock() - time) / CLOCKS_PER_SEC);
	scanf("%lf", &time);

	delete g;

	return 0;
}
