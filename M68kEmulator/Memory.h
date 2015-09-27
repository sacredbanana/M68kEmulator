#pragma once
#include <cstdint>
#include <string>

using namespace std;

class Memory
{
private:
	uint8_t *memoryBlock;
	unsigned int sizeInKB;
	void clearMemory();
	void insertString(string s, unsigned int address);
public:
	Memory(unsigned int sizeInKB = 256);
	~Memory();
	uint32_t startingLocation = 0;
	uint8_t readByteFromMemory(uint32_t address, int offset = 0);
	uint16_t readWordFromMemory(uint32_t address, int offset = 0);
	uint32_t readLongFromMemory(uint32_t address, int offset = 0);
	void writeByteToMemory(uint8_t data, uint32_t address, int offset = 0);
	void writeWordToMemory(uint16_t data, uint32_t address, int offset = 0);
	void writeLongToMemory(uint32_t data, uint32_t address, int offset = 0);
	void dumpMemoryToFile(std::string fileName);
	void dumpMemoryToConsole(unsigned int rowsToShow = 20);
	void loadMemoryFromFile(std::string fileName);
};

