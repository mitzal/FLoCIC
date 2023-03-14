#include "MyImgCompressBasic.h"
#include "AuxCompFunctions.h"
#include "Interpolative.h"
#include "ArithmeticCoding.hpp"

#pragma warning (disable : 4996)


MyImgCompressBasic::MyImgCompressBasic(MyRawImageData* r, SimplePredictors* s)
{
	SP = s;

	H = r->ReturnHeight();
	W = r->ReturnWidth();
	vector<vector<int>> IRD = r->ReturnImageData();
	
	I = ClearMatrix(I);
	I = CreateMatrix(W, H);

	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			I[x][y] = IRD[x][y];

}


vector<vector<int>>  MyImgCompressBasic::CreateMatrix(int W, int H)
{
	vector<vector<int>> M;
	M.resize(W);
	for (int i = 0; i < W; i++)
		M[i].resize(H);

	return M;
}

void MyImgCompressBasic::ClearInternal()
{
	PredictionArray.clear();
	PositivePredictionArray.clear();
	DePredictionArray.clear(); 
	DePositivePredictionArray.clear();
	BOut.clear();

	vector<unsigned char> ACByteOut;

}




vector<vector<int>> MyImgCompressBasic::ClearMatrix(vector<vector<int>> M)
{
	if (M.size() == 0)
		return M;
	for (int i = 0; i < (int)M.size(); i++)
		M[i].clear();

	M.clear();
	return M;
}


void MyImgCompressBasic::PredictionJPEGLS()
{
	PredictionArray.clear();

	int x, y;

	PredictionArray.push_back(I[0][0]);
	for (x = 1; x < W; x++)
		PredictionArray.push_back(I[x][0] - I[x - 1][0]);

	int prediction;
	for (y = 1; y < H; y++)
		for (x = 0; x < W; x++)
		{
			prediction = SP->GetPredictionJPEGLS(x, y);
			PredictionArray.push_back(I[x][y] - prediction);
		}
}


void MyImgCompressBasic::PredictionJPEGLS_A()
{
	PredictionArray.clear();

	int x, y;

	PredictionArray.push_back(I[0][0]);
	for (x = 1; x < W; x++)
		PredictionArray.push_back(I[x][0] - I[x - 1][0]);

	int prediction;
	for (y = 1; y < H; y++)
		for (x = 0; x < W; x++)
		{
			prediction = SP->GetPredictionJPEGLS_A(x, y);
			PredictionArray.push_back(I[x][y] - prediction);
		}
}


void MyImgCompressBasic::PredictionUpperBest()
{
	PredictionArray.clear();
	SP->SetNoOfSimplePredictors(MAX_PREDICTORS-4);
	int x, y;

	PredictionArray.push_back(I[0][0]);
	for (x = 1; x < W; x++)
		PredictionArray.push_back(I[x][0] - I[x - 1][0]);

	int prediction;
	for (y = 1; y < H; y++)
		for (x = 0; x < W; x++)
		{
			prediction = SP->GetPredictionUpperBest(x, y);
			PredictionArray.push_back(I[x][y] - prediction);
		}
		 
}

void MyImgCompressBasic::PredictionPreviousBest()
{
	PredictionArray.clear();
	SP->SetNoOfSimplePredictors(MAX_PREDICTORS-4);
//	SP->SetNoOfSimplePredictors(6);
	int x, y;

	PredictionArray.push_back(I[0][0]);
	for (x = 1; x < W; x++)
		PredictionArray.push_back(I[x][0] - I[x - 1][0]);

	int prediction;
	for (y = 1; y < H; y++)
		for (x = 0; x < W; x++)
		{
			prediction = SP->GetPredictionPrevious(x, y);
			PredictionArray.push_back(I[x][y] - prediction);
		}
}


void MyImgCompressBasic::CompressInterpolative(int CodingType)
{

	PositivePredictionArray.push_back(PredictionArray[0]);
	for (int i = 1; i < (int)PredictionArray.size(); i++)
		PositivePredictionArray.push_back(ToPositiveValue(PredictionArray[i]));


	Entropy = CalculateEntropy(DoVectorStatistic(PositivePredictionArray));

	CInterpolative IC(IC_CODING_FELICS);
	BOut = IC.Compress(PositivePredictionArray);
}

void MyImgCompressBasic::DecompressInterpolative(vector<char> B)
{
	CInterpolative IC(IC_CODING_FELICS);
	DePositivePredictionArray = IC.Decompress(B);

	for (int i = 0; i < DePositivePredictionArray.size(); i++)
	{
		if (PositivePredictionArray[i] != DePositivePredictionArray[i])
		{
			int stop = 0;
		}
			

	}


	if (PositivePredictionArray == DePositivePredictionArray)
		printf("\n\n Interpolative decoding successfull.\n");
	else
		printf("\n\n ---Interpolative decoding failed.\n");

	DePredictionArray.push_back(DePositivePredictionArray[0]);
	for (int i = 1; i < (int)DePositivePredictionArray.size(); i++)
		DePredictionArray.push_back(UnWrapePositiveValues(DePositivePredictionArray[i]));

	if (PredictionArray == DePredictionArray)
		printf("\n\n UnWrapping successfull.\n");
	else
		printf("\n\n ---UnWrapping failed.\n");

}

void MyImgCompressBasic::DePredictionJPEGLS()
{ 
	O = ClearMatrix(O);

	O = CreateMatrix(W, H);

	int PredictionIndex = 0;
	O[0][0] = DePredictionArray[PredictionIndex++];
	int x, y;
	for (x = 1; x < W; x++)
	{
		O[x][0] = O[x - 1][0] + DePredictionArray[PredictionIndex];
		PredictionIndex++;
	}

	int prediction;
	for (y = 1; y < H; y++)
		for (x = 0; x < W; x++)
		{
			prediction = SP->GetPredictionJPEGLSDe(O, x, y);
			O[x][y] = prediction + DePredictionArray[PredictionIndex];
			PredictionIndex++;
		}

}




void MyImgCompressBasic::CompressAC()
{
	PositivePredictionArray.clear();

	PositivePredictionArray.push_back(PredictionArray[0]);
	for (int i = 1; i < (int)PredictionArray.size(); i++)
		PositivePredictionArray.push_back(ToPositiveValue(PredictionArray[i]));

	Compression::ACe123<int> AC;
	ACByteOut = AC.Encode(PositivePredictionArray);
}


int MyImgCompressBasic::DecompressInterpolative(vector<unsigned char> I)
{
	Compression::ACe123<int> AC;
	vector<int> OPPA = AC.Decode(I);

	if (OPPA == PositivePredictionArray)
		return 1;
	return 0;
}