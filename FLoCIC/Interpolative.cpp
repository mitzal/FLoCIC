//#include "AuxCompFunctions.h"
#include "Interpolative.h"

#include <stdio.h>
#include <string.h>

#pragma warning (disable : 4996)


CInterpolative::CInterpolative() 
{ 
	BIPointer = 0;  
	CodingFlag = IC_CODING_BINARY; 
	T = NULL;
};

CInterpolative::CInterpolative(int cf) 
{ 
	BIPointer = 0; 
	CodingFlag = cf; 
	T = NULL;
}


CInterpolative::~CInterpolative(void)
{
	if (T != NULL)
	{
		delete[] T;
		T = NULL;
	}
}


vector<char> CInterpolative::Compress(vector<int> V)
{
	C.resize(V.size());
	C[0] = V[0];
	for (int i = 1; i < (int)V.size(); i++)
		C[i] = C[i - 1] + V[i];

	B.clear();
	B.resize(C.size() * 10);
	BIPointer = 0;
	ToKBitsBinary(32, (int)C.size());
	ToKBitsBinary(10, C[0]);
	ToKBitsBinary(32, C[C.size() - 1]);

	DoCodingInterpolative(0, (int)C.size() - 1 );
	B.resize(BIPointer);
	return B;
}

vector<int> CInterpolative::Decompress(vector<char> D)
{
	int n = ReturnIntegerValue(D, 0, 32);
	DCMP.clear();
	DCMP.resize(n);

	DCMP[0] = ReturnIntegerValue(D, 32, 10);
	DCMP[n-1] = ReturnIntegerValue(D, 42, 32);

	int nb = ReturnNoOfBits(DCMP[n - 1]);
	T = new char[nb + 2];

	BI = D;
	BIPointer = 74;
	DoDecodingInterpolative(0, n - 1);

	vector<int> R;
	R.resize(n);
	R[0] = DCMP[0];
	for (int i = 1; i < n; i++)
		R[i] = DCMP[i] - DCMP[i - 1];

	return R;
}


void CInterpolative::InsertCode(int L, int H, int num)
{
	H = H - L;
	num = num - L;
	int k = ReturnNoOfBits(H);
	ToKBitsBinary(k, num);
}


void CInterpolative::DoCodingInterpolative(int L, int R)
{


	if (R - L != 1)
	{

		if (C[R] != C[L])
		{
			int m = L + (int)((R - L) / 2);

			if (m == 220926)
				int qqq = 10;

			switch (CodingFlag)
			{
			case IC_CODING_BINARY: InsertCode(C[L], C[R], C[m]); break;
			case IC_CODING_FELICS: InsertFelicsCode(C[L], C[R], C[m]); break;
			case IC_CODING_TRUNCATED_BINARY: InsertTruncatedBinary(C[L], C[R], C[m]); break;
			}


			if (L < m)
				DoCodingInterpolative(L, m);

			if (m < R - 1)
				DoCodingInterpolative(m, R);
		}
	}
}


////////////////////////////////////////////////////////////////// decompress


int CInterpolative::ReadCode(int L, int H)
{
	H = H - L;
	int k = ReturnNoOfBits(H);

	char* T = new char[k+1];
	T[k] = '\0';

	int i;

	for (i = 0; i < k; i++)
		T[i] = BI[BIPointer++];

	int z = toInteger(T);
	delete[] T;

	return z+L;
}


void CInterpolative::DoDecodingInterpolative(int L, int R)
{
	if (R - L != 1)
	{

		if (DCMP[R] == DCMP[L])
		{
			for (int i = L + 1; i < R; i++)
				DCMP[i] = DCMP[L];

		}
		else
		{
			int m = L + (int)((R - L) / 2);

			if (m == 262142) 
				int qqq = 10;

			switch (CodingFlag)
			{
			case IC_CODING_BINARY: DCMP[m] = ReadCode(DCMP[L], DCMP[R]); break;
			case IC_CODING_FELICS: DCMP[m] = DecodeFelic(DCMP[L], DCMP[R]); break;
			case IC_CODING_TRUNCATED_BINARY: DCMP[m] = DecodeTruncatedBinary(DCMP[L], DCMP[R]); break;
			}


			if (L < m)
				DoDecodingInterpolative(L, m);

			if (m < R - 1)
				DoDecodingInterpolative(m, R);
		}
	}
}


// aux Functions


string CInterpolative::toBinary(int n)
{
	string r;
	while (n != 0) { r = (n % 2 == 0 ? "0" : "1") + r; n /= 2; }
	return r;
}


int CInterpolative::ReturnNoOfBits(int n)
{
	return (((int)(log((double)(n)) / log(2.0))) + 1);
}

void CInterpolative::ToKBitsBinary(int k, int n)
{
	string s = toBinary(n);
	int i;
	for (i = 0; i < (k - (int)s.size()); i++)
		B[BIPointer++]='0';

	for (i = 0; i < (int)s.size(); i++)
		B[BIPointer++] = s[i];
}


int CInterpolative::ReturnIntegerValue(vector<char> D, int position, int length)
{

	char T[100];

	int j = 0;

	for (int i = position; i < position + length; i++)
		T[j++] = D[i];

	T[j] = '\0';
	return toInteger(T);
}


int CInterpolative::toInteger(char* bstring)
{

	char* end;
	long int value = strtol(bstring, &end, 2);

	return (int)value;
}


int CInterpolative::InsertFelicsCode(int L, int H, int number)
{

	vector<char> A;
	if (number < L) return 0;
	if (number > H) return 0;

	vector<char> FC;
	int d = H - L + 1;
	int k = (int)(log((double)(d)) / log(2.0));
	int a = (int)pow(2.0, k + 1) - d;

	int b = 2 * (d - (int)pow(2.0, k));

	int b2 = (int)b / 2;

	number = number - L;
	if (number < b2)
		ToKBitsBinary(k + 1, number);
	else
		if (number >= a + b2)
			ToKBitsBinary(k + 1, number - a);
		else
			ToKBitsBinaryInv(k, number - b2);
	return 1;
}



void CInterpolative::ToKBitsBinaryInv(int k, int n)
{
	int BiPointerOld = BIPointer;
	ToKBitsBinary(k, n);

	for (int i = BiPointerOld; i <BIPointer; i++)
		if (B[i] == '0')
			B[i] = '1';
		else
			B[i] = '0';
}



int CInterpolative::DecodeFelic(int L, int H)
{

	int d = H - L + 1;
	int k = (int)(log((double)(d)) / log(2.0));
	int a = (int)pow(2.0, k + 1) - d;

	int b = 2 * (d - (int)pow(2.0, k));
	int b2 = (int)b / 2;

	int i, n;
	int EndFlag = 0;

	for (i = 0; i < k + 1; i++)
		if (BIPointer < (int)BI.size())
		{
			T[i] = BI[BIPointer++];
			T[i + 1] = '\0';
		}
		else
		{	
			//		at the end, it is a short code
			EndFlag = 1;
			goto Short;
		}



	n = toInteger(T);
	if (n < b)
	{
		if (n < b2)
			return (n + L);
		return(L + n + a);
	}

	if (!EndFlag)
	{
		BIPointer--;
		T[k] = '\0';
	}
Short:
	for (i = 0; i < k; i++)  // invert bits
		if (T[i] == '0')
			T[i] = '1';
		else
			T[i] = '0';

	n = toInteger(T);
	return (L + b2 + n);

}

void CInterpolative::InsertTruncatedBinary(int L, int H, int x)
{
	H -= L;
	x -= L;
	L = 0;

	int nb = ReturnNoOfBits(H);
	int u = (int)pow(2.0, nb)-1;
	int shortCodes = u - H;

	vector<char> V;
	if (x < shortCodes) // code with nb-1 bits
		ToKBitsBinary(nb - 1, x);
	else
		ToKBitsBinary(nb, x + shortCodes);
}

int CInterpolative::DecodeTruncatedBinary(int L, int H)
{
	int StoreL = L;
	H -= L;
	L = 0;

	int nb = ReturnNoOfBits(H);
	int u = (int)pow(2.0, nb) - 1;
	int shortCodes = u - H;
	int i;

	for (i = 0; i < nb-1; i++)
		T[i] = BI[BIPointer++];

	T[nb-1] = '\0';

	int r = toInteger(T);

	if (r >= shortCodes)
	{
		T[i] = BI[BIPointer++];
		T[nb] = '\0';
		r = toInteger(T);
		r -= shortCodes;
	}

	r = r + StoreL;
	return r;
}
