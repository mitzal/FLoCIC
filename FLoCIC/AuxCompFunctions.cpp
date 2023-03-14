#include <math.h>
#include <iostream>
#include <string>
#include "AuxCompFunctions.h"

#pragma warning (disable : 4996)


std::string toBinary(int n)
{
    std::string r;
    while(n!=0) {r=(n%2==0 ?"0":"1")+r; n/=2;}
    return r;
}


int ReturnNoOfBits(int n)
{
	return ( ((int)( log((double)(n)) / log(2.0))) + 1 );
}

int toInteger(char* bstring)
{

	char * end;
	long int value = strtol (bstring,&end,2);

	return (int)value;
}


int Odd(int k)
{
	if ( k % 2) return 1;
	return 0;
}

vector<char> ToKBitBinary(int k, int n)
{
	std::string s = toBinary(n);
	vector<char> V;
	V.resize(k-(int)s.size(), '0');

	for (int i = 0; i < (int)s.size(); i++)
		V.push_back(s[i]);

	return V;
}

vector<char> ToKBitBinaryInv(int k, int n)
{
	vector<char> V = ToKBitBinary(k, n);	

	for (int i = 0; i < (int)V.size(); i++)
	{
		if (V[i] == '0') 
			V[i] = '1';
		else
			V[i] = '0';
	}

	return V;
}


vector<char> ReturnFelicsCode(int L, int H, int number)
{

	vector<char> A;
	if (number < L) return A;
	if (number > H) return A;

	vector<char> FC;
	int d = H - L + 1;
	int k = (int)( log((double)(d)) / log(2.0));
	int a = (int)pow(2.0, k+1) - d;

	int b = 2 * (d - (int)pow(2.0, k) );

	int b2 = (int)b/2;

	number = number - L;
	if (number < b2 )
		return ToKBitBinary(k+1, number);
	if (number >= a + b2)
		return ToKBitBinary(k+1, number - a);

	return ToKBitBinaryInv(k, number - b2);
}

int DecodeFelic(int L, int H, vector<char> B, int& Findex)
{
	
	int d = H - L + 1;
	int k = (int)( log((double)(d)) / log(2.0));
	int a = (int)pow(2.0, k+1) - d;

	int b = 2 * (d - (int)pow(2.0, k) );
	int b2 = (int)b/2;

	int i, n;
	char* F = new char[k+2];
	int EndFlag = 0;

	for (i = 0; i < k+1; i++)
		if ( Findex < (int)B.size() )
		{
			F[i] = B[Findex++];
			F[i+1] = '\0';
		}
		else
			EndFlag = 1;



	n = toInteger(F);
	if (n < b)
	{
		n = toInteger(F);
		delete[] F;
		if (n < b2)
			return (n+L);
		return(L+n+a);
	}

	if (!EndFlag )
	{
		Findex--;
		F[k] = '\0';
	}

	for (i = 0; i < k; i++)  // invert bits
		if (F[i] == '0') 
			F[i] = '1';
		else
			F[i] = '0'; 

	n = toInteger(F);
	delete[] F;
	return (L+ b2 + n);
	
} 


int ReturnIntegerValue(vector<char> D, int position, int length)
{

	char T[100];
    
	int j=0;

	for (int i = position; i < position + length; i++)
		T[j++] = D[i];

	T[j]='\0';
	return toInteger(T);
}



int  SaveStringToFile(char* fn, vector<char> V)
{
	FILE* f = fopen(fn, "wt");

	if (f == NULL) return 0;

	for (unsigned int i = 0; i < V.size(); i++)
		fputc(V[i], f);

	fclose(f);
	return 1;


}

void PrintString(vector<char> A, int d)
{
	unsigned i = 0;

	while ( i < A.size() )
	{
		putchar(A[i]);
		i++;
		if ((i % d) == 0)
			putchar('\n');
	}
	putchar('\n');
}


void PrintIntegers(vector<unsigned int> A, int d)
{
	unsigned i = 0;

	while ( i < A.size() )
	{
		cout << A[i]<< "," ;
		i++;
		if ((i % d) == 0)
			cout << endl;
	}
	cout << endl;
}


double CalculateEntropy(vector<char> Vabc, vector<char> V)
{

	int nabc = (int)Vabc.size();
	int* A = new int[nabc];
	int i,j;
	for (i = 0; i < nabc; i++)
		A[i] = 0;

	int m = (int)V.size();
	for (i = 0; i < m; i++)
	{
		j = 0;
		
		while ((j < nabc) && (V[i] != Vabc[j]))
			j++;

		if (j < nabc)
			A[j]++;
	}


	double h = 0.0;
	double a, b;
	for (i = 0; i < nabc; i++)
	{
		if (A[i] != 0)
		{
			a = (double)A[i] / (double)m;
			b = log(a) / log(2.0);
			h = h + a * b;
		}
	}

	return -h;
}

double CalculateEntropy(vector<unsigned char> Vabc, vector<unsigned char> V)
{

	int nabc = (int)Vabc.size();
	int* A = new int[nabc];
	int i, j;
	for (i = 0; i < nabc; i++)
		A[i] = 0;

	int m = (int)V.size();
	for (i = 0; i < m; i++)
	{
		j = 0;

		while ((j < nabc) && (V[i] != Vabc[j]))
			j++;

		if (j < nabc)
			A[j]++;
	}


	double h = 0.0;
	double a, b;
	for (i = 0; i < nabc; i++)
	{
		if (A[i] != 0)
		{
			a = (double)A[i] / (double)m;
			b = log(a) / log(2.0);
			h = h + a * b;
		}
	}
	return -h;
}

double CalculateEntropy(vector<int> A)
{

	int m = (int)A.size();

	int sum = 0;
	for (int i = 0; i < m; i++)
		sum += A[i];


	double h = 0.0;
	double a, b;
	for (int i = 0; i < m; i++)
	{
		if (A[i] != 0)
		{
			a = (double)A[i] / (double)sum;
			b = log(a) / log(2.0);
			h = h + a * b;
		}
	}
	return -h;
}


//


int SumCC(vector<char> D)
{

	unsigned int i;
	int s = 0;
	for (i = 0; i < D.size(); i++)
	{
		s = s + D[i] -48 + 1;

	}
	return s;
}

int SumVCC(vector<char> D)
{

	unsigned int i;
	int s = 0;
	for (i = 0; i < D.size(); i++)
	{
		s = s + D[i] -48;

	}
	return s;
}



void F4Entropy(vector<char> V, double& e, int& s)
{
	vector<char> Q;
	Q.push_back('0'); Q.push_back('1'); Q.push_back('2'); Q.push_back('3');

	e = CalculateEntropy(Q, V);
	s = SumCC(V);
}

void F8Entropy(vector<char> V, double& e, int& s)
{
	vector<char> Q;
	Q.push_back('0'); Q.push_back('1'); Q.push_back('2'); Q.push_back('3');
	Q.push_back('4'); Q.push_back('5'); Q.push_back('6'); Q.push_back('7');

	e = CalculateEntropy(Q, V);
	s = SumCC(V);
}


// %%%%%%%%%%%%% Golomb

vector<char> TruncatedBinaryCode(int n, int x)
// n je velikost abecede zacensi pri 0
// x je element abecede
{
	int nb = ReturnNoOfBits(n);
	int u = (int)pow (2.0, nb);
	int unused = u - n;

	if (x < unused) // code with nb-1 bits
		return ToKBitBinary(nb-1, x);

	return ToKBitBinary(nb, x+unused);
}

vector<char> UnaryCode(int n, char comma)
{
	vector<char> U;
	int i;

	if (comma == '0')
	{
		for (i = 0; i < n; i++)
			U.push_back('1');
		U.push_back('0');
		return U;
	}

	for (i = 0; i < n; i++)
		U.push_back('0');
	U.push_back('1');
	return U;
}


int FromUnaryCode(vector<char> V, int pointer, char comma)
{

	int i = pointer;
	int straza = i;

	if (comma == '0')
	{
		while ((V[i] == '1') && (i < (int)V.size() ))
			i++;
		return i-straza;
	}
	while ((V[i] == '1') && (i < (int)V.size() ))
		i++;
	return i-straza;


}


vector<char> Golomb(int m, int x)
// m parameter
// x is coded nonnegative number
{

	int q = (x)/m;
	int r = x - q*m;

	vector<char> U = UnaryCode(q, '0');
	vector<char> R = TruncatedBinaryCode(m, r);
	U.insert(U.end(), R.begin(), R.end() );

	return U;
}

vector<char> SignedGolomb(int m, int x)
// m parameter
// x signed integer
{

	if (x == 0)
		return Golomb(m, x);

	if (x > 0)
	{
		x = 2*x-1;
		return Golomb(m, x);
	}
	
	x = -x;
	x = 2*x;
	return Golomb(m, x);
}


int FromGolomb(int m, int& ap, vector<char> s)
{
	int q = FromUnaryCode(s, ap, '0');

	ap = ap + q + 1;


	vector<char>* C = new vector<char>[m];
	for (int i = 0; i < m; i++)
		C[i] = TruncatedBinaryCode(m, i);

	int found = 0;

	int ii = 0;
	while (!found)
	{
		found = CompareV(C[ii], ap, s);
		if (!found)
			ii++;
	}

	ap = ap + (int)C[ii].size();

	int r = m*q + ii;
	return r;
}


int FromSignedGolomb(int m, int& ap, vector<char> s)
{
	int g = FromGolomb(m, ap, s);

	if (g == 0) return 0;

	if (Odd(g))
	{
		g = g >> 1;
		return g+1;
	}
	g = g >> 1;
	g = -g;
	return g;

}

////////////////////// end Golomb

int CompareV(vector<char> A, int pB, vector<char> B)
{
	
	int Thesame = 1;
	int j=0;

	while ((j < (int)A.size()) && Thesame)
	{
		if (A[j] == B[pB + j])
			j++;
		else
			Thesame = 0;
	}
	return Thesame;
}


vector<char> ShiftCC(vector<char> A, int s)
{
	for (unsigned int i = 0; i < A.size(); i++)
		A[i] = A[i] + s;

	return A;
}


int ToPositiveValue(int a)
{
	if (a >= 0)
		return (2 * a);
	else
		return (2 * abs(a) - 1);
}

int UnWrapePositiveValues(int a)
{
	int b;
	if (Odd(a)) // liho
	{
		b = a + 1;
		b = b / 2;
		b = b * (-1);
		return b;
	}
	else
		return a / 2;
}



vector<int> DoMatrixStatistic(int Height, int Width, vector<vector<int>> M)
{
	vector<int> Statistics;
	if (M.size() > 0)
	{
		int i, j;
		int max = 0;

		for (j = 0; j < Height; j++)
			for (i = 0; i < Width; i++)
				if (max < M[i][j])
					max = M[i][j];

		max++;

		Statistics.resize(max, 0);

		for (j = 0; j < Height; j++)
			for (i = 0; i < Width; i++)
				Statistics[M[i][j]]++;

	}
	return Statistics;
}

vector<int> DoVectorStatistic(vector<int> V)
{
	vector<int> Statistics;
	if (V.size() > 0)
	{
		int i;
		int max = 0;
		int min = 1000000;

		for (i = 0; i < V.size(); i++)
		{
			if (max < V[i])
				max = V[i];

			if (min > V[i])
				min = V[i];
		}

		if (min != 0)
			for (i = 0; i < V.size(); i++)
				V[i] = V[i] - min;

		int NoOfElements = max - min + 1;

		Statistics.resize(NoOfElements, 0);

		for (i = 0; i < V.size(); i++)
			Statistics[V[i]]++;

	}
	return Statistics;
}
