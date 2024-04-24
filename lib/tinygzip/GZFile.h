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

class GZFile : public ifstream {
private:
	enum GZ_FLAG_BIT {
		FTEXT,
		FHCRC,
		FEXTRA,
		FNAME,
		FCOMMENT,
	};
    bool gz_validHeader;   // Flag to indicate if the header is valid
    uint8_t gz_compressionMethod; // Compression method
	uint8_t gz_flags;
    uint32_t gz_modificationTime; // Modification time
    uint8_t gz_extraFlags;        // Extra flags
    uint8_t gz_operatingSystem;   // Operating system

	string originalFilename;
public:
    GZFile(const string& filename);
    void open(const string& filename);

	void printHeader();
private:
    void readHeader();
	bool getFlag(GZ_FLAG_BIT bit);
};

#endif