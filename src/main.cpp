#include <iostream>
#include <bitset>
#include <tinygzip/tinygzip.h>
using namespace std;

int main()
{
	// GZFile file("test.txt.gz");
	Decompressor decomp("test.txt.gz", "uncompressed.txt");
	decomp.puff();
	
}