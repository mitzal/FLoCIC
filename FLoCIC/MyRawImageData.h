#pragma once
#include <vector>
using namespace std;


class MyRawImageData
{
private:
	vector<vector<int>> InputImageData;
	int W, H;

	void ClearInputImageData();

public:



	MyRawImageData();
	void SetInputDataBW(int w, int h, vector<vector<int>> P);
	int ReturnWidth() { return W; }
	int ReturnHeight() { return H; }
	vector<vector<int>>& ReturnImageData() { return InputImageData; }
	double ReturnEntropy();
	void PixelMatrixToFile(const char* FName, int d);
};

