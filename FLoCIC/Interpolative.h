#pragma once

#include<vector>
#include <string>

using namespace std;

#define IC_CODING_BINARY 0
#define IC_CODING_FELICS 1
#define IC_CODING_TRUNCATED_BINARY 2

class CInterpolative
{
private:

	int CodingFlag;  // 0 code with binar coding
	                 // 1 code with Felics coding
					 // 2 truncated binary
	char* T;         // array to convert it into number by function "toInteger()" 
					 

	vector<int> C;          // vector of cummulative values
	vector<char> B;			// compressed binary stream
	vector<char> BI;		// input binary stream
	vector<unsigned int> DCMP;		// decompressed data

	
	int BIPointer;			// pointer into the BI vector

	int InsertFelicsCode(int L, int H, int number);
	int DecodeFelic(int L, int H);

	void InsertTruncatedBinary(int L, int H, int num);
	int DecodeTruncatedBinary(int L, int R);

	void DoCodingInterpolative(int L, int R);
	void DoDecodingInterpolative(int L, int R);

	void InsertCode(int L, int H, int num);
	int ReadCode(int L, int H);


	string toBinary(int n);
	int ReturnNoOfBits(int n);

	void ToKBitsBinary(int k, int n);
	void ToKBitsBinaryInv(int k, int n);

	int ReturnIntegerValue(vector<char> D, int position, int length);
	int toInteger(char* bstring);



public:
	CInterpolative();
	CInterpolative(int cf);
	~CInterpolative(void);

	void SetCoding(int cf) { CodingFlag = cf; }
	vector<char> Compress(vector<int> V);
	vector<int> Decompress(vector<char> D);
};
