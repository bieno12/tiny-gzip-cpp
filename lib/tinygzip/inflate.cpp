#include "deflate.h"
#include <bitset>
int Decompressor::bits(int need)
{
	long val;

	val = this->bitbuffer;
	string repr = bitset<8>(bitbuffer).to_string();
	while (this->bitcount < need)
	{
		char c;
		c = in.get();
		// if (c == EOF)
		// {
		// 	if(in.eof())
		// 		throw runtime_error("EOF flag is set");
		// 	throw runtime_error("Unexpected end of file");
		// }

		// load 8 bits
		val |= (unsigned long)((unsigned char)c) << this->bitcount;
		this->bitcount += 8;
	}
	this->bitbuffer = (int)(val >> need);
	this->bitcount -= need;
	int result = (int)(val & ((1L << need) - 1));
	return result;
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
		writeOut(&c, 1);
	}
}


Huffman Decompressor::constructHuffman(vector<int> length)
{
	Huffman h;
	int n = length.size();
	int count[MAXBITS + 1];
	int next_code[MAXBITS + 1];

	memset(count, 0, (MAXBITS + 1) * sizeof (int));
	memset(next_code, 0, (MAXBITS + 1) * sizeof (int));

	h.length = length;

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
		if(length[sym] == 0) continue;
		auto code = make_pair(length[sym], next_code[length[sym]]);
		h.sym2code[sym] = code;
		h.code2sym[code] = sym;
		next_code[length[sym]]++;
	}
	return h;
}

void Decompressor::printHuffman(Huffman h)
{
	for(auto &[codepair, symbol] : h.code2sym)
	{
		auto &[len, codevalue] = codepair;
		cout << "(" << len << ", " << bitset<9>(codevalue) << " = " << symbol << endl;
	}
}

int Decompressor::decodeNext(Huffman &h)
{
	int codevalue = 0;
	for (int nbits = 1; nbits <= MAXBITS; nbits++)
	{
		codevalue |= bits(1);
		auto code = make_pair(nbits, codevalue);
		if(h.code2sym.find(code) != h.code2sym.end())
			return h.code2sym[code];

		codevalue <<= 1;
	}
	return -1;
}

int Decompressor::codes(Huffman& lencode, Huffman& distcode)
{
	int symbol;
	int len;
	unsigned dist;

	static const short lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short lext[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const short dext[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};
	
	// cout << "len code" << endl;
	// printHuffman(lencode);
	
	// cout << "dist code" << endl;
	// printHuffman(distcode);
	do {

		symbol = decodeNext(lencode);
		if (symbol < 0)
			throw runtime_error("Invalid symbol " + to_string(symbol));
		if (symbol <= 255)
		{
			//write letiral
			char c = (char)symbol;
			cout << "letiral symbol: " << c << "(" << int(c) << ")" << endl;
			writeOut(&c, 1);
			cout << "buff: \"" << outputBuffer << "\"" << endl;
		}
		else if (symbol > 256)
		{
			//then is  a length code
			symbol -= 257;
			if (symbol >= 29)
                throw runtime_error("Invalid fixed code");
			len = lens[symbol] + bits(lext[symbol]);
			//get distance code
			symbol = decodeNext(distcode);
			if(symbol < 0)
				throw runtime_error("invalid distance symbol");
			dist = dists[symbol] + bits(dext[symbol]);
			cout << "length = "<< len << ", distance = " << dist << endl;
			//write len bytes for output buffer
			writeOut(outputBuffer.c_str() + outputBuffer.size() - dist, len);
		}


	} while (symbol != 256);
	return 0;
}

int Decompressor::writeOut(const char *src, int n)
{
	for (int i = 0; i < n; i++)
		outputBuffer.push_back(src[i]);

	out.write(src, n);
	return n;
}

int Decompressor::fixed()
{
	static int virgin = 1;
    static Huffman lencode, distcode;

    /* build fixed huffman tables if first call (may not be thread safe) */
    if (virgin) {
        int symbol;
        vector<int> lengths;

        /* construct lencode and distcode */
        /* literal/length table */
        for (symbol = 0; symbol < 144; symbol++)
            lengths.push_back(8);
        for (; symbol < 256; symbol++)
            lengths.push_back(9);
        for (; symbol < 280; symbol++)
            lengths.push_back(7);
        for (; symbol < FIXLCODES; symbol++)
            lengths.push_back(8);
        lencode = constructHuffman(lengths);

        /* distance table */
		lengths.resize(0);
        for (symbol = 0; symbol < MAXDCODES; symbol++)
            lengths.push_back(5);
		distcode = constructHuffman(lengths);
        /* do this just once */
        virgin = 0;
    }

    /* decode data until end-of-block code */
    return codes(lencode,distcode);
}

int Decompressor::dynamic()
{
	static const short order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
	int nlen, ndist, ncode;
	int index;
	
	Huffman lencode, distcode;

	nlen = bits(5) + 257;
	ndist = bits(5) + 1;
	ncode = bits(4) + 4;

	if (nlen > MAXLCODES || ndist > MAXDCODES)
		throw runtime_error("bad counts of len or dist codes");

	vector<int> lengths(19);
	for(index = 0; index < ncode; index++)
		lengths[order[index]]= bits(3);
	
	Huffman tableh = constructHuffman(lengths);


	//decode the table using tableh;
	lengths.clear();
	lengths.resize(nlen + ndist);
	index = 0;
	while(index < nlen + ndist)
	{
		int symbol, len;
		
		symbol = decodeNext(tableh);
		if (symbol < 0)
			throw runtime_error("invalid symbol when decoding huffman tables");
		if (symbol < 16)
		{
			lengths[index++] = symbol;
		}
		else
		{
			len = 0;
			if (symbol == 16)
			{
				len = lengths[index - 1];
				symbol = 3 + bits(2);
			}
			else if (symbol == 17)
			{
				symbol = 3 +  bits(3);
			}
			else if (symbol == 18)
			{
				symbol = 11 + bits(7);
			}
			while(symbol--)
				lengths[index++] = len;				
		}
	} 

	if (lengths[256] == 0)
        throw runtime_error("there better be an end-of block symbol");
	lencode = constructHuffman(vector<int>(lengths.begin(), lengths.begin() + nlen));
	distcode = constructHuffman(vector<int>(lengths.begin() + nlen, lengths.end()));

	return codes(lencode, distcode);
}

int Decompressor::puff()
{
	int last, type;

	do
	{
		last = bits(1);
		type = bits(2);
		cout << "last :" << bool(last) << ", type: " << type << endl;
		if (type == 0)
			stored();
		else if (type == 1)
			fixed();
		else if (type == 2)
			dynamic();
	} while (!last);

	return 0;
}

Decompressor::Decompressor(string inputFilename, string outputFilename) : in(inputFilename)
{
	out = ofstream(outputFilename, ios::binary);
	outputBuffer.reserve(MAXDISTANCE);
	if (!out.is_open())
		throw std::ios_base::failure(std::string("Failed to open file " + outputFilename + ": ") + strerror(errno));
	in.printHeader();
}