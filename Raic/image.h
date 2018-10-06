#pragma once

struct rgba
{
	double r;
	double g;
	double b;
	double a;
};

class image
{
public:
	image(std::string filename, bool convertToLAB);
	int width = 0;
	int height = 0;
	int count() const { return data.size(); }
	std::vector<rgba> data;
};

