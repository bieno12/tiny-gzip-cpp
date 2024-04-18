#include "headers.h"


bool GZFile::open(const string& filename) {
	std::ifstream checkFile(filename);
	file.open(filename, std::ios::binary);
	if (!file.is_open())
		throw std::ios_base::failure(std::string("Failed to open file " + filename + ": ") + strerror(errno));
	return true;
}

int GZFile::read(char* buffer, int size) {
	file.read(buffer, size);
	return file.gcount();
}

void GZFile::close() {
	file.close();
}

GZFile::~GZFile() {
	if (file.is_open())
		close();
}
void GZFile::readHeader() {
	char header[10];
	file.read(header, 10);
	if (file.gcount() != 10 || header[0] != 0x1F || header[1] != 0x8B || header[2] != 8) {
		throw InvalidGzipFileFormat("Invalid gzip file header");
	}
	compressionMethod = header[2];
	if(compressionMethod != 0x08)
		throw InvalidGzipFileFormat("Invalid Compression method " + to_string(compressionMethod));

	flags = header[3];
	modificationTime = *reinterpret_cast<uint32_t*>(header + 4);
	extraFlags = header[8];
	operatingSystem = header[9];

	if (getFlag(FEXTRA)) {
		char extraLength[2];
		file.read(extraLength, 2);
		int length = (extraLength[1] << 8) | extraLength[0];
		file.ignore(length);
	}
	if (getFlag(FNAME)) {
		// Skip original filename (it is null terminated)
		file.ignore(256, '\0');
	}
	if (getFlag(FCOMMENT)) {
		// Skip null terminated comment
		file.ignore(256, '\0');
	}
	if (getFlag(FHCRC)) {
		// Skip the crc of header
		file.ignore(2);
	}
}

bool GZFile::getFlag(FLAG_BIT bit)
{
	return (flags & (1 << bit)) != 0;
}