#pragma once
#include "image.h"

class seedAffinityMatrix;
class image;
class tiles;

struct bfaItem
{
	double distance;
	unsigned int label;
	rgba rgba;
};

struct seedPosition
{
	seedPosition(int x, int y) : x(x), y(y) {}
	seedPosition() = default;
	int x;
	int y;
};

class bfaMatrix
{
public:
	bfaMatrix(int width, int height, image& image);
	~bfaMatrix();
	void bfaMatrix::placeSeed(const tiles& tiles, const seedAffinityMatrix& seedAffinityMatrix);
	void reSeed(const tiles& tiles, const seedAffinityMatrix& seedAffinityMatrix, double fraction, int maxSeeds);
	void deleteSmallSupervoxels(int cutoff);
	void saveLabels(const std::string & filename);

	int width;
	int height;
	int nCells;
	int nextLabel = 0;
	int resolvedLabels = 0;
	std::vector<bfaItem> data;
	std::vector<seedPosition> seedPositions;
};

class bfaSolver
{
public:
	virtual void solve(bfaMatrix& bfaMatrix, double regularization, bool isConnectivity8) const = 0;
};

class bfaRawSolver : public bfaSolver
{
public:
	virtual void solve(bfaMatrix& bfaMatrix, double regularization, bool isConnectivity8) const override;
protected:
	int doIteration(bfaMatrix& bfaMatrix, double regularization, int yStart, int yEnd, bool isConnectivity8) const;
};

class bfaMultiRawSolver : public bfaRawSolver
{
public:
	void solve(bfaMatrix& bfaMatrix, double regularization, bool isConnectivity8) const override;
private:
	int doIteration(bfaMatrix& bfaMatrix, double regularization, bool isConnectivity8) const;
};

class bfaHeapSolver : public bfaSolver
{
public:
	void solve(bfaMatrix& bfaMatrix, double regularization, bool isConnectivity8) const override;
};

