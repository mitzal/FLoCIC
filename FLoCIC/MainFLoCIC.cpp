#include <stdio.h>
#include <string.h>
#include <vector>
#include <stddef.h>
#include <iostream>
#include <filesystem>

#include "CImg.h"
#include "MyRawImageData.h"
#include "MyImgCompressBasic.h"
#include "SimplePredictors.h"

#pragma warning (disable : 4996)

using namespace std;
using namespace cimg_library;

#define NO_OF_IMAGES 24

#define PATH_TO_IMAGES ".\\RasterImages\\"

char ImageName[NO_OF_IMAGES][128];
MyRawImageData MRID;

int LoadBMP(const char* BMPFileName)
{
	CImg<unsigned char> image;

	cimg::exception_mode(0);
	try {
		image.load(BMPFileName);
	}
	catch (CImgException& e) {

		printf("CImg Library Error: %s", e.what());
		return 0;
	}

	int width = image.width();
	int height = image.height();

	int x, y;
	vector<vector<int>> PInfo; 
	PInfo.resize(width);
	for (x = 0; x < width; x++)
		PInfo[x].resize(height);

	
	for (x = 0; x < width; x++)
		for (y = 0; y < height; y++)
		{
			PInfo[x][y] = (int)image(x, y, 0, 0);
		}

	MRID.SetInputDataBW(width, height, PInfo);

	return 1;
}


void ChangeFNameSuffix(const char* name, const char* suf, char *Fname)
{
	Fname[0] = '\0';
	int l = (int)strlen(name);
	strncat(Fname, name, l - 3);
	strcat(Fname, suf);

}


char* GetPathAndName(char* S, const char* name)
{
	S[0] = '\0';
	strcpy(S, PATH_TO_IMAGES);
	strcat(S, name);
	return S;
}


int SelectPrediction()
{
	printf("0...Prediction JPEGLS like\n");
	printf("1...Prediction Upper best\n");
	printf("2...Prediction Left best\n");

	int PIndex=0;
	scanf("%d", &PIndex);
	return PIndex;
}


int SelectImage()
{
	printf("  0...Baboon\n");
	printf("  1...Balloon\n");
	printf("  2...Barbara\n");
	printf("  3...Barb2\n");
	printf("  4...Board\n");
	printf("  5...Boats\n");
	printf("  6...Cameraman\n");
	printf("  7...Flower\n");
	printf("  8...Fruits\n");
	printf("  9...Girl\n");
	printf(" 10...Gold\n");
	printf(" 11...Hotel\n");
	printf(" 12...Lena\n");
	printf(" 13...Malamute\n");
	printf(" 14...Man\n");
	printf(" 15...Monarch\n");
	printf(" 16...Mushrooms\n");
	printf(" 17...Parrots\n");
	printf(" 18...Pens\n");
	printf(" 19...Peppers\n");
	printf(" 20...Rainier\n");
	printf(" 21...Sun\n");
	printf(" 22...Yacht\n");
	printf(" 23...Zelda\n");

	printf("99...Exit\n");

	int ImageIndex;
	scanf("%d", &ImageIndex);

	for (int i = 0; i < NO_OF_IMAGES; i++)
		ImageName[i][0] = '\0';


	

	char  S[128];
	strcpy(ImageName[0], GetPathAndName(S, "Baboon.bmp"));
	strcpy(ImageName[1], GetPathAndName(S, "Balloon.bmp"));
	strcpy(ImageName[2], GetPathAndName(S, "Barbara.bmp"));
	strcpy(ImageName[3], GetPathAndName(S, "Barb2.bmp"));
	strcpy(ImageName[4], GetPathAndName(S, "BOARD.bmp"));
	strcpy(ImageName[5], GetPathAndName(S, "boats.bmp"));
	strcpy(ImageName[6], GetPathAndName(S, "cameraman.bmp"));
	strcpy(ImageName[7], GetPathAndName(S, "flower.bmp"));
	strcpy(ImageName[8], GetPathAndName(S, "fruits.bmp"));
	strcpy(ImageName[9], GetPathAndName(S, "GIRL.bmp"));
	strcpy(ImageName[10], GetPathAndName(S, "GOLD.bmp"));
	strcpy(ImageName[11], GetPathAndName(S, "HOTEL.bmp"));
	strcpy(ImageName[12], GetPathAndName(S, "Lena.bmp"));
	strcpy(ImageName[13], GetPathAndName(S, "Malamute.bmp"));
	strcpy(ImageName[14], GetPathAndName(S, "Man.bmp"));
	strcpy(ImageName[15], GetPathAndName(S, "Monarch.bmp"));
	strcpy(ImageName[16], GetPathAndName(S, "Mushrooms.bmp"));
	strcpy(ImageName[17], GetPathAndName(S, "Parrots.bmp"));
	strcpy(ImageName[18], GetPathAndName(S, "pens.bmp"));
	strcpy(ImageName[19], GetPathAndName(S, "pepper.bmp"));
	strcpy(ImageName[20], GetPathAndName(S, "Rainier.bmp"));
	strcpy(ImageName[21], GetPathAndName(S, "Sun.bmp"));
	strcpy(ImageName[22], GetPathAndName(S, "yacht.bmp"));
	strcpy(ImageName[23], GetPathAndName(S, "ZELDA.bmp"));


	return ImageIndex;
}


void PrintImageInfo(int ind,  double entropy)
{
	printf("\nImage info %s\n", ImageName[ind]);
	printf("Resolution %d x %d\n", MRID.ReturnHeight(), MRID.ReturnWidth());

	int ISize = MRID.ReturnHeight() * MRID.ReturnWidth() * 8;
	printf("Image size in bits %d and bytes %d\n", ISize, ISize / 8);
	printf("Entropy of original input is %f\n", entropy);
}

void PrintEntropyInfo(const char* S, double entropy)
{
	printf("\n%s \n", S);
	printf("Entropy %f \n", entropy);
}

void PrintCompressionInfo(const char* S, int BascSize, double CR, double SpaceSaving)
{
	printf("%s: no of bits %d and bytes %d \n", S, BascSize, BascSize / 8);
	printf("CR: %f, Space saving: %f\n", CR, SpaceSaving);
}


int CompareMatrices(int W, int H, vector<vector<int>> A, vector<vector<int>> B)
{
	int x, y;
	int ok = 1;
	for (x = 0; x < W; x++)
		for (y = 0; y < H; y++)
			if (A[x][y] != B[x][y])
			{
				ok = 0;
				int stop = 0;
			}

	return ok;
}


void PrintImageData(int ImageIndex)
{
	char FName[256];
	char temp[256]; 
	temp[0] = '\0';
	strcpy(temp, ImageName[ImageIndex]);
	int L = strlen(temp);
	int i = L - 4;
	while (temp[i] != '\\')
		i--;
	int k = 0;
	for (int j = i+1; j < L - 4; j++)
		FName[k++] = temp[j];
	FName[k++] = '.';
	FName[k++] = 'p';
	FName[k++] = 'd';
	FName[k++] = 't';
	FName[k++] = '\0';

	MRID.PixelMatrixToFile(FName, 3);
}



int main()
{


	int ImageIndex = SelectImage();

	if (ImageIndex == 99)
		return 1;

	if (LoadBMP(ImageName[ImageIndex]))
	{

		int PredictionType = SelectPrediction();

		switch (ImageIndex)
		{
		case 99: return 0;
		default:

			int ImageSize = MRID.ReturnHeight() * MRID.ReturnWidth() * 8;

			PrintImageInfo(ImageIndex, MRID.ReturnEntropy());
//			PrintImageData(ImageIndex);

			char Fname[128];
			ChangeFNameSuffix(ImageName[ImageIndex], "TXT", &Fname[0]);

			SimplePredictors sp(&MRID);
			MyImgCompressBasic CB(&MRID, &sp);


			int ric, rac;
			ric = rac = 0;
			switch (PredictionType)
			{
			case 0: {
				CB.PredictionJPEGLS();

				printf("\n\n Interpolative Started\n\n");
				CB.CompressInterpolative(1);
				printf("\n\n Entropy: %f \n\n", CB.ReturnEntropy());

				vector<char> R = CB.ReturnBitStream();
				ric = ((int)R.size() / 8);

				CB.DecompressInterpolative(R);
				CB.DePredictionJPEGLS();
				vector<vector<int>> RImage = CB.ReturnReconstructedMatrix();

				if (RImage == MRID.ReturnImageData())
					printf("Interpolative Decompression Succesfull.\n");
				else
					printf("---Interpolative Decompression failed!!!\n");


				printf("\n\n Arithmetic coding started \n\n");
				CB.CompressAC();
				vector<unsigned char> RAC = CB.ReturnACCompressedBytes();
				rac = RAC.size();
				int ACOKFlag = CB.DecompressInterpolative(RAC);

				if (ACOKFlag)
					printf("Arithmetic Decoding Succesfull. \n");
				else
					printf("---Arithmetic Decoding Succesfull. \n");

				printf("\n-------------------------------\n");

				break;
			}
			case 1: {
				CB.PredictionUpperBest();

				printf("\n\n Interpolative Started\n\n");
				CB.CompressInterpolative(1);

				printf("\n\n Entropy: %f \n\n", CB.ReturnEntropy());

				vector<char> R = CB.ReturnBitStream();
				ric = ((int)R.size() / 8);

				printf("\n\n Arithmetic coding started \n\n");
				CB.CompressAC();
				vector<unsigned char> RAC = CB.ReturnACCompressedBytes();
				rac = RAC.size();
				int ACOKFlag = CB.DecompressInterpolative(RAC);

/*				if (ACOKFlag)
					printf("Arithmetic Decoding Succesfull. \n");
				else
					printf("---Arithmetic Decoding Succesfull. \n");
					*/
				printf("\n-------------------------------\n");
				break;
			}
			case 2: {
				CB.PredictionPreviousBest();

				printf("\n\n Interpolative Started\n\n");
				CB.CompressInterpolative(1);
				printf("\n\n Entropy: %f \n\n", CB.ReturnEntropy());

				vector<char> R = CB.ReturnBitStream();
				ric = ((int)R.size() / 8);

				printf("\n\n Arithmetic coding started \n\n");
				CB.CompressAC();
				vector<unsigned char> RAC = CB.ReturnACCompressedBytes();
				rac = RAC.size();
				break;
			}

			}

			printf("\n\n Interpolative %d \n", ric);
			printf("\n\n CR %f \n", (float)(MRID.ReturnHeight() * MRID.ReturnWidth())/ (float)ric);

			printf("\n\n Arithmetic coding %d \n", rac);
			printf("\n\n CR %f \n", (float)(MRID.ReturnHeight() * MRID.ReturnWidth()) / (float)rac);

			printf("\n");

		}
	}
	else
		printf("\n\n Image not found! \n\n");
	printf("Hit ... \n");
	

	return 0;
}
