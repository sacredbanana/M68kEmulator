// M68kEmulator.cpp : Defines the entry point for the console application.
//

#include "CPUCore.h"
#include "Memory.h"
#include "ProgramLoader.h"
#include <iostream>
#ifdef _DEBUG
#define DEBUG_MODE 1
#else
#define DEBUG_MODE 0
#endif
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
		<< "Made by Cameron Armstrong 2015-2018" << settextcolor(console_text_colors::light_green) << endl << endl;

#else
	cout << "M68kEmulator" << endl	<< "Made by Cameron Armstrong 2015-2018" << endl << endl;
#endif

	Memory *memory = new Memory(256);
	CPUCore *cpu = new CPUCore(memory, 68000);
	if (!ProgramLoader::loadProgram("program.S68", cpu, memory)) {
		cout << "Program loader failed. Exiting." << endl;
		return 1;
	}
	bool cpuRunning = true;
	while (cpuRunning)
		cpuRunning = cpu->startNextCycle();
	cout << endl << "Execution completed." << endl << endl;
	if (DEBUG_MODE) {
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

