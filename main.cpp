// for stb
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <random>
#include <direct.h>
#include <vector>

#include "StatelessShuffle.h"
#include "StatelessGrouping.h"

#define DETERMINISTIC() true

uint32 GetSeed()
{
#if DETERMINISTIC()
	static std::mt19937 rng;
	std::uniform_int_distribution<uint32> dist(0);
	return dist(rng);
#else
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<uint32> dist(0);
	return dist(rng);
#endif
}

bool ShuffleTest()
{
	// shuffle 16 things 3 times
	{
		printf("Shuffling 16 items with 4 rounds\n");
		StatelessShuffle shuffleIterator;
		shuffleIterator.SetIndexBits(4);
		shuffleIterator.SetRoundCount(4);
		for (int testIndex = 0; testIndex < 4; ++testIndex)
		{
			uint32 seed = GetSeed();
			printf("  seed = %u\n  ", seed);
			shuffleIterator.SetSeed(seed);
			bool first = true;
			for (uint32 index = 0; index < 16; ++index)
			{
				uint32 shuffledIndex = shuffleIterator.IndexToShuffledIndex(index);
				uint32 unshuffledIndex = shuffleIterator.ShuffledIndexToIndex(shuffledIndex);
				printf("%s%u",(first ? "" : ", "), shuffledIndex);
				if (index != unshuffledIndex)
				{
					printf("Error! Round trip failure in " __FUNCTION__);
					return false;
				}
				first = false;
			}
			printf("\n\n");
		}
	}

	// Shuddle 12 things, 3 times
	{
		printf("Shuffling 12 items with 4 rounds\n");
		StatelessShuffle shuffleIterator;
		shuffleIterator.SetIndexBits(4);
		shuffleIterator.SetRoundCount(4);
		for (int testIndex = 0; testIndex < 4; ++testIndex)
		{
			uint32 seed = GetSeed();
			printf("  seed = %u\n  ", seed);
			shuffleIterator.SetSeed(seed);
			bool first = true;
			for (uint32 index = 0; index < 16; ++index)
			{
				uint32 shuffledIndex = shuffleIterator.IndexToShuffledIndex(index);

				// when shuffling less than a power of 2, ignore positions that are out of bounds
				if (shuffledIndex >= 12)
					continue;

				uint32 unshuffledIndex = shuffleIterator.ShuffledIndexToIndex(shuffledIndex);
				printf("%s%u", (first ? "" : ", "), shuffledIndex);
				if (index != unshuffledIndex)
				{
					printf("Error! Round trip failure in " __FUNCTION__);
					return false;
				}
				first = false;
			}
			printf("\n\n");
		}
	}

	return true;
}

bool FadeInTest()
{
	printf("Running Fade In Test...\n");

	int width, height, comp;
	unsigned char* pixels = stbi_load("logo.png", &width, &height, &comp, 1);
	if (!pixels)
	{
		printf("could not load image in " __FUNCTION__);
		return false;
	}

	uint32 pixelCount = width * height;
	uint32 indexBits = uint32(std::ceil(std::log2(float(pixelCount))));
	if (indexBits & 1)
		indexBits++;
	uint32 roundedUpPixelCount = 1 << indexBits;

	StatelessShuffle shuffleIterator;
	shuffleIterator.SetIndexBits(indexBits);
	shuffleIterator.SetRoundCount(4);
	shuffleIterator.SetSeed(GetSeed());

	static const uint32 c_numFramesSaved = 16;
	int lastFrameSaved = -1;

	std::vector<unsigned char> pixelsOut(pixelCount, 0);
	int pixelsWritten = 0;
	for (uint32 index = 0; index < roundedUpPixelCount; ++index)
	{
		uint32 shuffledIndex = shuffleIterator.IndexToShuffledIndex(index);
		if (shuffledIndex >= pixelCount)
			continue;

		pixelsOut[shuffledIndex] = pixels[shuffledIndex];
		pixelsWritten++;

		uint32 thisFrame = (c_numFramesSaved - 1) * pixelsWritten / (pixelCount - 1);
		if (thisFrame != lastFrameSaved)
		{
			lastFrameSaved = thisFrame;
			char fileName[256];
			sprintf(fileName, "out/FadeIn_%u.png", thisFrame);
			stbi_write_png(fileName, width, height, 1, pixelsOut.data(), 0);
		}
	}

	stbi_image_free(pixels);

	return true;
}

bool FadeInRGBTest()
{
	printf("Running Fade In RGB Test...\n");

	int width, height, comp;
	unsigned char* pixels = stbi_load("logo.png", &width, &height, &comp, 3);
	if (!pixels)
	{
		printf("could not load image in " __FUNCTION__);
		return false;
	}

	uint32 pixelCount = width * height;
	uint32 indexBits = uint32(std::ceil(std::log2(float(pixelCount))));
	if (indexBits & 1)
		indexBits++;
	uint32 roundedUpPixelCount = 1 << indexBits;

	StatelessGrouping shuffledGroupIterator;
	shuffledGroupIterator.SetIndexBits(indexBits);
	shuffledGroupIterator.SetRoundCount(4);
	shuffledGroupIterator.SetGroupSize(3);
	shuffledGroupIterator.SetSeed(GetSeed());

	static const uint32 c_numFramesSaved = 16;
	int lastFrameSaved = -1;

	std::vector<unsigned char> pixelsOut(pixelCount * 3, 0);
	std::vector<uint32> members;
	for (uint32 index = 0; index < roundedUpPixelCount; ++index)
	{
		shuffledGroupIterator.GetGroup(index, members);

		for (int i = 0; i < 3; ++i)
		{
			uint32 shuffledIndex = members[i];
			if (shuffledIndex < pixelCount)
				pixelsOut[shuffledIndex * 3 + i] = pixels[shuffledIndex * 3 + i];
		}

		uint32 thisFrame = (c_numFramesSaved - 1) * index / (roundedUpPixelCount - 1);
		if (thisFrame != lastFrameSaved)
		{
			lastFrameSaved = thisFrame;
			char fileName[256];
			sprintf(fileName, "out/FadeInRGB_%u.png", thisFrame);
			stbi_write_png(fileName, width, height, 3, pixelsOut.data(), 0);
		}
	}

	stbi_image_free(pixels);

	return true;
}

bool FadeInRGB2Test()
{
	printf("Running Fade In RGB 2 Test...\n");

	int width, height, comp;
	unsigned char* pixels = stbi_load("logo.png", &width, &height, &comp, 3);
	if (!pixels)
	{
		printf("could not load image in " __FUNCTION__);
		return false;
	}

	uint32 pixelCount = width * height;
	uint32 indexBits = uint32(std::ceil(std::log2(float(pixelCount))));
	if (indexBits & 1)
		indexBits++;
	uint32 roundedUpPixelCount = 1 << indexBits;

	StatelessGrouping shuffledGroupIterator;
	shuffledGroupIterator.SetIndexBits(indexBits);
	shuffledGroupIterator.SetRoundCount(4);
	shuffledGroupIterator.SetGroupSize(4);
	shuffledGroupIterator.SetSeed(GetSeed());

	static const uint32 c_numFramesSaved = 16;
	int lastFrameSaved = -1;

	std::vector<unsigned char> pixelsOut(pixelCount * 3, 0);
	std::vector<uint32> members;
	for (uint32 index = 0; index < roundedUpPixelCount; ++index)
	{
		shuffledGroupIterator.GetGroup(index, members);

		for (int i = 0; i < 3; ++i)
		{
			uint32 shuffledIndex = members[i + 1];
			if (shuffledIndex < pixelCount)
				pixelsOut[shuffledIndex * 3 + i] = pixels[shuffledIndex * 3 + i];
		}

		uint32 thisFrame = (c_numFramesSaved - 1) * index / (roundedUpPixelCount - 1);
		if (thisFrame != lastFrameSaved)
		{
			lastFrameSaved = thisFrame;
			char fileName[256];
			sprintf(fileName, "out/FadeInRGB2_%u.png", thisFrame);
			stbi_write_png(fileName, width, height, 3, pixelsOut.data(), 0);
		}
	}

	stbi_image_free(pixels);

	return true;
}

int main(int argc, char** argv)
{
	_mkdir("out");

	if (!ShuffleTest())
		return 1;

	if (!FadeInTest())
		return 1;

	if (!FadeInRGBTest())
		return 1;

	if (!FadeInRGB2Test())
		return 1;

	return 0;
}
