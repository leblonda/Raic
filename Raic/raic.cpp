#include "stdafx.h"
#include <iostream>
#include <memory>
#include <fstream>
#include "image.h"
#include "bfaMatrix.h"
#include "seedAffinityMatrix.h"
#include "tiles.h"

int main(int argc, char** argv)
{
	using namespace std;

	cout << "RAIC 1.0, Copyright 2018, Antoine Leblond, Claude Kauffmann\r\n\r\n";
	
	if (argc != 3)
	{
		cout << "Usage: RAIC inputFilename outputFilename\r\n";
		cout << "Parameters are defined in code.\r\n";
		return 1;
	}

	string source(argv[1]);
	string dest(argv[2]);
	
	int seedInterval = 10;
	double regularizationFactor = 0.75;
	auto reseedingFractions = vector<double>{ 0.25,0.20,0.15 };
	int numberOfIterations = int(reseedingFractions.size());
	int maximumSeeds = 4000;
	int minimumSize = 0;
	double seedWindowFraction = 0.75;
	double reseedWindowFraction = 1;
	bool isConnectivity8 = true;
	bool isLAB = true;
	bool isRAIC = false;

	if (isRAIC)
	{
		reseedWindowFraction = 0;
		numberOfIterations = 0;
		reseedingFractions.clear();
		maximumSeeds = 0;
	}

	cout << "Input file: " << source << "\r\n";
	cout << "Output file: " << dest << "\r\n";
	cout << "Is RAIC: " << (isRAIC? "true" : "false") << "\r\n";
	cout << "Seed interval: " << seedInterval << "\r\n";
	cout << "Seed Window fraction: " << seedWindowFraction << "\r\n";
	cout << "Reseed Window fraction: " << reseedWindowFraction << "\r\n";
	cout << "Regularization factor: " << regularizationFactor << "\r\n";
	cout << "Number of reseeding iterations: " << numberOfIterations << "\r\n";
	cout << "Is 8 connected: " << (isConnectivity8 ? "true" : "false") << "\r\n";
	cout << "Is LAB: " << (isLAB ? "true" : "false") << "\r\n";
	cout << "Reseeding fractions:";
	for (auto x : reseedingFractions)
		cout << " " << x;
	cout << "\r\n";
	cout << "Maximal number of seeds: " << maximumSeeds << "\r\n";
	cout << "Minimal superpixel size: " << minimumSize << "\r\n\r\n";

	image image(source, isLAB);

	tiles tiles(image.width, image.height, seedInterval, seedInterval, seedWindowFraction, reseedWindowFraction, isRAIC);
	cout << "Number of initial seeds: " << tiles.getNumberOfTiles() << "\r\n";

	auto bfa_matrix = make_unique<bfaMatrix>(image.width, image.height, image);

	seedAffinityMatrix seed_affinity_matrix{};
	seed_affinity_matrix.Calculate(image);

	bfa_matrix->placeSeed(tiles, seed_affinity_matrix);

	unique_ptr<bfaSolver> seedSolver = make_unique<bfaMultiRawSolver>();
	unique_ptr<bfaSolver> reseedSolver = make_unique<bfaHeapSolver>();

	seedSolver->solve(*bfa_matrix, regularizationFactor, isConnectivity8);

	for (int i = 0; i < numberOfIterations; i++)
	{
		bfa_matrix->reSeed(tiles, seed_affinity_matrix, reseedingFractions[i], maximumSeeds);
		reseedSolver->solve(*bfa_matrix, regularizationFactor, isConnectivity8);

		if (bfa_matrix->nextLabel == maximumSeeds)
			break;
	}

	if (minimumSize != 0)
	{
		bfa_matrix->deleteSmallSupervoxels(minimumSize);
		seedSolver->solve(*bfa_matrix, regularizationFactor, isConnectivity8);
	}
	
	bfa_matrix->saveLabels(dest);
	
	cout << "Number of labels: " << bfa_matrix->nextLabel << "\r\n";
	cout << "\r\n";

	return 0;
}