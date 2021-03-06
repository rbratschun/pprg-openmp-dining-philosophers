// stdafx.h: Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO: Hier auf zusätzliche Header, die das Programm erfordert, verweisen.
#include <chrono>		//	Stopwatch, measure elapsed time
#include <ctime>		//	Time Functions
#include <fstream>		//	Write to file
#include <iomanip>
#include <iostream>		//	I/O, Console input
#include <omp.h>		//	OpenMP Lib
#include <signal.h>		//	SIGNALS (EXIT ...), event handlers
#include <string>		//	Read line from console, process input
#include <sstream>		//	String Streams for conversions
#include <windows.h>	//	Console functions