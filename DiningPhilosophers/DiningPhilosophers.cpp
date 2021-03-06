#include <chrono>		//	Measure time
#include <ctime>		//	Measure time
#include <fstream>		//	Filewriter
#include <iomanip>
#include <iostream>		//	Console input
#include <omp.h>        //	OpenMP Library
#include <signal.h>		//	Use SIGNALS
#include <stdio.h>      //	I/O, Console Input
#include <stdlib.h>     //	Use NULL
#include <sstream>		// Time to string conversions
#include <windows.h>    //	Windows func
#include "stdafx.h"    //	Default Header

using namespace std;

#pragma region Definitions
#define NUM_PHILOSOPHERS 5
#pragma endregion

#pragma region static Variables
static int TIME_MAX_THINKING = 1000;		//	The maximum number of milli seconds to THINK
static int TIME_MAX_EATING = 5000;			//	The maximum number of milli seconds to EAT
static std::chrono::time_point<std::chrono::high_resolution_clock> TIME_STARTED;				// Time the app has been started
static bool FLAG_INIT_STOP = false;			//	Boolean FLAG to get prepare app shutdown
static __int64 WAITING_ELAPSED = 0;
static __int64 TOTAL_ELAPSED = 0;
static omp_lock_t forks[NUM_PHILOSOPHERS];  //	Array of omp_lock_t, representing the status of the forks 
static ofstream out_data("log.txt");
#pragma endregion

static void putBackForks(int left, int right, int id) {
	if ((rand() % 10) % 2) {
		omp_unset_lock(&forks[left]);
		omp_unset_lock(&forks[right]);
	}
	else {
		omp_unset_lock(&forks[right]);
		omp_unset_lock(&forks[left]);
	}
	printf("phil %d returned both forks\n", id);
}

void philosopher()
{
	//Wait for all threads to start
#pragma omp barrier

	// thread_num == phil idx
	int id = omp_get_thread_num();

	// fork variables, hold index of respective forks
	int fork_right;
	int fork_left;

	//Philosophers 0 through NUM_PHIL-2 reach to the left first then the right.
	if (!id % 2)
	{
		fork_right = id;
		fork_left = id + 1;
	}
	//Philosopher NUM_PHIL-1 has its left and right swapped.  Notice that it reaches in the
	//opposite order of all other philosophers.
	else
	{
		fork_right = 0;
		fork_left = id;
	}

	//Acquire chopsticks (semaphores), eat, wait for 100 microseconds, then release 
	//chopsticks (semaphores).
	// int i;
	bool FLAG_CONTINUE = true;

	//for (i = 0; i < MEALS; i++)
	while (FLAG_CONTINUE)
	{
		int thinking = (rand() % TIME_MAX_THINKING) + 1;
		Sleep(thinking);
		printf("phil %d finished thinking after %d milliseconds\n", id, thinking);

		auto t1 = std::chrono::high_resolution_clock::now();
		omp_set_lock(&forks[fork_left]);
		printf("phil %d took the left fork\n", id);

		omp_set_lock(&forks[fork_right]);
		printf("phil %d took the right fork\n", id);

		auto t2 = std::chrono::high_resolution_clock::now();
		WAITING_ELAPSED = WAITING_ELAPSED + std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

		int eating = (rand() % TIME_MAX_EATING) + 1;
		Sleep(eating);
		printf("phil %d finished eating after %d milliseconds\n", id, eating);

		// release forks, unset locks
		putBackForks(fork_left, fork_right, id);

		// conditional cleanup
		if (FLAG_INIT_STOP) {
			FLAG_CONTINUE = false; // break; break loop
		}
	}
}

static void log(string s, bool newLine)
{
	string entry = s;
	if (newLine) {
		entry.append("\n");
	}
	out_data << entry;
}

static void shutdown_handler(int signal)
{
	FLAG_INIT_STOP = true;

	for (int i = 0; i < NUM_PHILOSOPHERS; i++)
		omp_destroy_lock(&forks[i]);

	string s = "Total Waiting Time Elapsed: ";
	s.append(to_string(WAITING_ELAPSED) + " ms");
	log(s, true);

	auto t2 = std::chrono::high_resolution_clock::now();
	TOTAL_ELAPSED = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - TIME_STARTED).count();

	s = "Total Runtime: ";
	s.append(to_string(TOTAL_ELAPSED) + " ms");
	log(s, true);

	if (TOTAL_ELAPSED > 0) {
		s = "Waiting / Total Ratio: ";
		double ratio = (WAITING_ELAPSED / (double)TOTAL_ELAPSED) * 100;
		stringstream stream;
		stream << fixed << setprecision(2) << ratio;
		s.append(stream.str() + " %");
		stream.clear();
		log(s, true);

		s = "Waiting Time / Philosopher: ";
		double waiting = (WAITING_ELAPSED / (double)NUM_PHILOSOPHERS);
		stringstream stream2;
		stream2 << fixed << setprecision(2) << waiting;
		s.append(stream2.str() + " ms");
		stream.clear();
		log(s, true);

		s = "Waiting Total / Philosopher Ratio: ";
		double total_ratio = ((WAITING_ELAPSED / (double)NUM_PHILOSOPHERS) / (double)TOTAL_ELAPSED) * 100;
		stringstream stream3;
		stream3 << fixed << setprecision(2) << total_ratio;
		s.append(stream3.str() + " %");
		log(s, true);
		stream.clear();
		stream2.clear();
		stream3.clear();
	}
	s.clear();
	exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv)
{
	log("Dining philosophers", true);

	TIME_STARTED = std::chrono::high_resolution_clock::now();
	log("We have 5 philosophers waiting for dinner", true);
	printf("\n\tEnter the maximum thinking time (ms, default 1000) ...\n");

	string input;
	getline(cin, input);
	TIME_MAX_THINKING = stoi(input);
	log("Maximum thinking time has been set to " + input, true);
	printf("\n\tMaximum thinking = %d ...\n", TIME_MAX_THINKING);

	printf("\n\tEnter the maximum eating time (ms, default 5000) ...\n");
	getline(cin, input);
	log("Maximum eating time has been set to " + input, true);
	TIME_MAX_EATING = stoi(input);
	printf("\n\tMaximum eating = %d ...\n", TIME_MAX_EATING);

	input.clear();

	//	INIT OMP_INIT_LOCK for all forks
	for (int i = 0; i < NUM_PHILOSOPHERS; i++)
		omp_init_lock(&forks[i]);

	/* ATTACH SIGNAL EVENT HANDLERS */
	signal(SIGINT, shutdown_handler);
	signal(SIGTERM, shutdown_handler);
	signal(SIGBREAK, shutdown_handler);

	// OMP Parallel EXECUTION, num threads equals num philosophers
#pragma omp parallel num_threads(NUM_PHILOSOPHERS)
	{
		philosopher();
	}

	// Ensure safely shutdown of app
	FLAG_INIT_STOP = true;
	return EXIT_SUCCESS;
}