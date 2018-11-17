#include "ProgramLoader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>

bool ProgramLoader::loadProgram(string fileName, CPUCore *cpu, Memory *memory)
{
    string line;
    ifstream programFile;
    programFile.open(fileName);
    if (programFile.is_open())
    {
        cout << "Loading..." << endl;
        while (getline(programFile, line))
        {
            string recordType = line.substr(0, 2);
            unsigned int recordSize = extractHex(line.substr(2, 2));
            string record = line.substr(4, recordSize * 2);
            if (recordType == "S1") {
                string memoryString = record.substr(0, 4);
                uint32_t memoryAddress = extractHex(memoryString);
                for (unsigned int i = 4; i < ((recordSize * 2) - 2); i += 2) {
                    unsigned int data = extractHex(record.substr(i, 2));
                    memory->writeByteToMemory(data, memoryAddress);
                    memoryAddress++;
                }
            }
            else if (recordType == "S2") {
                string memoryString = record.substr(0, 6);
                uint32_t memoryAddress = extractHex(memoryString);
                for (unsigned int i = 6; i < ((recordSize * 2) - 2); i += 2) {
                    unsigned int data = extractHex(record.substr(i, 2));
                    memory->writeByteToMemory(data, memoryAddress);
                    memoryAddress++;
                }
            }
            else if (recordType == "S3") {
                string memoryString = record.substr(0, 8);
                uint32_t memoryAddress = extractHex(memoryString);
                for (unsigned int i = 8; i < ((recordSize * 2) - 2); i += 2) {
                    unsigned int data = extractHex(record.substr(i, 2));
                    memory->writeByteToMemory(data, memoryAddress);
                    memoryAddress++;
                }
            }
            else if (recordType == "S7") {
                string memoryString = record.substr(0, 8);
                uint32_t memoryAddress = extractHex(memoryString);
                cpu->setProgramCounter(memoryAddress);
                memory->startingLocation = memoryAddress;
            }
            else if (recordType == "S8") {
                string memoryString = record.substr(0, 6);
                uint32_t memoryAddress = extractHex(memoryString);
                cpu->setProgramCounter(memoryAddress);
                memory->startingLocation = memoryAddress;
            }
            else if (recordType == "S9") {
                string memoryString = record.substr(0, 4);
                uint32_t memoryAddress = extractHex(memoryString);
                cpu->setProgramCounter(memoryAddress);
                memory->startingLocation = memoryAddress;
            }
            
        }
        cout << "Program loaded!" << endl << endl;
        programFile.close();
        return true;
    }

    else cout << "Unable to open file" << endl;
    return false;
    
}

inline uint32_t ProgramLoader::extractHex(string record)
{
    return strtoul(record.c_str(), NULL, 16);
}