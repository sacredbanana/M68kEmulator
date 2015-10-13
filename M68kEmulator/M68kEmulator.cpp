// M68kEmulator.cpp : Defines the entry point for the console application.
//

#include "CPUCore.h"
#include "Memory.h"
#include "ProgramLoader.h"
#include <iostream>
#ifdef WIN32
#include "conmanip.h"
using namespace conmanip;
#endif

using namespace std;

int main()
{

#ifdef WIN32
	// create a console context object, used for restoring console settings
	console_out_context ctxout;
	// create a console object
	console_out conout(ctxout);

	// change the title of the console
	conout.settitle("M68kEmulator");

	conout.setbgcolor(console_bg_colors::blue);
	system("cls");

	// output text with colors
	cout << settextcolor(console_text_colors::light_yellow)
		//<< setbgcolor(console_bg_colors::cyan)
		<< "M68kEmulator" << endl
		<< settextcolor(console_text_colors::light_magenta)
		<< "(c) Cameron Armstrong 2015" << endl << endl;

#else
	cout << "M68kEmulator" << endl	<< "(c) Cameron Armstrong 2015" << endl << endl;
#endif

	Memory *memory = new Memory(256);
	CPUCore *cpu = new CPUCore(memory, 68000);
	cpu->debugMode = false;
	ProgramLoader::loadProgram("program.S68", cpu, memory);
	bool cpuRunning = true;
	while (cpuRunning)
		cpuRunning = cpu->startNextCycle();
	cout << endl << "Execution completed." << endl << endl;
	if (cpu->debugMode) {
		cpu->displayInfo();
		memory->dumpMemoryToConsole();
		memory->dumpMemoryToFile("core_dump.txt");
	}

#ifdef WIN32
	// restore console attributes (text and background colors)
	ctxout.restore(console_cleanup_options::restore_attibutes);
#endif

    return 0;
}

