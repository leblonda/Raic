#include "stdafx.h"
#include <queue>
#include "bfaMatrix.h"

namespace
{
	inline double calculateDistance2(rgba* a, rgba* b)
	{
		double dr = double(a->r) - double(b->r);
		double dg = double(a->g) - double(b->g);
		double db = double(a->b) - double(b->b);
		return dr * dr + dg * dg + db * db;
	}

	struct propagation
	{
		propagation(int x, int y, double distance):x(x),y(y),distance(distance){}
		int x;
		int y;
		double distance;
	};

	class comp {
	public:
		inline bool operator()(propagation& t1, propagation& t2)
		{
			return t1.distance > t2.distance;
		}
	};

	inline void test(bfaItem* p, int myLabel, double myDistance, int delta, int x, int y, std::priority_queue<propagation, std::vector<propagation>, comp>& pq)
	{
		auto cp = p + delta;
		auto d = calculateDistance2(&p->rgba, &cp->rgba) + myDistance;
		if (d < cp->distance)
		{	
			cp->label = myLabel;
			cp->distance = d;
			pq.push(propagation(x,y,d));
		}
	}
}

void bfaHeapSolver::solve(bfaMatrix & bfaMatrix, double regularization, bool isConnectivity8) const
{
	auto pq = std::priority_queue<propagation, std::vector<propagation>, comp>{};
	
	{
		auto startSeed = bfaMatrix.resolvedLabels;
		auto stopSeed = bfaMatrix.nextLabel;
		auto & seeds = bfaMatrix.seedPositions;
		for (int i = startSeed; i < stopSeed; i++)
		{
			pq.push(propagation(seeds[i].x, seeds[i].y, 0));
		}
	}

	auto & dataMatrix = bfaMatrix.data;
	auto height = bfaMatrix.height;
	auto width = bfaMatrix.width;
	int h1 = bfaMatrix.height - 1;
	int w1 = bfaMatrix.width - 1;

	while (!pq.empty())
	{
		auto item = pq.top();
		auto x = item.x;
		auto y = item.y;
		auto cell = &dataMatrix[y*width + x];
		pq.pop();
		if(cell->distance<item.distance)
			continue;
		auto myLabel = cell->label;
		auto myDistance = cell->distance + regularization;

		if (y != 0)
		{
			auto wm = -static_cast<int>(bfaMatrix.width);
			test(cell, myLabel, myDistance, wm, x,y-1,pq);

			if (isConnectivity8)
			{
				if (x != 0)
				{
					test(cell, myLabel, myDistance, wm - 1, x - 1, y - 1, pq);
				}
				if (x != w1)
				{
					test(cell, myLabel, myDistance, wm + 1, x + 1, y - 1, pq);
				}
			}
		}

		if (x != 0)
		{
			test(cell, myLabel, myDistance, -1, x - 1, y, pq);
		}
		if (x != w1)
		{
			test(cell, myLabel, myDistance, 1, x + 1, y, pq);
		}

		if (y != h1)
		{
			test(cell, myLabel, myDistance, bfaMatrix.width, x , y + 1, pq);

			if (isConnectivity8)
			{
				if (x != 0)
				{
					test(cell, myLabel, myDistance, bfaMatrix.width - 1, x - 1, y + 1, pq);
				}
				if (x != w1)
				{
					test(cell, myLabel, myDistance, bfaMatrix.width + 1, x + 1, y + 1, pq);
				}
			}
		}

		cell->label = myLabel & 0x7FFFFFFF;
	}

	bfaMatrix.resolvedLabels = bfaMatrix.nextLabel;
}
