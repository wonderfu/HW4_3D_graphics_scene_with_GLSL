#include "noise.h"

double noise[NoiseWidth][NoiseHeight][NoiseDepth];

void generateNoise()
{
	for (int x = 0; x < NoiseWidth; ++x)
	{
		for (int y = 0; y < NoiseHeight; ++y)
		{
			for (int z = 0; z < NoiseDepth; ++z)
			{
				noise[x][y][z] = (rand() % 32768) / 32768.0;
			}
		}
	}
}

double smoothNoise(double x, double y, double z)
{
	//get fractional part of x and y
	double fractX = x - int(x);
	double fractY = y - int(y);
	double fractZ = z - int(z);

	//wrap around
	int x1 = (int(x) + NoiseWidth) % NoiseWidth;
	int y1 = (int(y) + NoiseHeight) % NoiseHeight;
	int z1 = (int(z) + NoiseDepth) % NoiseDepth;

	//neighbor values
	int x2 = (x1 + NoiseWidth - 1) % NoiseWidth;
	int y2 = (y1 + NoiseHeight - 1) % NoiseHeight;
	int z2 = (z1 + NoiseDepth - 1) % NoiseDepth;

	//smooth the noise with bilinear interpolation
	double value = 0.0;
	value += fractX       * fractY       * fractZ       * noise[x1][y1][z1];
	value += fractX       * (1 - fractY) * fractZ       * noise[x1][y2][z1];
	value += (1 - fractX) * fractY       * fractZ       * noise[x2][y1][z1];
	value += (1 - fractX) * (1 - fractY) * fractZ       * noise[x2][y2][z1];

	value += fractX       * fractY       * (1 - fractZ) * noise[x1][y1][z2];
	value += fractX       * (1 - fractY) * (1 - fractZ) * noise[x1][y2][z2];
	value += (1 - fractX) * fractY       * (1 - fractZ) * noise[x2][y1][z2];
	value += (1 - fractX) * (1 - fractY) * (1 - fractZ) * noise[x2][y2][z2];

	return value;
}

double turbulence(double x, double y, double z, double size)
{
	double value = 0.0, initialSize = size;

	while (size >= 1)
	{
		value += smoothNoise(x / size, y / size, z / size) * size;
		size /= 2.0;
	}

	return(128.0 * value / initialSize);
}