#pragma once

#include <vector>
#include <iostream>



using namespace std;

int Odd(int k);
int ReturnNoOfBits(int n);
std::string toBinary(int n);
int toInteger(char* bstring);
vector<char> ToKBitBinary(int k, int n);
vector<char> ReturnFelicsCode(int L, int H, int number);
int DecodeFelic(int L, int H, vector<char> B, int& Findex);
vector<char> ToKBitBinaryInv(int k, int n);
int ReturnIntegerValue(vector<char> D, int position, int length);

int  SaveStringToFile(char* fn, vector<char> V);
void PrintString(vector<char> A, int d);
void PrintIntegers(vector<unsigned int> A, int d);

double CalculateEntropy(vector<char> abc, vector<char> Data);
double CalculateEntropy(vector<unsigned char> Vabc, vector<unsigned char> V);
double CalculateEntropy(vector<int> A);
void F4Entropy(vector<char> V, double& e, int& s);
void F8Entropy(vector<char> V, double& e, int& s);

int SumCC(vector<char> D);
int SumVCC(vector<char> D);


// Golomb
vector<char> TruncatedBinaryCode(int n, int x);
vector<char> UnaryCode(int n, char comma);
vector<char> Golomb(int m, int x);
vector<char> SignedGolomb(int m, int x);

int FromUnaryCode(vector<char> V, int pointer, char comma);
int FromGolomb(int m, int& ap, vector<char> s);
int FromSignedGolomb(int m, int& ap, vector<char> s);


int CompareV(vector<char> A, int pB, vector<char> B);

vector<char> ShiftCC(vector<char> A, int s);

int ToPositiveValue(int a);
int UnWrapePositiveValues(int a);

vector<int> DoMatrixStatistic(int Height, int Width, vector<vector<int>> M);
vector<int> DoVectorStatistic(vector<int> V);
