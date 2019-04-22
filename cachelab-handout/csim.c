/*
 * Derek Ng, djng@wpi.edu,
 * Matt Edwards, maedwards@wpi.edu
 */

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

struct CacheLine {
	unsigned long tag; //tag of a cache line
	int lastUsed; //number of cache accesses since a specific cache line was used
	bool valid; //valid bit of cache line
};

struct CacheData {
	int s; //number of set bits in an address
	int E; //number of cache lines in a set
	int b; //number of block bits in an address
	int hits; //total number of hits
	int misses; //total number of misses
	int evicts; //total number of evicts
};

/**
 * Initializes the cache simulator.
 * @param data A pointer to a struct containing data about the cache
 * @return A pointer to an empty cache
 */
struct CacheLine* initCache(struct CacheData* data) {
	unsigned long S = 1 << data->s; //number of sets in the cache
	int E = data->E; //number of lines in a set
	struct CacheLine* cache = (struct CacheLine*) malloc(sizeof(struct CacheLine) * S * E); //the cache
	struct CacheLine empty = {0, 0, false}; //an empty line (to be put in the cache)
	for(unsigned long i = 0; i < S; i++) { //populates the cache with empty lines
		for(int j = 0; j < E; j++) {
			*(cache + i * E + j) = empty;
		}
	}
	return cache;
}

/**
 * Replace a current line in the cache with a different line.
 * @param cacheLine The line in the cache to replace
 * @param tag The tag to replace the line with
 */
void replace(struct CacheLine* cacheLine, unsigned long tag) {
	cacheLine->tag = tag; //set the tag
	cacheLine->lastUsed = 0; //set last used to 0
	cacheLine->valid = true; //set the valid bit to true
}

/**
 * Accesses the cache once.
 * @param cache A pointer to the cache
 * @param data A pointer to a struct containing data about the cache
 * @param address An address from a memory trace
 */
void oneStep(struct CacheLine* cache, struct CacheData* data, unsigned long address) {
	int tagSize = 64 - data->b - data->s; //number of bits in the tag
	unsigned long tag = address >> (data->b + data->s); //the contents of the tag
	unsigned long setIndex = address << tagSize >> (tagSize + data->b); //the set number
	for(int i = 0; i < data->E; i++) { //adds 1 to each line in the set, used for the LRU replacement policy
		(cache + setIndex * data->E + i)->lastUsed++;
	}

	bool replaced = false;
	for(int i = 0; !replaced && i < data->E; i++) { //loops through lines in the set until match found
		//match if tag matches and is valid
		if(((cache + setIndex * data->E + i)->tag == tag) && (cache + setIndex * data->E + i)->valid) {
			data->hits++;
			(cache + setIndex * data->E + i)->lastUsed = 0; //resets the last used counter
			replaced = true;
		}
	}
	if(!replaced) { //a miss
		data->misses++;
		for(int j = 0; !replaced && j < data->E; j++) { //loops through lines in the set until invalid line
			if((cache + setIndex * data->E + j)->valid == false) { //invalid line if valid bit is not set
				replace((cache + setIndex * data->E + j), tag); //replace invalid line with current line
				replaced = true;
			}
		}
		if(!replaced) { //evict
			data->evicts++;
			int maxLastUsed = 0; //maximum number of times since a line in a set has been used
			int maxLastUsedIndex = 0; //the line that has least been used
			for(int j = 0; j < data->E; j++) { //finds the line least used
				if((cache + setIndex * data->E + j)->lastUsed > maxLastUsed) { //new least used line
					maxLastUsed = (cache + setIndex * data->E + j)->lastUsed;
					maxLastUsedIndex = j;
				}
			}
			replace((cache + setIndex * data->E + maxLastUsedIndex), tag); //replaces least used line with current line
		}
	}
}

/**
 * Prints proper usage of command line arguments.
 */
void usage(void) {
	printf("Usage:\n");
	printf("./csim -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("  -s <num>  Number of set index bits.\n");
	printf("  -E <num>  Number of lines per set.\n");
	printf("  -b <num>  Number of block offset bits.\n");
	printf("  -t <file> Trace file.\n");
}

/**
 * Main function.
 * @param argc Number of arguments on the command line
 * @param argv[] The argments on the command line
 * @return 0
 */
int main(int argc, char* argv[]) {
	bool done = false; //true if program should terminate
	int s = -1; //number of set bits in an address
	int E = -1; //number of lines in each set
	int b = -1; //number of block bits an address
	char* t = NULL; //file path of trace file
	int opt = '\0'; // a command line option
	char* ptr = 0; //pointer to get command line argument
	while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) { //loops through command line until no args left
		switch (opt) {
		case 's': //s option
			s = (int) strtol(optarg, &ptr, 10);
			break;
		case 'E': //E option
			E = (int) strtol(optarg, &ptr, 10);
			break;
		case 'b': //b option
			b = (int) strtol(optarg, &ptr, 10);
			break;
		case 't': //t option
			t = optarg;
			break;
		default: //not valid option, exits program
			usage();
			done = true;
		}
	}

	if((s < 1 || E < 1 || b < 1) && !done) { //an argument is missing or incorrect, exits program
		printf("Missing required command line argument\n");
		usage();
		done = true;
	}

	FILE* fp = fopen(t, "r"); //opens trace file
	if(fp == false) { //cannot read the file
		printf("Cannot read file\n");
		done = true;
	} else if(!done) { //file successfully opened
		struct CacheData data = {s, E, b, 0, 0, 0}; //initial data about cache
		struct CacheLine* cache = initCache(&data); //initialize cache

		char operation = '\0'; //an operation (I, S, L, M) from the trace file
		unsigned long address = 0; //an address from the trace file
		int size = 0; //a size from the trace file
		while(fscanf(fp, " %c %lx,%d", &operation, &address, &size) != EOF) { //exits loop when end of the file is reached
			switch(operation) {
			case 'L': //L and S accesses the cache once each
			case 'S':
				oneStep(cache, &data, address);
				break;
			case 'M': //M accesses the cache twice
				oneStep(cache, &data, address);
				oneStep(cache, &data, address);
				break;
			default: //I does nothing
				break;
			}
		}
		fclose(fp); //close file
	    printSummary(data.hits, data.misses, data.evicts); // print summary
	    free(cache); //free memory
	}
    return 0;
}
