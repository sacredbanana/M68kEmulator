#pragma once
#include "Memory.h"
#include "CPUCore.h"

class ProgramLoader
{
public:
	static void loadProgram(string fileName, CPUCore *cpu, Memory *memory);
private:
	static inline uint32_t ProgramLoader::extractHex(string record);
};

