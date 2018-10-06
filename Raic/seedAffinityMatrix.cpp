#include "stdafx.h"
#include "seedAffinityMatrix.h"
#include "image.h"
#include <ppl.h>

namespace
{
	inline double calculateDistance2(const rgba& a, const rgba& b)
	{
		double dr = double(a.r) - double(b.r);
		double dg = double(a.g) - double(b.g);
		double db = double(a.b) - double(b.b);
		return dr * dr + dg * dg + db * db;
	}
}

seedAffinityMatrix::seedAffinityMatrix()
{
}

seedAffinityMatrix::~seedAffinityMatrix()
{
}

void seedAffinityMatrix::Calculate(image& image)
{
	width = image.width;
	height = image.height;
	nCells = width*height;
	data = std::vector<double>(nCells, 0);
	
	int blockSize = 1024;
	Concurrency::parallel_for(0, (nCells + blockSize-1) / blockSize , [&](int block)
	{
		int start= block*blockSize;
		int end= start+blockSize;
		if (end > nCells)
			end = nCells;
		for(int i=start;i<end;i++)
		{
			int x = i%width;
			int y = i/width;

			double axis0;
			if (x == 0 || x == width - 1)
			{
				axis0 = std::numeric_limits<double>::max();
			}
			else
			{
				axis0 = calculateDistance2(image.data[i], image.data[i-1]) + calculateDistance2(image.data[i], image.data[i + 1]) + 1;
			}

			double axis1;
			if (y == 0 || y == height - 1)
			{
				axis1 = std::numeric_limits<double>::max();
			}
			else
			{
				axis1 = calculateDistance2(image.data[i], image.data[i - width]) + calculateDistance2(image.data[i], image.data[i + width]) + 1;
			}

			double axis2;
			double axis3;
			if (x == 0 || x == width - 1 || y == 0 || y == height - 1)
			{
				axis2 = std::numeric_limits<double>::max();
				axis3 = std::numeric_limits<double>::max();
			}
			else
			{
				axis2 = calculateDistance2(image.data[i], image.data[i - width - 1]) + calculateDistance2(image.data[i], image.data[i + width + 1]) + 1;
				axis3 = calculateDistance2(image.data[i], image.data[i + width - 1]) + calculateDistance2(image.data[i], image.data[i - width + 1]) + 1;
			}

			data[i] = 1 / axis0 + 1 / axis1 + 1 / axis2 + 1 / axis3;
		}
	});
}
