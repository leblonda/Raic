#pragma once
#include <vector>

struct domain
{
	domain(int start, int stop) :start(start), stop(stop) {}
	~domain() {}
	int start;
	int stop;
};

class tiles
{
public:
	tiles(int imageWidth, int imageHeight, double tileWidth, double tileHeight, double seedWindowFraction, double reseedWindowFraction, bool isRaic) : _imageWidth(imageWidth), _imageHeight(imageHeight)
	{
		int nXTiles;
		int nYTiles;

		if (isRaic)
		{
			nXTiles = int(ceil(double(imageWidth) / tileWidth));
			nYTiles = int(ceil(double(imageHeight) / tileHeight));
		}
		else
		{
			nXTiles = int(floor(double(imageWidth) / tileWidth));
			nYTiles = int(floor(double(imageHeight) / tileHeight));
		}

		auto stepXDividors = double(imageWidth) / nXTiles;
		auto stepYDividors = double(imageHeight) / nYTiles;
		
		_xDividors.reserve(nXTiles+1);
		_yDividors.reserve(nYTiles+1);
		_xSeedDomains.reserve(nXTiles);
		_ySeedDomains.reserve(nYTiles);
		_xReseedDomains.reserve(nXTiles);
		_yReseedDomains.reserve(nYTiles);

		for (int i = 0; i < nXTiles; i++)
		{
			_xDividors.push_back(int(round(i*stepXDividors)));
		}
		_xDividors.push_back(imageWidth);

		for (int i = 0; i < nYTiles; i++)
		{
			_yDividors.push_back(int(round(i*stepYDividors)));
		}
		_yDividors.push_back(imageHeight);

		auto seedStartFraction = (1.0 - seedWindowFraction) / 2.0;
		auto seedStopFraction = 1.0 - seedStartFraction;
		
		auto reseedStartFraction = (1.0 - reseedWindowFraction) / 2.0;
		auto reseedStopFraction = 1.0 - reseedStartFraction;

		for (int i = 0; i < nXTiles; i++)
		{
			auto start = _xDividors[i];
			auto stop = _xDividors[i + 1];
			auto gap = double(stop - start);
			auto firstSeedStart = int(round(double(start) + seedStartFraction*gap));
			auto firstSeedStop = int(round(double(start) + seedStopFraction*gap));
			auto reseedStart = int(round(double(start) + reseedStartFraction*gap));
			auto reseedStop = int(round(double(start) + reseedStopFraction*gap));
			
			_xSeedDomains.push_back(domain(firstSeedStart, firstSeedStop));
			_xReseedDomains.push_back(domain(reseedStart, reseedStop));
		}

		for (int i = 0; i < nYTiles; i++)
		{
			auto start = _yDividors[i];
			auto stop = _yDividors[i + 1];
			auto gap = double(stop - start);
			auto firstSeedStart = int(round(double(start) + seedStartFraction*gap));
			auto firstSeedStop = int(round(double(start) + seedStopFraction*gap));
			auto reseedStart = int(round(double(start) + reseedStartFraction*gap));
			auto reseedStop = int(round(double(start) + reseedStopFraction*gap));
			
			_ySeedDomains.push_back(domain(firstSeedStart, firstSeedStop));
			_yReseedDomains.push_back(domain(reseedStart, reseedStop));
		}
	}
	~tiles() = default;
	inline const std::vector<int>& getXDividors() const { return _xDividors; }
	inline const std::vector<int>& getYDividors() const { return _yDividors; }
	inline const std::vector<domain>& getXSeedDomains() const { return _xSeedDomains; }
	inline const std::vector<domain>& getYSeedDomains() const { return _ySeedDomains; }
	inline const std::vector<domain>& getXReseedDomains() const { return _xReseedDomains; }
	inline const std::vector<domain>& getYReseedDomains() const { return _yReseedDomains; }
	inline int getNumberOfTiles() const {
		return int((_xDividors.size()-1)*(_yDividors.size()-1));
	}
	inline int getImageWidth() const { return _imageWidth; }
	inline int getImageHeight() const { return _imageHeight; }
private:
	std::vector<int> _xDividors;
	std::vector<int> _yDividors;
	std::vector<domain> _xSeedDomains;
	std::vector<domain> _ySeedDomains;
	std::vector<domain> _xReseedDomains;
	std::vector<domain> _yReseedDomains;
	int _imageWidth;
	int _imageHeight;
};