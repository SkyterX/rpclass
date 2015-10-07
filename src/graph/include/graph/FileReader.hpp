#pragma once

#include "graph/IReader.hpp"

class FileReader : public IReader {
public:
	FileReader();
	FileReader(FILE* input);
	virtual ~FileReader();

	void Open(const char* fileName);
	void Close();
	char NextChar() override;
	unsigned int NextUnsignedInt() override;
	std::string ReadLine() override;
	bool HasNext() override;

private:
	void EnsureCapacity();
	void SkipWhitespaces();

	FILE* input;
	static const int BUFFER_SIZE = 10000000;
	static const int CRITICAL_CAPACITY = 100;
	char* buffer;
	char* currentPosition;
};
