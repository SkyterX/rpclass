#pragma once

#include <string>

// TODO : Abstract reader based on NextChar() and HasNext()
class IReader {
public:
	virtual ~IReader() {}

	virtual bool HasNext() = 0;
	virtual char NextChar() = 0;
	virtual unsigned int NextUnsignedInt() = 0;
	virtual std::string ReadLine() = 0;
};