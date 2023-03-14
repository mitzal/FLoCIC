#include "SimplePredictors.h"
#include "AuxCompFunctions.h"


SimplePredictors::SimplePredictors(MyRawImageData* I)
{
	NoOfPredictors = 0;
	W = I->ReturnWidth();
	H = I->ReturnHeight();
	ImageData = I->ReturnImageData();
	Predictors.clear();
	PreviousBestIndex = 1;
}

int SimplePredictors::SetNoOfSimplePredictors(int n)
{ 
	Predictors.clear();
	NoOfPredictors = n;
	if (NoOfPredictors > MAX_PREDICTORS)
		NoOfPredictors = MAX_PREDICTORS;

	Predictors.resize(W);
	return 1;
}

int SimplePredictors::GetPredictionJPEGLS(int x, int y)
{
	if (x == 0)
		return ImageData[x][y - 1];

	int a = ImageData[x][y - 1];
	int c = ImageData[x - 1][y - 1];
	int b = ImageData[x - 1][y];

	if ((c > a) && (c > b))
		if (a < b)
			return a;
		else
			return b;

	if ((c < a) && (c < b))
		if (a > b)
			return a;
		else
			return b;

	int d = a + b - c;
	return d;
}


int SimplePredictors::GetPredictionJPEGLS_A(int x, int y)
{
	if (x == 0)
		return ImageData[x][y - 1];

	int a = ImageData[x][y - 1];
	int c = ImageData[x - 1][y - 1];
	int b = ImageData[x - 1][y];
	int d;

	if ((c > a) && (c > b))
		if (a < b)
			d = a + 1;
		else
			d = b + 1;
	else
		if ((c < a) && (c < b))
			if (a > b)
				d =  a - 1;
			else
				d =  b - 1;
		else
			d = a + b - c;


	if (d < 0)
		d = 0;

	if (d > 255)
		d = 255;

	return d;

}



int SimplePredictors::GetPredictionJPEGLSDe(vector<vector<int>>& M, int x, int y)
{
	if (x == 0)
		return M[x][y - 1];

	int a = M[x][y - 1];
	int c = M[x - 1][y - 1];
	int b = M[x - 1][y];

	if ((c > a) && (c > b))
		if (a < b)
			return a;
		else
			return b;

	if ((c < a) && (c < b))
		if (a > b)
			return a;
		else
			return b;

	int d = a + b - c;
	return d;

}



int SimplePredictors::CalculatePredictor(int x, int y, int PredictorIndex)
{
	if (PredictorIndex >= NoOfPredictors)
		return -1;

	if (ImageData.size()  == 0)
		return -1;

	int a, b, c, d;

	if (x == 0)
	{
		a = c = b = ImageData[x][y - 1];
		d = ImageData[x + 1][y - 1];
	}
	else
		if (x == W - 1)
		{
			a = ImageData[x - 1][y];
			d = b = ImageData[x][y - 1];
			c = ImageData[x - 1][y - 1];
		}
		else
		{
			a = ImageData[x - 1][y];
			b = ImageData[x][y - 1];
			c = ImageData[x - 1][y - 1];
			d = ImageData[x + 1][y - 1];
		}


	switch (PredictorIndex)
	{
	case 0: return a;
	case 1: return b;
	case 2: return c;
	case 3: return (a + b - c);
	case 4: return d;
	case 5: return ((int)((a + c) / 2));
	case 6: return ((int)((c + b) / 2));
	case 7: return ((int)((b + d) / 2));
	case 8: return ((int)((a + d) / 2));
	case 9: return ((int)((c + d) / 2));
	case 10: return ((int)(b + c - d));
	case 11: return ((int)(c + b - a));
	case 12: return ((int)(a + c - b));
	}
	return 0;
}

int SimplePredictors::GetPredictionUpperBest(int x, int y)
{
	int min = 1000;
	int p, v;

	if (y == 1)
		p = GetPredictionJPEGLS(x, y);
	else
		p = CalculatePredictor(x, y, Predictors[x]);

	for (int i = 0; i < NoOfPredictors; i++)
	{
		v = CalculatePredictor(x, y, i);
		v = ImageData[x][y] - v;
		v = ToPositiveValue(v);
		if (v < min)
		{
			if (i == 12)
				int qq = 0;
			min = v;
			Predictors[x] = i;
		}
	}

	return p;
}


int SimplePredictors::GetPredictionPrevious(int x, int y)
{

	int min = 1000;
	int p, v;

	if (y == 1)
		p = GetPredictionJPEGLS(x, y);
	else
		p = CalculatePredictor(x, y, PreviousBestIndex);

	for (int i = 0; i < NoOfPredictors; i++)
	{
		v = CalculatePredictor(x, y, i);
		v = ImageData[x][y] - v;
		v = ToPositiveValue(v);
		if (v < min)
		{
			min = v;
			PreviousBestIndex = i;
		}
	}

	return p;

}