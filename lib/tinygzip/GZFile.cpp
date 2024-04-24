#include "GZFile.h"
#include <iomanip>

GZFile::GZFile(const string& filename) : ifstream(filename) {
	if (!is_open())
		throw std::ios_base::failure(std::string("Failed to open file " + filename + ": ") + strerror(errno));
	readHeader();
}

void GZFile::open(const string& filename) {
	ifstream::open(filename, std::ios::binary);
	if (!is_open())
		throw std::ios_base::failure(std::string("Failed to open file " + filename + ": ") + strerror(errno));
}



void GZFile::readHeader() {
	uint8_t header[10];
	read((char *)header, 10);
	if (gcount() != 10)
		throw InvalidGzipFileFormat("Invalid gzip file header");

	if(header[0] != 0x1F || header[1] != 0x8B)
		throw InvalidGzipFileFormat("Not a gzip file");

	gz_compressionMethod = header[2];
	if(gz_compressionMethod != 0x08)
		throw InvalidGzipFileFormat("Invalid Compression method " + to_string(gz_compressionMethod));

	gz_flags = header[3];
	gz_modificationTime = *reinterpret_cast<uint32_t*>(header + 4);
	gz_extraFlags = header[8];
	gz_operatingSystem = header[9];

	if (getFlag(FEXTRA)) {
		char extraLength[2];
		read(extraLength, 2);
		int length = (extraLength[1] << 8) | extraLength[0];
		ignore(length);
	}
	if (getFlag(FNAME)) {
		// Skip original filename (it is null terminated)
		char c; read(&c, 1);
		while(c != '\0')
		{
			originalFilename.push_back(c);
			read(&c, 1);
		}
	}
	if (getFlag(FCOMMENT)) {
		// Skip null terminated comment
		ignore(256, '\0');
	}
	if (getFlag(FHCRC)) {
		// Skip the crc of header
		ignore(2);
	}
}

bool GZFile::getFlag(GZ_FLAG_BIT bit)
{
	return (gz_flags & (1 << bit)) != 0;
}

void GZFile::printHeader() {

    std::cout << "Compression Method: " << static_cast<int>(gz_compressionMethod) << std::endl;

    // Print flags using their names
    std::cout << "Flags:" << std::endl;
    if (getFlag(FTEXT)) std::cout << "  FTEXT" << std::endl;
    if (getFlag(FHCRC)) std::cout << "  FHCRC" << std::endl;
    if (getFlag(FEXTRA)) std::cout << "  FEXTRA" << std::endl;
    if (getFlag(FNAME)) std::cout << "  FNAMEL " << originalFilename << std::endl;
    if (getFlag(FCOMMENT)) std::cout << "  FCOMMENT" << std::endl;

    std::cout << "Modification Time: " << gz_modificationTime << std::endl;
    std::cout << "Extra Flags: " << static_cast<int>(gz_extraFlags) << std::endl;
    std::cout << "Operating System: " << static_cast<int>(gz_operatingSystem) << std::endl;
}
