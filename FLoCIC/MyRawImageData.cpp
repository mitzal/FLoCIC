#include "MyRawImageData.h"
#include "AuxCompFunctions.h"

//_CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)


MyRawImageData::MyRawImageData()
{
	W = H = 0;
	InputImageData.clear();

}


void MyRawImageData::ClearInputImageData()
{

	for (int i = 0; i < (int)InputImageData.size(); i++)
		InputImageData[i].clear();
	InputImageData.clear();
}

void MyRawImageData::SetInputDataBW(int w, int h, vector<vector<int>> P)
{
	ClearInputImageData();

	InputImageData = P;
	W = w;
	H = h;
}

double MyRawImageData::ReturnEntropy()
{	
	return CalculateEntropy(DoMatrixStatistic(H, W, InputImageData));
}


void MyRawImageData::PixelMatrixToFile(const char* FName,  int d)
{
	FILE* f = fopen(FName, "wt");

	fprintf(f, "%*d", -4, W);
	fprintf(f, "%*d", -4, H);
	fprintf(f, "\n");

	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
			fprintf(f, "%*d ", -3, InputImageData[x][y]);
		fprintf(f, "\n");
	}

	fclose(f);

}
