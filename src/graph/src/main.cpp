#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <graph/OSMGraph.hpp>
#include <graph/GraphStatistics.hpp>
#include <graph/io/FileReader.hpp>
#include <ctime>

using namespace std;
using namespace graph;
using namespace graphIO;
using namespace graphOSM;


int main() {
	double time = clock();

	FileReader fileReader;
	//fileReader.Open("input.txt");
	fileReader.Open("osm-bawu.gr");
	//fileReader.Open("osm-ger.gr");

	auto g = ReadGraphFrom(fileReader);
	printf("Graph reading time: %.3lf\n", (clock() - time) / CLOCKS_PER_SEC);
	fileReader.Close();

	time = clock();
	printf("Max Degree: %d\n", GraphStatistics::GetMaximalVertexDegree(*g));
	printf("Min Degree: %d\n", GraphStatistics::GetMinimalVertexDegree(*g));
	printf("Average Degree: %.3lf\n", GraphStatistics::GetAverageVertexDegree(*g));
	printf("Single oriented edges count: %d\n", GraphStatistics::GetSingleOrientedEdgesCount(*g));

	printf("Statistics time: %.3lf\n", (clock() - time) / CLOCKS_PER_SEC);
	scanf("%lf", &time);

	return 0;
}
