#include "IOManager.h"
#include <fstream>
#include <string>

namespace UpiEngine{

	bool IOManager::readFileToBuffer(std::string filePath, std::vector<unsigned char> &buffer)
	{
		std::ifstream file(filePath, std::ios::binary);
		if (file.fail()) {
			perror(filePath.c_str());
			return false;
		}

		//seek to the end
		file.seekg(0, std::ios::end);

		//get filesize

		int fileSize = file.tellg();

		//seekkaa takaisin alkuuun
		file.seekg(0, std::ios::beg);

		//reduce filesize any header bytes
		fileSize -= file.tellg();

		buffer.resize(fileSize);
		file.read((char *)&(buffer[0]), fileSize);
		file.close();

		return true;
	}

	bool IOManager::readFileToBuffer(std::string filePath, std::string& buffer)
	{
		std::ifstream file(filePath, std::ios::binary);
		if (file.fail()) {
			perror(filePath.c_str());
			return false;
		}

		//seek to the end
		file.seekg(0, std::ios::end);

		//get filesize

		int fileSize = file.tellg();

		//seekkaa takaisin alkuuun
		file.seekg(0, std::ios::beg);

		//reduce filesize any header bytes
		fileSize -= file.tellg();

		buffer.resize(fileSize);
		file.read((char *)&(buffer[0]), fileSize);
		file.close();

		return true;
	}

}