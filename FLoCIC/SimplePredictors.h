#pragma once
#include "MyRawImageData.h"

#include <vector>

#define MAX_PREDICTORS 13


class SimplePredictors
{
private:
    int NoOfPredictors;
    int W, H;
    vector<int> Predictors;
    vector<vector<int>> ImageData;

    int PreviousBestIndex;
public:
    SimplePredictors(MyRawImageData* I);

    int SetNoOfSimplePredictors(int n);
    int CalculatePredictor(int x, int y, int PredictorIndex);
    int GetPredictionJPEGLS(int x, int y);
    int GetPredictionJPEGLSDe(vector<vector<int>>& M, int x, int y);

    int GetPredictionJPEGLS_A(int x, int y);
    int GetPredictionUpperBest(int x, int y);
    int GetPredictionPrevious(int x, int y);

};

