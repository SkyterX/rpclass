#pragma once

#include <graph/io/IReader.hpp>

#include <cstdio>
#include <iostream>
#include <cstring>

namespace graphIO {
	class FileReader : public IReader {
	private:
		static const int BUFFER_SIZE = 10000000;
		static const int CRITICAL_CAPACITY = 100;
		FILE* input;
		char* buffer;
		char* currentPosition;

	public:
		FileReader() {
			input = nullptr;
			buffer = new char[BUFFER_SIZE + 1];
			memset(buffer, 0, BUFFER_SIZE + 1);
		}

		FileReader(FILE* input) {
			this->input = input;
			memset(buffer, 0, BUFFER_SIZE + 1);
		}

		virtual ~FileReader() {
			Close();
			delete[] buffer;
			currentPosition = nullptr;
		}

		void Open(const char* fileName) {
			input = fopen(fileName, "rt");

			if (input == nullptr) {
				std::cerr << "Error: can't open input file." << std::endl;
				exit(0);
			}

			fread(buffer, sizeof(char), BUFFER_SIZE, input);
			currentPosition = buffer;
		}

		void Close() {
			fclose(input);
		}

		char NextChar() override {
			EnsureCapacity();
			SkipWhitespaces();
			if (!HasNext()) return 0;
			return *(currentPosition++);
		}

		unsigned int NextUnsignedInt() override {
			EnsureCapacity();
			SkipWhitespaces();
			if (!HasNext()) return 0;

			int result = 0;
			while (*currentPosition > ' ')
				result = result * 10 + (*(currentPosition++) & 15);
			return result;
		}

		std::string ReadLine() override {
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

		bool HasNext() override {
			return *currentPosition != (char)0 || feof(input) == 0;
		}

	private:
		void EnsureCapacity() {
			int residualSize = buffer + BUFFER_SIZE - currentPosition;
			if (residualSize < CRITICAL_CAPACITY) {
				memcpy(buffer, currentPosition, residualSize);
				int readCount = fread(buffer + residualSize, 1, BUFFER_SIZE - residualSize, input);
				if (readCount < BUFFER_SIZE - residualSize)
					buffer[residualSize + readCount] = (char)0;
				currentPosition = buffer;
			}
		}

		void SkipWhitespaces() {
			while (*currentPosition <= ' ' && HasNext())
				++currentPosition;
		}
	};
}
