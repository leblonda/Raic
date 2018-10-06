#pragma once
class image;

class seedAffinityMatrix
{
public:
	seedAffinityMatrix();
	~seedAffinityMatrix();
	void Calculate(image& image);
	int width=0;
	int height=0;
	int nCells = 0;
	std::vector<double> data;
};

