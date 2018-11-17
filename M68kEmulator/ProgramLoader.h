#pragma once
#include "Memory.h"
#include "CPUCore.h"

class ProgramLoader
{
public:
    static bool loadProgram(string fileName, CPUCore *cpu, Memory *memory);
private:
    static inline uint32_t extractHex(string record);
};
