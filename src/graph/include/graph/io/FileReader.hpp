#pragma once

#include <graph/io/IReader.hpp>

#include <cstdio>
#include <iostream>
#include <cstring>

namespace graphIO
{
	class FileReader : public IReader {
	private:
		static const int BUFFER_SIZE = 10000000;
		static const int CRITICAL_CAPACITY = 100;
		FILE* input;
		char* buffer;
		char* currentPosition;
		bool isClosed;

	public:
		FileReader() {
			input = nullptr;
			buffer = new char[BUFFER_SIZE + 1];
			memset(buffer, 0, BUFFER_SIZE + 1);
			isClosed = true;
		}

		FileReader(FILE* input) {
			isClosed = false;
			this->input = input;
			memset(buffer, 0, BUFFER_SIZE + 1);
		}

		virtual ~FileReader() {
			Close();
			delete[] buffer;
			currentPosition = nullptr;
		}

		bool Open(const char* fileName) {
			input = fopen(fileName, "rt");

			if (input == nullptr)
				return false;
			
			isClosed = false;
			fread(buffer, sizeof(char), BUFFER_SIZE, input);
			currentPosition = buffer;
			return true;
		}

		void Close() {
			if(!isClosed) {
				isClosed = true;
				fclose(input);
			}
		}

		char NextChar() override {
			EnsureCapacity();
			SkipWhitespaces();
			if (IsEof()) return 0;
			return *(currentPosition++);
		}

		unsigned int NextUnsignedInt() override {
			EnsureCapacity();
			SkipWhitespaces();
			if (IsEof()) return 0;

			int result = 0;
			while (*currentPosition > ' ')
				result = result * 10 + (*(currentPosition++) & 15);
			return result;
		}

		std::string ReadLine() override {
			std::string result = "";

			EnsureCapacity();
			while (*currentPosition != '\r' && *currentPosition != '\n' && !IsEof()) {
				result += *currentPosition;
				++currentPosition;
			}

			if (!IsEof()) {
				if (*currentPosition == '\r')
					++currentPosition;
				if (*currentPosition == '\n')
					++currentPosition;
			}
			return result;
		}
		
		bool HasNext() override {
			EnsureCapacity();
			char* pos = currentPosition;
			SkipWhitespaces();
			bool hasNext = !IsEof();
			currentPosition = pos;
			return hasNext;
		}

	private:
		bool IsEof() const {
			return  *currentPosition == (char)0 && feof(input) != 0;;
		}

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
			while (*currentPosition <= ' ' && !IsEof()) {
				++currentPosition;
			}
		}
	};
}
