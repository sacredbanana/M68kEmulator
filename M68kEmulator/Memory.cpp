#include "Memory.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdint>

Memory::Memory(unsigned int sizeinKB)
{
    this->sizeInKB = sizeinKB;
    unsigned int sizeInBytes = sizeinKB * 1024;
    memoryBlock = new uint8_t[sizeInBytes];
    clearMemory(0);
}


Memory::~Memory()
{
    delete memoryBlock;
}

uint8_t Memory::readByteFromMemory(uint32_t address, int offset)
{
    return memoryBlock[address + offset];
}

uint16_t Memory::readWordFromMemory(uint32_t address, int offset)
{
    uint16_t data = memoryBlock[address + offset] << 8;
    data += memoryBlock[address + offset + 1];
    return data;
}

uint32_t Memory::readLongFromMemory(uint32_t address, int offset)
{
    uint32_t data = memoryBlock[address + offset] << 24;
    data += memoryBlock[address + offset + 1] << 16;
    data += memoryBlock[address + offset + 2] << 8;
    data += memoryBlock[address + offset + 3];
    return data;
}

void Memory::writeByteToMemory(uint8_t data, uint32_t address, int offset)
{
    memoryBlock[address + offset] = data;
}

void Memory::writeWordToMemory(uint16_t data, uint32_t address, int offset)
{
    uint8_t firstByte = (uint8_t)(data >> 8);
    uint8_t secondByte = (uint8_t)data;
    memoryBlock[address + offset] = firstByte;
    memoryBlock[address + 1 + offset] = secondByte;
}

void Memory::writeLongToMemory(uint32_t data, uint32_t address, int offset)
{
    uint8_t firstByte = (uint8_t)(data >> 24);
    uint8_t secondByte = (uint8_t)(data >> 16);
    uint8_t thirdByte = (uint8_t)(data >> 8);
    uint8_t fourthByte = (uint8_t)data;
    memoryBlock[address + offset] = firstByte;
    memoryBlock[address + 1 + offset] = secondByte;
    memoryBlock[address + 2 + offset] = thirdByte;
    memoryBlock[address + 3 + offset] = fourthByte;
}

void Memory::clearMemory(uint8_t value)
{
    unsigned int sizeInBytes = sizeInKB * 1024;
    memset(memoryBlock, value, sizeInBytes);
}

void Memory::dumpMemoryToFile(string fileName)
{
    ofstream file;
    file.open(fileName);

    unsigned int rowsToShow = (sizeInKB * 1024) / 16;

    file << "RAM Size: " << sizeInKB << "KB" << endl << endl;
    file << std::left << setw(17) << "Address" << setw(3) << "0" << setw(3) << "1" << setw(3) << "2" << setw(3) << "3" << setw(3) << "4" << setw(3) << "5" << setw(3) << "6"
        << setw(3) << "7" << setw(3) << "8" << setw(3) << "9" << setw(3) << "A" << setw(3) << "B" << setw(3) << "C" << setw(3) << "D" << setw(3) << "E" << setw(3) << "F"
        << "0123456789ABCDEF" << endl;
    file << setfill('-') << setw(81) << "-" << endl;
    for (unsigned int row = 0; row < rowsToShow; row++) {
        file << right << setw(16) << setfill('0') << hex << row * 16 << " " << left << setfill(' ');
        for (int offset = 0; offset < 16; offset++)
            file << setw(3) << hex << uppercase << (int)memoryBlock[(16 * row) + offset];
        for (int offset = 0; offset < 16; offset++) {
            if (((16 * row) + offset) >= (sizeInKB * 1024)) {
                file << " ";
                break;
            }

            uint8_t character = memoryBlock[(16 * row) + offset];
            if (character < 32 || character > 126)
                file << "-";
            else
                file << dec << character;
        }
        file << endl;

    }

    file.close();
}

void Memory::dumpMemoryToConsole(unsigned int rowsToShow) 
{
    cout << "RAM Size: " << sizeInKB << "KB" << endl << endl;
    cout << std::left << setw(17) << "Address" << setw(3) << "0" << setw(3) << "1" << setw(3) << "2" << setw(3) << "3" << setw(3) << "4" << setw(3) << "5" << setw(3) << "6"
        << setw(3) << "7" << setw(3) << "8" << setw(3) << "9" << setw(3) << "A" << setw(3) << "B" << setw(3) << "C" << setw(3) << "D" << setw(3) << "E" << setw(3) << "F"
        << "0123456789ABCDEF" << endl;
    cout << setfill('-') << setw(81) << "-" << endl;
    for (unsigned int row = 0; row < rowsToShow; row++) {
        cout << right << setw(16) << setfill('0') << hex << (row * 16) + startingLocation << " " << left << setfill(' ');
        for (int offset = 0; offset < 16; offset++)
            cout << setw(2) << hex << setfill('0') << uppercase << (int)memoryBlock[startingLocation + (16 * row) + offset] << " ";
        for (int offset = 0; offset < 16; offset++) {
            if ((startingLocation + (16 * row) + offset) >= (sizeInKB * 1024)) {
                cout << " ";
                break;
            }
            uint8_t character = memoryBlock[startingLocation + (16 * row) + offset];
            if (character < 32 || character > 126)
                cout << "-";
            else
                cout << dec << character;
        }
        cout << endl;
    }
}

void Memory::loadMemoryFromFile(string fileName)
{

    //for (int address = 0; address < )
}

void Memory::insertString(string s, unsigned int address)
{
    unsigned int offset = 0;

    for (char character : s)
        memoryBlock[address + offset++] = (uint8_t)character;
}
