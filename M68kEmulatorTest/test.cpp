#include "pch.h"
#include "../M68kEmulator/Memory.cpp"
#include "../M68kEmulator/CPUCore.cpp"

class CPUInitTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        cpu->setAllRegisters(testNumber);
    }

    ~CPUInitTest() {
        delete cpu;
    }

    CPUCore *cpu = new CPUCore(NULL, 68000);
    uint32_t testNumber = 0x1234ABCD;
};

class MemoryTest : public ::testing::Test {
protected:
    Memory *memory = new Memory(8);

    ~MemoryTest() {
        delete memory;
    }
};

class InstructionTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        cpu->setAllRegisters(0x1234ABCD);
    }

    ~InstructionTest() {
        delete memory;
        delete cpu;
    }

    Memory *memory = new Memory(8);
    CPUCore *cpu = new CPUCore(memory, 68000);
};

TEST_F(CPUInitTest, DataRegister) {
    for (int reg = 0; reg <= 7; reg++)
        EXPECT_EQ(cpu->getDataRegister(reg), testNumber);
}

TEST_F(CPUInitTest, AddressRegister) {
    for (int reg = 0; reg <= 7; reg++)
        EXPECT_EQ(cpu->getAddressRegister(reg), testNumber);
}

TEST_F(MemoryTest, ByteReadWrite)
{
    memory->writeByteToMemory(0xCA, 24);
    EXPECT_EQ(memory->readByteFromMemory(24), 0xCA);
}

TEST_F(MemoryTest, WordReadWrite)
{
    memory->writeWordToMemory(0xCA87, 24);
    EXPECT_EQ(memory->readWordFromMemory(24), 0xCA87);
    EXPECT_EQ(memory->readByteFromMemory(24), 0xCA);
    EXPECT_EQ(memory->readByteFromMemory(25), 0x87);
}

TEST_F(MemoryTest, LongReadWrite)
{
    memory->writeLongToMemory(0xCA87BEEF, 24);
    EXPECT_EQ(memory->readLongFromMemory(24), 0xCA87BEEF);
    EXPECT_EQ(memory->readByteFromMemory(24), 0xCA);
    EXPECT_EQ(memory->readByteFromMemory(25), 0x87);
    EXPECT_EQ(memory->readByteFromMemory(26), 0xBE);
    EXPECT_EQ(memory->readByteFromMemory(27), 0xEF);
}

TEST_F(InstructionTest, ClearByte)
{
    // CLR.B D0
    memory->writeWordToMemory(0x4200, 0);
    cpu->startNextCycle();
    EXPECT_EQ(cpu->getDataRegister(0), 0x1234AB00);
    
    // CLR.B (A0)
    cpu->setAddressRegister(0, 0x16);
    cpu->setProgramCounter(0);
    memory->writeWordToMemory(0x4210, 0);
    memory->writeByteToMemory(0xCA, 0x16);
    cpu->startNextCycle();
    EXPECT_EQ(memory->readByteFromMemory(0x16), 0);

    // CLR.B (A1)+
    cpu->setAddressRegister(1, 0x16);
    cpu->setProgramCounter(0);
    memory->writeWordToMemory(0x4219, 0);
    memory->writeWordToMemory(0xCA87, 0x16);
    cpu->startNextCycle();
    EXPECT_EQ(memory->readByteFromMemory(0x16), 0);
    EXPECT_EQ(cpu->getAddressRegister(1), 0x17);

    // CLR.B -(A2)
    cpu->setAddressRegister(2, 0x17);
    cpu->setProgramCounter(0);
    memory->writeWordToMemory(0x4222, 0);
    memory->writeWordToMemory(0xCA87, 0x16);
    cpu->startNextCycle();
    EXPECT_EQ(memory->readByteFromMemory(0x16), 0);
    EXPECT_EQ(cpu->getAddressRegister(2), 0x16);

    // CLR.B 7(A3)
    cpu->setAddressRegister(3, 0x10);
    cpu->setProgramCounter(0);
    memory->writeLongToMemory(0x422B0007, 0);
    memory->writeWordToMemory(0xCA87, 0x16);
    cpu->startNextCycle();
    EXPECT_EQ(memory->readByteFromMemory(0x17), 0);

    // CLR.B -7(A3)
    cpu->setAddressRegister(3, 0x10);
    cpu->setProgramCounter(0);
    memory->writeLongToMemory(0x422BFFF9, 0);
    memory->writeWordToMemory(0xCA87, 0x8);
    cpu->startNextCycle();
    EXPECT_EQ(memory->readByteFromMemory(0x9), 0);

    // CLR.B 7(A4, 8)
    cpu->setAddressRegister(3, 0x10);
    cpu->setProgramCounter(0);
    memory->writeLongToMemory(0x422b0007, 0);
    memory->writeWordToMemory(0xCA87, 0x16);
    cpu->startNextCycle();
    EXPECT_EQ(memory->readByteFromMemory(0x17), 0);
    EXPECT_EQ(cpu->getAddressRegister(3), 0x10);
}