#ifndef DEFLATE_H
#define DEFLATE_H
#include "GZFile.h"
#include <vector>
#include <map>
#define MAXBITS 15

// for simplicity use vector<int>  to represent a huffman tree
//where huffman[code]  = symbol;
struct Huffman
{
	map<int, int> sym2code;
	map<int, int> code2sym;
	vector<int> length;
};

class Decompressor
{
private:
	//input compressed stream
	string inputFilename;
	GZFile in;

	//output uncompressed(inflated) stream
	string outputFilename;
	ofstream out;
	//
	int bitbuffer;
	int bitcount;


	//reads need number of bits from bit buffer and in stream (if needed)
	int bits(int need);

	// process a stored block
	void stored();
	Huffman constructHuffman(vector<int> length, int n);
	
public:
	Decompressor(string inputFilename, string outputFilename);



};
#endif