#pragma once

#include <vector>
#include "MyRawImageData.h"
#include "AuxCompFunctions.h"
#include "SimplePredictors.h"

using namespace std;



class MyImgCompressBasic
{
private:
	vector<vector<int>> I;
	vector<vector<int>> O;
	int W, H;
	double Entropy;
	SimplePredictors* SP;

	vector<int> PredictionArray , PositivePredictionArray, DePredictionArray, DePositivePredictionArray;
	vector<char> BOut;

	vector<unsigned char> ACByteOut;

	vector<vector<int>> ClearMatrix(vector<vector<int>>);
	vector<vector<int>>  CreateMatrix(int W, int H);

public:
	MyImgCompressBasic(MyRawImageData* r, SimplePredictors* s);

	void ClearInternal();

	void PredictionJPEGLS();
	void PredictionJPEGLS_A();
	void PredictionUpperBest();
	void PredictionPreviousBest();



	void CompressInterpolative(int t);
	vector<char> ReturnBitStream() { return BOut; }

	void DecompressInterpolative(vector<char> B);
	void DePredictionJPEGLS();
	vector<vector<int>> ReturnReconstructedMatrix() { return O; }

	void CompressAC();
	vector<unsigned char> ReturnACCompressedBytes() { return ACByteOut; 	}
	int DecompressInterpolative(vector<unsigned char> );

	double ReturnEntropy() { return Entropy; }
};

