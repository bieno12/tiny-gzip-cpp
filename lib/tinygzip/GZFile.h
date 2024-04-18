#ifndef GZFILE_H
#define GZFILE_H

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstring>
using namespace std;
class InvalidGzipFileFormat : public std::runtime_error {
public:
    InvalidGzipFileFormat(const std::string& message)
        : std::runtime_error(message) {}
};

class GZFile {
private:
	enum FLAG_BIT {
		FTEXT,
		FHCRC,
		FEXTRA,
		FNAME,
		FCOMMENT,
	};
    std::ifstream file; // Input file stream
    bool validHeader;   // Flag to indicate if the header is valid
    uint8_t compressionMethod; // Compression method
	uint8_t flags;
    uint32_t modificationTime; // Modification time
    uint8_t extraFlags;        // Extra flags
    uint8_t operatingSystem;   // Operating system
public:
    GZFile(string filename);
    bool open(const string& filename);
    int read(char* buffer, int size);
    void close();
    ~GZFile();

	void printHeader();
private:
    void readHeader();
	bool getFlag(FLAG_BIT bit);

};

#endif