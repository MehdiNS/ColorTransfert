#include <iostream>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <chrono>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;
using namespace std::chrono;
using namespace glm;

struct imageStats_t
{
	float lMean;
	float aMean;
	float bMean;
	float lStdDev;
	float aStdDev;
	float bStdDev;
};

void lab2rgb(float* lab, unsigned char* rgb, unsigned int size);
void rgb2lab(unsigned char* rgb, float* lab, unsigned int size);
imageStats_t computeMeanAndStandardDeviation(float* lab, unsigned int size);
inline float clamp(float x, float a, float b) { return std::max(a, std::min(b, x)); }
inline float saturate(float x) { return clamp(x, 0.f, 1.f); }

int main(int argc, char *argv[])
{
	string sourceFilename = argv[1];
	string targetFilename = argv[2];
	string outputFilename = argv[3];
	
	auto t_start = std::chrono::high_resolution_clock::now();

	//open source image
	int xSource, ySource, nSource;
	int force_channels = 4;
	unsigned char* sourceData = stbi_load(sourceFilename.c_str(), &xSource, &ySource, &nSource, force_channels);
	if (!sourceData)
	{
		fprintf(stderr, "ERROR: could not load %s\n", sourceFilename.c_str());
		return 0;
	}
	unsigned int sizeSource = xSource * ySource * 4;
	float* sourceDataLab = new float[sizeSource] {0.f};

	//open target image
	int xTarget, yTarget, nTarget;
	unsigned char* targetData = stbi_load(targetFilename.c_str(), &xTarget, &yTarget, &nTarget, force_channels);
	if (!targetData)
	{
		fprintf(stderr, "ERROR: could not load %s\n", targetFilename.c_str());
		return 0;
	}
	unsigned int sizeTarget = xTarget * yTarget * 4;
	float* targetDataLab = new float[sizeTarget] {0.f};

	auto t_end = std::chrono::high_resolution_clock::now();
	//cout << "Images loading time : " << std::chrono::duration<float, std::milli>(t_end - t_start).count() << "ms" << endl;

	t_start = std::chrono::high_resolution_clock::now();

	//Convert source and target to Lab
	rgb2lab(sourceData, sourceDataLab, sizeSource);
	rgb2lab(targetData, targetDataLab, sizeTarget);

	//Compute mean and standard deviation by channel
	auto sStats = computeMeanAndStandardDeviation(sourceDataLab, sizeSource);
	auto tStats = computeMeanAndStandardDeviation(targetDataLab, sizeTarget);
	
	float lRatio = (tStats.lStdDev / sStats.lStdDev);
	float aRatio = (tStats.aStdDev / sStats.aStdDev);
	float bRatio = (tStats.bStdDev / sStats.bStdDev);
	
	//Color transform
	for (unsigned int i = 0; i < sizeSource; i = i + 4)
	{
		sourceDataLab[i]     = (sourceDataLab[i]	 - sStats.lMean) * lRatio + tStats.lMean;
		sourceDataLab[i + 1] = (sourceDataLab[i + 1] - sStats.aMean) * aRatio + tStats.aMean;
		sourceDataLab[i + 2] = (sourceDataLab[i + 2] - sStats.bMean) * bRatio + tStats.bMean;
	}

	//Back to RGB space
	lab2rgb(sourceDataLab, sourceData, sizeSource);

	t_end = std::chrono::high_resolution_clock::now();
	//cout << "Images processing time : " << std::chrono::duration<float, std::milli>(t_end - t_start).count() << "ms" << endl;

	//Write the final image
	t_start = std::chrono::high_resolution_clock::now();
	if (!stbi_write_png(outputFilename.c_str(), xSource, ySource, 4, sourceData, xSource * 4))
	{
		fprintf(stderr, "ERROR: could not write screenshot file \n");
	}
	t_end = std::chrono::high_resolution_clock::now();

	//cout << "Final image writing time : " << std::chrono::duration<float, std::milli>(t_end - t_start).count() << "ms" << endl;
}

imageStats_t computeMeanAndStandardDeviation(float* lab, unsigned int size)
{
	float lCount, aCount, bCount;
	lCount = aCount = bCount = 0.;

	for (unsigned int i = 0; i < size; i = i + 4)
	{
		lCount += lab[i];
		aCount += lab[i + 1];
		bCount += lab[i + 2];
	}
	float lMean = lCount / size;
	float aMean = aCount / size;
	float bMean = bCount / size;

	lCount = aCount = bCount = 0.f;
	for (unsigned int i = 0; i < size; i = i + 4)
	{
		lCount += pow(lab[i] - lMean, 2);
		aCount += pow(lab[i + 1] - aMean, 2);
		bCount += pow(lab[i + 2] - bMean, 2);
	}
	float lStdDev = sqrt(lCount / size);
	float aStdDev = sqrt(aCount / size);
	float bStdDev = sqrt(bCount / size);

	return{ lMean, aMean, bMean, lStdDev, aStdDev, bStdDev };
}

void lab2rgb(float* lab, unsigned char* rgb, unsigned int size)
{
	float temp1[9] = { 1, 1, 1, 1, 1, -1, 1, -2, 0 };
	float temp2[9] = { sqrt(3.f) / 3.f, 0, 0, 0, sqrt(6.f) / 6.f, 0 , 0, 0, sqrt(2.f) / 2.f };
	mat3 m1_lab2rgb = transpose(make_mat3(temp1));
	mat3 m2_lab2rgb = transpose(make_mat3(temp2));

	float aLMS2RGB[9] = { 4.4679f, -3.5873f, 0.1193f ,-1.2186f, +2.3809f, -0.1624f, 0.0497f, -0.2439f, 1.2045f };
	mat3 mLMS2RGB = transpose(make_mat3(aLMS2RGB));

	for (unsigned int i = 0; i < size; i = i + 4)
	{
		float l = lab[i];
		float a = lab[i + 1];
		float b = lab[i + 2];
		vec3 lab(l, a, b);

		//Convert to LMS
		vec3 LMS = m1_lab2rgb * m2_lab2rgb * lab;
		LMS = pow(vec3(10), LMS);
		
		//Convert to RGB
		auto rgbv = mLMS2RGB * LMS;
		rgb[i]	   = static_cast<unsigned char>(255.*saturate(rgbv.x));
		rgb[i + 1] = static_cast<unsigned char>(255.*saturate(rgbv.y));
		rgb[i + 2] = static_cast<unsigned char>(255.*saturate(rgbv.z));
	}
}

void rgb2lab(unsigned char* rgb, float* lab, unsigned int size)
{
	float temp1[9] = { 1.f / sqrt(3.f) , 0, 0, 0, 1.f / sqrt(6.f), 0 , 0, 0, 1.f / sqrt(2.f) };
	float temp2[9] = { 1, 1, 1, 1, 1, -2, 1, -1, 0 };

	mat3 m1_rgb2lab = transpose(make_mat3(temp1));
	mat3 m2_rgb2lab = transpose(make_mat3(temp2));

	float aRGB2LMS[9] = { 0.3811f, 0.5783f, 0.0402f, 0.1967f, 0.7244f, 0.0782f,0.0241f, 0.1288f, 0.8444f };
	mat3 mRGB2LMS = transpose(make_mat3(aRGB2LMS));

	for (unsigned int i = 0; i < size; i = i + 4)
	{
		// log10(0) = -inf, so gotta use FLT_TRUE_MIN
		// otherwise, image stats are fucked up
		float r = std::max(FLT_TRUE_MIN, rgb[i] / 255.f);
		float g = std::max(FLT_TRUE_MIN, rgb[i + 1] / 255.f);
		float b = std::max(FLT_TRUE_MIN, rgb[i + 2] / 255.f);
		vec3 rgb(r, g, b);

		//Convert to LMS
		vec3 LMS = mRGB2LMS * rgb;
		LMS = vec3(log10(LMS.x), log10(LMS.y), log10(LMS.z));

		//Convert to lab
		vec3 labv = m1_rgb2lab * m2_rgb2lab  * LMS;

		lab[i] = labv.x;
		lab[i + 1] = labv.y;
		lab[i + 2] = labv.z;
	}
}
