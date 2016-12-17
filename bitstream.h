#ifndef _bitstream_h
#define _bitstream_h

#include <fstream>
#include "lib/utility.h"


/* Defines a classes for reading and writing one bit at a time to a stream
 * which is associated with provided file since fstream doesn't provide this functionality
 *
 * Other than reading/writing bits, also provide this additional functionality:
 * read/write byte, get stream size and rewind input stream to beginning for reading again
 * read/write bit and byte are done in raw binary format, no formating is done
 *
 * Besides this limited functionality everything else ir restricted:
 * doesn't support standard << >> stream operators, everything is done through provided methods
 */


static const s32 BITS_IN_BYTE = 8;
// returns the next bit from index position within a byte
static inline s32 getBit(s32 byte, s32 index) { return (byte & (1 << index)) != 0; }
// sets bit to one in index position within a byte
static inline void setBit(s32 & byte, s32 index) { byte |= (1 << index); }


class ifbitstream
{
	std::ifstream file; // stream buffer

	s32 byte; // value of current byte
	s32 index; // bit position within a byte

public:

	ifbitstream() { /* empty */ }

	ifbitstream(const char* filename): file(filename, std::fstream::binary | std::fstream::in)
	{
		this->byte = 0;
		this->index = BITS_IN_BYTE; // so that the next read will trigger to get the first byte
	}

	void open(const char* filename)
	{
		this->byte = 0;
		this->index = BITS_IN_BYTE; // so that the next read will trigger to get the first byte
		file.open(filename, std::fstream::binary | std::fstream::in);
	}

	bool is_open() { return file.is_open(); }
	void close() { file.close(); }


	// returns next bit within a stream, -1 (EOF) if stream has reached the end
	s32 readBit()
	{
		if (!file.is_open()) ERROR("ifbitstream: can't read a bit from a stream not associated with a file");

		// get the next byte
		if (index == BITS_IN_BYTE)
		{
			if ((byte = file.get()) == EOF) return EOF;
			index = 0;
		}
		// return next bit and advance bit position
		return getBit(byte, index++);
	}

	// returns next byte within a stream, -1 (EOF) if stream has reached the end
	// if stream has only 1-7 bits left then return those instead of EOF, next read of bit or byte will return EOF
	s32 readByte()
	{
		if (!file.is_open()) ERROR("ifbitstream: can't read a bit from a stream not associated with a file");

		if (index == BITS_IN_BYTE) return file.get();

		// get remaining bits from this byte
		s32 result = byte >> index;
		// and the rest from the next byte
		if ((byte = file.get()) != EOF)
			result |= ((byte & ((1 << index) - 1)) << (BITS_IN_BYTE - index));
		else 
			this->index = BITS_IN_BYTE;

		return result;
	}


	// returns a s64 value of only first 4 bytes being valid from stream
	// a s64 not u32 is chosen, so that -1 (EOF) could be returned
	// so a client must use s64 type to receive those 4 bytes, otherwise if s32 type is used and
	// method returns (2^31 - 1) (all 32bits set to one): which is still valid 4 byte value,
	// will get interpreted as -1 (EOF) by truncating to type s32 and falsely indicating the EOF
	// just like with readByte() if stream has only 1-31 bits left then return those instead of EOF
	s64 readFourBytes()
	{
		s64 bytes = 0;
		for (s32 offset = 0; offset < 4; ++offset)
		{
			s64 byte = readByte();
			if (byte == EOF && offset == 0) return EOF;
			else if (byte == EOF) break;

			bytes |= byte << (8*offset);
		}
		return bytes;
	}

	// rewinds read stream to beginning
	void rewind()
	{
		if (!file.is_open()) ERROR("ifbitstream: can't rewind to beggining of a stream not associated with a file");

		// clear incase file was allready in eof state, otherwise seekg won't work
		file.clear();
		file.seekg(0, std::fstream::beg);
	}

	// returns the size of the stream in bytes
	u32 size()
	{
		if (!file.is_open()) ERROR("ifbitstream: can't get a size of a stream not associated with a file");

		file.clear();
		std::streampos pos = file.tellg();
		file.seekg(0, std::fstream::end);
		std::streampos end = file.tellg();
		file.seekg(pos);
		return end;
	}
};

class ofbitstream
{
	std::ofstream file; // stream buffer

	s32 byte; // value of current byte
	s32 index; // bit position within a byte

public:

	ofbitstream() { /* empty */ }

	ofbitstream(const char* filename)
	{
		this->byte = 0;
		this->index = BITS_IN_BYTE; // so that the next read will trigger to get the first byte
		file.open(filename, std::fstream::binary | std::fstream::out);
	}

	void open(const char* filename)
	{
		this->byte = 0;
		this->index = BITS_IN_BYTE; // so that the next read will trigger to get the first byte
		file.open(filename, std::fstream::binary | std::fstream::out);
	}

	bool is_open() { return file.is_open(); }
	void close() { file.close(); }


	// writes given bit to the stream
	void writeBit(bool bit)
	{
		if (!file.is_open()) ERROR("ofbitstream: can't write a bit to a stream not associated with a file");

		if (index == BITS_IN_BYTE)
		{
			byte = 0;
			index = 0;
		}

		// if bit is 1, then set approriate bit position to 1
		// otherwhise skip since it allready has 0 there by default
		if (bit) setBit(byte, index);

		// if index is 0, then put the new byte to a stream, otherwhise overwrite last byte
		if (index == 0) file.put(byte);
		else if (bit)
		{
			file.seekp(-1, std::fstream::cur);
			file.put(byte);
		}
		++index; // advance bit position
	}

	// writes a given byte to a stream
	// unsigned char is essential, otherwhise some bitwise operations might not work, for example:
	// if byte is 0b11111111 and num is any number, then (byte >> num) will return byte unchanged (0b11111111)
	void writeByte(u8 byte)
	{
		if (!file.is_open()) ERROR("ofbitstream: can't write a byte to a stream not associated with a file");

		if (index == BITS_IN_BYTE)
		{
			file.put(byte);
			return;
		}

		// fill the remaining of this->byte and overwrite the last byte in a stream with it
		this->byte |= ((byte & ((1 << (BITS_IN_BYTE - index)) - 1)) << index);
		file.seekp(-1, std::fstream::cur);
		file.put(this->byte);
		// put the rest of bits to a new byte and put it in a stream
		this->byte = (byte >> (BITS_IN_BYTE - index));
		file.put(this->byte);
	}

	// writes 4 bytes to a stream from a given u32 value
	void writeFourBytes(u32 bytes)
	{
		for (s32 offset = 0; offset < 4; ++offset)
		{
			u8 byte = *((u8*)&bytes + offset);
			writeByte(byte);
		}
	}

	// returns the size of the stream in bytes
	u32 size()
	{
		if (!file.is_open()) ERROR("ofbitstream: can't get a size of a stream not associated with a file");
		file.clear();

		std::streampos pos = file.tellp();
		file.seekp(0, std::fstream::end);
		std::streampos end = file.tellp();
		file.seekp(pos);
		return end;
	}
};

#endif