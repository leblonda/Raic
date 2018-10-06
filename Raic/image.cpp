#include "stdafx.h"
#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
	inline void rgbtolab(double &sR, double &sG, double &sB)
	{
		const double epsilon = 0.008856;
		const double kappa = 903.3;
		const double Xr = 0.950456;
		const double Yr = 1.0;
		const double Zr = 1.088754;

		auto R = sR / 255.0;
		auto G = sG / 255.0;
		auto B = sB / 255.0;
		double r, g, b;
		double X, Y, Z;
		double xr, yr, zr;
		double fx, fy, fz;

		if (R <= 0.04045)	r = R / 12.92;
		else				r = pow((R + 0.055) / 1.055, 2.4);
		if (G <= 0.04045)	g = G / 12.92;
		else				g = pow((G + 0.055) / 1.055, 2.4);
		if (B <= 0.04045)	b = B / 12.92;
		else				b = pow((B + 0.055) / 1.055, 2.4);

		X = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
		Y = r * 0.2126729 + g * 0.7151522 + b * 0.0721750;
		Z = r * 0.0193339 + g * 0.1191920 + b * 0.9503041;

		xr = X / Xr;
		yr = Y / Yr;
		zr = Z / Zr;

		if (xr > epsilon)	fx = pow(xr, 1.0 / 3.0);
		else				fx = (kappa*xr + 16.0) / 116.0;
		if (yr > epsilon)	fy = pow(yr, 1.0 / 3.0);
		else				fy = (kappa*yr + 16.0) / 116.0;
		if (zr > epsilon)	fz = pow(zr, 1.0 / 3.0);
		else				fz = (kappa*zr + 16.0) / 116.0;

		sR = 116.0*fy - 16.0;
		sG = 500.0*(fx - fy);
		sB = 200.0*(fy - fz);
	}
}

image::image(std::string filename, bool convertToLAB)
{
	int texChannels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &width, &height, &texChannels, STBI_rgb);
	data = std::vector<rgba>(width*height, rgba());
	auto ptr = pixels;
	for (int i = 0; i<width*height; i++)
	{
		data[i].r = double(*(ptr++));
		data[i].g = double(*(ptr++));
		data[i].b = double(*(ptr++));
		if(convertToLAB)
			rgbtolab(data[i].r, data[i].g, data[i].b);
	}
	stbi_image_free(pixels);
}
