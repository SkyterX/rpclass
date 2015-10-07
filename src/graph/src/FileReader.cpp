#define _CRT_SECURE_NO_WARNINGS

#include "graph/FileReader.hpp"

#include <cstdio>
#include <iostream>

FileReader::FileReader() {
	input = nullptr;
	buffer = new char[BUFFER_SIZE + 1];
	memset(buffer, 0, BUFFER_SIZE + 1);
}

FileReader::FileReader(FILE* input) {
	this->input = input;
	memset(buffer, 0, BUFFER_SIZE + 1);
}

FileReader::~FileReader() {
	Close();
	delete[] buffer;
	currentPosition = nullptr;
}

void FileReader::Open(const char* fileName) {
	input = fopen(fileName, "rt");

	if (input == nullptr) {
		std::cerr << "Error: can't open input file." << std::endl;
		exit(0);
	}

	fread(buffer, sizeof(char), BUFFER_SIZE, input);
	currentPosition = buffer;
}

void FileReader::Close() {
	fclose(input);
}

void FileReader::EnsureCapacity() {
	int residualSize = buffer + BUFFER_SIZE - currentPosition;
	if (residualSize < CRITICAL_CAPACITY) {
		memcpy(buffer, currentPosition, residualSize);
		int readCount = fread(buffer + residualSize, 1, BUFFER_SIZE - residualSize, input);
		if (readCount < BUFFER_SIZE - residualSize)
			buffer[residualSize + readCount] = (char)0;
		currentPosition = buffer;
	}
}


void FileReader::SkipWhitespaces() {
	while (*currentPosition <= ' ' && HasNext())
		++currentPosition;
}

bool FileReader::HasNext() {
	return *currentPosition != (char)0 || feof(input) == 0;
}

char FileReader::NextChar() {
	EnsureCapacity();
	SkipWhitespaces();
	if (!HasNext()) return 0;
	return *(currentPosition++);
}

unsigned int FileReader::NextUnsignedInt() {
	EnsureCapacity();
	SkipWhitespaces();
	if (!HasNext()) return 0;

	int result = 0;
	while (*currentPosition > ' ')
		result = result * 10 + (*(currentPosition++) & 15);
	return result;
}

std::string FileReader::ReadLine() {
	std::string result = "";

	EnsureCapacity();
	while (*currentPosition != '\r' && *currentPosition != '\n' && HasNext()) {
		result += *currentPosition;
		++currentPosition;
	}

	if (HasNext()) {
		if (*currentPosition == '\r')
			++currentPosition;
		if (*currentPosition == '\n')
			++currentPosition;
	}
	return result;
}
