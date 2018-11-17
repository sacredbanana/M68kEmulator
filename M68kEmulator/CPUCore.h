#pragma once
#include <cstdint>
#include "Memory.h"

#define SP A[7]

class CPUCore
{
private:
    uint32_t D[8]; // Data registers
    uint32_t A[8]; // Address registers + SP
    uint32_t PC; // Program Counter register
    uint16_t SR; // Status register

    enum models {
        MC68000 = 68000,
        MC68010 = 68010,
        MC68020 = 68020,
        MC68040 = 68040,
        MC68060 = 68060
    } model;

    Memory *memory = nullptr;

    bool decodeInstruction(uint16_t instruction);
    void writeByteToDataRegister(uint8_t data, int reg);
    void writeWordToDataRegister(uint16_t data, int reg);
    void writeLongToDataRegister(uint32_t data, int reg);
    void writeByteToAddressRegister(uint8_t data, int reg);
    void writeWordToAddressRegister(uint16_t data, int reg);
    void writeLongToAddressRegister(uint32_t data, int reg);
public:
    CPUCore(Memory *memory, int model);
    ~CPUCore();
    bool startNextCycle();
    void displayInfo();
    void setProgramCounter(unsigned int memoryLocation);
    // Sets all data and address registers to a value. Used for testing.
    void setAllRegisters(uint32_t value);
    // Returns the contents of data register
    uint32_t getDataRegister(int reg);
    // Returns the contents of address register
    uint32_t getAddressRegister(int reg);
    // Set the contents of data register
    void setDataRegister(int reg, uint32_t data);
    // Set the contents of data register
    void setAddressRegister(int reg, uint32_t data);
};

