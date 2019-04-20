/*
 * Derek Ng, djng@wpi.edu
 */
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

struct CacheLine {
	unsigned long tag;
	int lastUsed;
	bool valid;
};

struct CacheData {
	int s;
	int E;
	int b;
	int hits;
	int misses;
	int evicts;
};

struct CacheLine* initCache(struct CacheData* data) {
	unsigned long S = 1 << data->s;
	int E = data->E;
	struct CacheLine* cache = (struct CacheLine*) malloc(sizeof(struct CacheLine) * S * E);

	//printf("asdfghjkl;%ld %d %ld %ld %ld\n", sizeof(struct CacheLine), E, S, sizeof(*cache), sizeof(struct CacheLine)* E* S);
	struct CacheLine empty = {0, 0, false};
	for(unsigned long i = 0; i < S; i++) {
		for(int j = 0; j < E; j++) {
			*(cache + i * S + E) = empty;
		}
	}
	return cache;
}

void replace(struct CacheLine* cache, unsigned long setIndex, int lineIndex, unsigned long tag) {
	(cache + setIndex + lineIndex)->tag = tag;
	(cache + setIndex + lineIndex)->lastUsed = 0;
	(cache + setIndex + lineIndex)->valid = true;
}

struct CacheLine* oneStep(struct CacheLine* cache, struct CacheData* data, unsigned long address) {
	int tagSize = 64 - data->b - data->s;
	unsigned long tag = address >> (data->b + data->s);
	unsigned long setIndex = address << tagSize >> (tagSize + data->b);
	//printf("%d %ld, %ld\n", tagSize, tag, setIndex);
	bool replaced = false;
	for(int i = 0; !replaced && i < data->E; i++) {
		if(((cache + setIndex + i)->tag == tag) && (cache + setIndex + i)->valid) {
			data->hits++;
			(cache + setIndex + i)->lastUsed = 0;
			replaced = true;
		}
	}
	if(!replaced) {
		data->misses++;
		for(int j = 0; !replaced && j < data->E; j++) {
			if((cache + setIndex + j)->valid == false) {
				replace(cache, setIndex, j, tag);
				replaced = true;
			}
		}
		if(!replaced) {
			data->evicts++;
			int maxLastUsed = 0;
			int maxLastUsedIndex = 0;
			for(int j = 0; j < data->E; j++) {
				if((cache + setIndex + j)->lastUsed > maxLastUsed) {
					maxLastUsed = (cache + setIndex + j)->lastUsed;
					maxLastUsedIndex = j;
				}
			}
			replace(cache, setIndex, maxLastUsedIndex, tag);
		}
	}
	return cache;
}


int main(int argc, char *argv[]) {
	bool done = false;
	int s = -1;
	int E = -1;
	int b = -1;
	char* t = NULL;
	int opt;
	char* ptr = 0;
	while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
		switch (opt) {
		case 's':
			s = (int) strtol(optarg, &ptr, 10);
			break;
		case 'E':
			E = (int) strtol(optarg, &ptr, 10);
			break;
		case 'b':
			b = (int) strtol(optarg, &ptr, 10);
			break;
		case 't':
			t = optarg;
			break;
		default:
			printf("-%c is not an argument.\n", opt);
			printf("Usage:\n");
			printf("-s <num> -E <num> -b <num> -t <file>\n");
			done = true;
		}
	}

	if(s == -1 || E == -1 || b == -1) {
		printf("Missing argument\n");
		printf("Usage:\n");
		printf("-s <num> -E <num> -b <num> -t <file>\n");
		done = true;
	}

	FILE* fp = fopen(t, "r");
	if(fp == false) {
		printf("Cannot read file\n");
		done = true;
	} else if(!done) {
		struct CacheData data = {s, E, b, 0, 0, 0};
		//unsigned long S = 1 << s;
		struct CacheLine* cache = initCache(&data);
		/*for(unsigned long i = 0; i < S; i++) {
			for(int j = 0; j < E; j++) {
				unsigned long tag = (cache + i * S + E)->tag;
				int lastUsed = (cache + i * S + E)->lastUsed;
				bool valid = (cache + i * S + E)->valid;
				printf("S: %ld, E: %d, tag: %lx, lastUsed: %d, valid: %d\n", S, E, tag, lastUsed, valid);
			}
		}*/

		char operation = '\0'; //the text in a row of the input file
		unsigned long address = 0;
		int size = 0;

		while(fscanf(fp, " %c %lx,%d", &operation, &address, &size) != EOF) { //exits loop when end of the file is reached
			//printf("%c %lx %d\n", operation, address, size);
			switch(operation) {
			case 'L':
			case 'S':
				oneStep(cache, &data, address);
				break;
			case 'M':
				oneStep(cache, &data, address);
				oneStep(cache, &data, address);
				break;
			default:
				break;
			}
			if(operation != 'I') {
			printf("%c %lx %d\t", operation, address, size);
			printf("%d ", data.hits);
			printf("%d ", data.misses);
			printf("%d\n", data.evicts);
			}
		}

		fclose(fp);
	    printSummary(data.hits, data.misses, data.evicts);
	    free(cache);
	}

	//printf("%d, %d, %d, %s %ld\n", s, E, b, t, sizeof(long));


    return 0;
}

