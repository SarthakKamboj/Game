#include "io.h"
#include <fstream>
#include <iostream>

namespace io {
	std::string get_file_contents(const char* path) {
		std::ifstream file(path, std::ios::binary);
		file.seekg(0, file.end);
		int size = file.tellg();
		file.seekg(0, file.beg);
		std::ios_base::iostate state = file.rdstate();
		if (state != std::ios_base::goodbit) {
			throw std::runtime_error("error opening file: " + std::string(path));
		}
		while (!file.eof()) {
			char* data = new char[(int)size + 1];
			memset(data, 0, size + 1);
			file.read(data, size);
			std::string fileStr(data);
			delete[] data;
			file.close();
			return fileStr;
		}
		return std::string();
	}
}