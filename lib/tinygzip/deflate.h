#ifndef DEFLATE_H
#define DEFLATE_H
#include "GZFile.h"
#include <vector>
#include <map>

#define MAXBITS 15              /* maximum bits in a code */
#define MAXLCODES 286           /* maximum number of literal/length codes */
#define MAXDCODES 30            /* maximum number of distance codes */
#define MAXCODES (MAXLCODES+MAXDCODES)  /* maximum codes lengths to read */
#define FIXLCODES 288           /* number of fixed literal/length codes */

#define MAXDISTANCE (32 * 1024)

// for simplicity use vector<int>  to represent a huffman tree
//where huffman[<len, code>]  = symbol;
struct Huffman
{
	map<int, pair<int, int>> sym2code;
	map<pair<int, int>, int> code2sym;
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
	string outputBuffer;
	int bitbuffer;
	int bitcount;

public:
	//reads need number of bits from bit buffer and in stream (if needed)
	int bits(int need);

	// process a stored block
	void stored();
	Huffman constructHuffman(vector<int> length);

	void printHuffman(Huffman h);
	int decodeNext(Huffman& h);
	
	int codes(Huffman& lencode, Huffman& distcode);

	int writeOut(const char *src, int n);

	int fixed();

	int dynamic();

	int puff();
public:
	Decompressor(string inputFilename, string outputFilename);
};
#endif