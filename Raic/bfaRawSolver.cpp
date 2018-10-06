#include "stdafx.h"
#include "bfaMatrix.h"
#include <ppl.h>

namespace
{
	inline double calculateDistance2(rgba* a, rgba* b)
	{
		double dr = double(a->r) - double(b->r);
		double dg = double(a->g) - double(b->g);
		double db = double(a->b) - double(b->b);
		return dr * dr + dg * dg + db * db;
	}

	inline void test(bfaItem* p, int myLabel, double myDistance, int& nUpdated, int delta)
	{
		auto cp = p + delta;
		auto d = calculateDistance2(&p->rgba, &cp->rgba) + myDistance;
		if (d < cp->distance)
		{
			cp->label = myLabel;
			cp->distance = d;
			nUpdated++;
		}
	}
}

int bfaRawSolver::doIteration(bfaMatrix& bfaMatrix,double regularization, int yStart, int yEnd, bool isConnectivity8) const
{
	auto p = &bfaMatrix.data[0] + yStart*bfaMatrix.width;
	int nUpdated = 0;
	int h1 = bfaMatrix.height - 1;
	int w1 = bfaMatrix.width - 1;
	for (int y = yStart; y < yEnd; y++)
	{
		for (int x = 0; x < bfaMatrix.width; x++)
		{
			auto myLabel = p->label;
			if (myLabel & 0x80000000)
			{
				auto myDistance = p->distance + regularization;

				if (y != 0)
				{
					auto wm = -static_cast<int>(bfaMatrix.width);
					test(p, myLabel, myDistance, nUpdated, wm);

					if (isConnectivity8)
					{
						if (x != 0)
						{
							test(p, myLabel, myDistance, nUpdated, wm - 1);
						}
						if (x != w1)
						{
							test(p, myLabel, myDistance, nUpdated, wm + 1);
						}
					}

				}

				if (x != 0)
				{
					test(p, myLabel, myDistance, nUpdated, -1);
				}
				if (x != w1)
				{
					test(p, myLabel, myDistance, nUpdated, 1);
				}

				if (y != h1)
				{
					test(p, myLabel, myDistance, nUpdated, bfaMatrix.width);

					if (isConnectivity8)
					{
						if (x != 0)
						{
							test(p, myLabel, myDistance, nUpdated, bfaMatrix.width - 1);
						}
						if (x != w1)
						{
							test(p, myLabel, myDistance, nUpdated, bfaMatrix.width + 1);
						}
					}
				}

				p->label = myLabel & 0x7FFFFFFF;
			}
			p++;
		}
	}
	return nUpdated;
}

int bfaMultiRawSolver::doIteration(bfaMatrix& bfaMatrix, double regularization, bool isConnectivity8) const
{
	std::atomic<int> count = 0;

	constexpr int step = 4;
	int nSteps = (bfaMatrix.height + step * 2 - 1) / (step * 2) + 1;

	Concurrency::parallel_for(0, nSteps, [&](size_t i)
	{
		int y = int(i * step * 2);
		if (y >= bfaMatrix.height)
			return;
		count += bfaRawSolver::doIteration(bfaMatrix, regularization, y, std::min(y + step, bfaMatrix.height), isConnectivity8);
	});

	Concurrency::parallel_for(0, nSteps, [&](int i)
	{
		int y = i * step * 2 + step;
		if (y >= bfaMatrix.height)
			return;
		count += bfaRawSolver::doIteration(bfaMatrix,regularization, y, std::min(y + step, bfaMatrix.height), isConnectivity8);
	});

	return count;
}

void bfaRawSolver::solve(bfaMatrix & bfaMatrix, double regularization, bool isConnectivity8) const
{
	while (doIteration(bfaMatrix, regularization,0,bfaMatrix.height, isConnectivity8) != 0)
	{
	}
	bfaMatrix.resolvedLabels = bfaMatrix.nextLabel;
}

void bfaMultiRawSolver::solve(bfaMatrix & bfaMatrix, double regularization, bool isConnectivity8) const
{
	while (doIteration(bfaMatrix, regularization, isConnectivity8) != 0)
	{
	}
	bfaMatrix.resolvedLabels = bfaMatrix.nextLabel;
}

