#ifndef _NOISE_H
#define _NOISE_H

#include <cstdlib>
#include <ctime>

#define NoiseWidth 128
#define NoiseHeight 128
#define NoiseDepth 128

void generateNoise();
double smoothNoise(double, double, double);
double turbulence(double, double, double, double);

#endif