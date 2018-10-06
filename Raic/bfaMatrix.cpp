#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <ppl.h>
#include "seedAffinityMatrix.h"
#include "tiles.h"
#include "bfaMatrix.h"
#include "image.h"

bfaMatrix::bfaMatrix(int width, int height, image& image) : width(width), height(height), nCells(width*height)
{
	data = std::vector<bfaItem>(nCells, bfaItem());
	auto p = &data[0];
	auto ip = &image.data[0];
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			p->label = 0;
			p->distance = 0x7FFFFFFF;
			p->rgba = *ip;
			p++;
			ip++;
		}
	}
}

bfaMatrix::~bfaMatrix()
{
}

void bfaMatrix::placeSeed(const tiles& tiles, const seedAffinityMatrix& seedAffinityMatrix)
{
	int label = 0;
	for (auto & y : tiles.getYSeedDomains())
	{
		for (auto & x : tiles.getXSeedDomains())
		{
			double maxAffinity = 0;
			int maxX = (x.start+x.stop)/2;
			int maxY = (y.start+y.stop)/2;

			for (int yy = y.start; yy < y.stop; yy++)
			{
				for (int xx = x.start; xx < x.stop; xx++)
				{
					auto p2 = &seedAffinityMatrix.data[0] + width * yy + xx;
					if (*p2 > maxAffinity)
					{
						maxAffinity = *p2;
						maxX = xx;
						maxY = yy;
					}
				}
			}

			auto p = &data[0] + width * maxY + maxX;
			p->distance = 0;
			p->label = label | 0x80000000;
			seedPositions.push_back(seedPosition(maxX, maxY));
			label++;
		}
	}
	nextLabel = label;
}

struct seedCandidate
{
	seedCandidate() = default;
	seedCandidate(int x, int y, double metric)
		: x(x),
		  y(y),
		  metric(metric)
	{
	}

	int x;
	int y;
	double metric;
};

inline bool compareCandidate(const seedCandidate &a, const seedCandidate &b)
{
	return a.metric>b.metric;
}

void bfaMatrix::reSeed(const tiles& tiles, const seedAffinityMatrix& seedAffinityMatrix, double fraction, int maxSeeds)
{
	auto maxItems = tiles.getNumberOfTiles();
	std::vector<seedCandidate> candidates{};

	int label = nextLabel;
	auto pData = &data[0];
	auto pAffinity = &seedAffinityMatrix.data[0];
	for (auto & y : tiles.getYReseedDomains())
	{
		for (auto & x : tiles.getXReseedDomains())
		{
			double maxD = -1;
			int maxX = 0;
			int maxY = 0;

			for (int yy = y.start; yy < y.stop; yy++)
			{
				for (int xx = x.start; xx < x.stop; xx++)
				{
					auto candidate = static_cast<double>((pData + yy*width + xx)->distance);
					candidate *= candidate;
					candidate *= *(pAffinity + yy*width + xx);
					if (candidate > maxD)
					{
						maxD = candidate;
						maxX = xx;
						maxY = yy;
					}
				}
			}
			candidates.push_back(seedCandidate(maxX, maxY, maxD));
		}
	}

	auto s = candidates.size();

	sort(candidates.begin(), candidates.end(), compareCandidate);

	auto cs = int(double(candidates.size()) * fraction);
	if(label+cs>maxSeeds)
	{
		cs = maxSeeds - label;
	}
	for(int i=0;i<cs;i++)
	{
		auto found = pData + candidates[i].y * width + candidates[i].x;
		found->distance = 0;
		found->label = label | 0x80000000;
		seedPositions.push_back(seedPosition(candidates[i].x, candidates[i].y));
		label++;
	}

	nextLabel = label;
}

void bfaMatrix::deleteSmallSupervoxels(int cutoff)
{
	std::vector<int> counts(nextLabel,0);
	for(auto &x : data)
	{
		counts[x.label]++;
		x.distance = 0x7FFFFFFF;
	}
	int i = 0;
	int label = 0;
	std::vector<seedPosition> newSeedPositions;

	for(auto const &seed : seedPositions)
	{
		if(counts[i]>=cutoff)
		{
			newSeedPositions.push_back(seed);
			int index = seed.y*width + seed.x;
			data[index].distance = 0;
			data[index].label = label | 0x80000000;
			label++;
		}
		i++;
	}

	seedPositions.swap(newSeedPositions);
	nextLabel = label;
	resolvedLabels = 0;
}

void bfaMatrix::saveLabels(const std::string& filename)
{
	int nCells = width*height;
	std::vector<int> labels(nCells,0);
	
	for(int i =0;i<nCells;i++)
	{
		auto label = data[i].label;
		if((label & 0x80000000)!=0)
		{
			label = label & 0x7FFFFFFF;
		}
		labels[i] = label;
	}

	std::ofstream fs;
	fs.open(filename.c_str(), std::ios::binary);
	fs.write(reinterpret_cast<const char*>(&labels[0]), nCells * sizeof(int));
	fs.close();
}
