#include "deflate.h"

int Decompressor::bits(int need)
{
	long val;
	val = this->bitbuffer;

	while (this->bitcount < need)
	{
		char c;
		c = in.get();
		if (c == EOF)
			throw runtime_error("Unexpected end of file");

		// load 8 bits
		val |= (long)(c) << this->bitcount;
	}
	this->bitbuffer = (int)(val >> need);
	this->bitcount -= need;
	return (int)(val & ((1L << need) - 1));
}

void Decompressor::stored()
{
	int len;

	// discard current bits
	this->bitbuffer = 0;
	this->bitcount = 0;

	char buf[4];
	in.read(buf, 4);
	if (in.gcount() != 0)
		throw runtime_error("Unexpected end of file, when reading len of stored block");
	if (buf[0] != ~buf[2] || buf[1] != ~buf[3])
		throw runtime_error("Len's compliment doesn't match, when processing stored block");
	
	char c;
	len = buf[1] << 8 | buf[0];
	for (int i = 0 ; i < len; i++)
	{
		c = in.get();
		if(c == EOF)
			throw runtime_error("Unexpected end of file, when reading store block");
		out.write(&c, 1);
	}
}


Huffman Decompressor::constructHuffman(vector<int> length, int n)
{
	Huffman h;
	h.length = length;
	int count[MAXBITS + 1];
	int next_code[MAXBITS + 1];
	 /* count number of codes of each length */
	for(int sym = 0; sym < n; sym++)
		count[length[sym]]++;

	/*	Find the numerical value of the smallest code for each
		code length:*/
	int code = 0;
	count[0] = 0;
	for (int bits = 1; bits <= MAXBITS; bits++)
	{
		code = (code + count[bits - 1]) << 1;
		next_code[bits] = code;
	}

	//assign numerical values to all codes
	for (int sym = 0; sym < n; sym++)
	{
		h.sym2code[sym] = next_code[length[sym]];
		h.code2sym[next_code[length[sym]]] = sym;
		next_code[length[sym]]++;
	}
	return h;
}

Decompressor::Decompressor(string inputFilename, string outputFilename) : in(inputFilename)
{
	out = ofstream(outputFilename, ios::binary);
	if (!out.is_open())
		throw std::ios_base::failure(std::string("Failed to open file " + outputFilename + ": ") + strerror(errno));
}