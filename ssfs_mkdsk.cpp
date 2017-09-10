

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <cstdio> 

#include "structs.h"

using namespace std;

int main(int argc, char const *argv[]) {

	/*

	Check file system parameters

	*/

	if(!argv[1] || !argv[2]) {
		cout << "Did not specify number of blocks and block size." << endl;
		exit(1);
	}

	int numBlocks = atoi(argv[1]);
	if(numBlocks < 1024 || numBlocks > 128000) {
		cout << "Number of blocks must be between 1024 and 128,000" << endl;
		exit(1);

	}

	int blockSize = atoi(argv[2]);
	if(blockSize < 128 || blockSize > 512) {
		cout << "Block size must be between 128 to 512 bytes." << endl;
		exit(1);
	}

	string filename;
	if(argv[3]) filename = argv[3];
	else filename = "DISK.bin";

	/*

	Instantiate data structures to write to disk

	*/

	SB SB {numBlocks, blockSize, 0}; // instantiate superblock struct
	bool iNodeMap[256] = {false};
	iNode iNodes[256];
	int blockListSize = numBlocks - 256-2-1; // subtract inode size, bitmap size, superblock
	bool* blockList = new bool [blockListSize] {false}; //free block list
	unsigned char userDataBlock[blockListSize];

	char* charBuffer = new char [blockSize];

	// initalize direct pointers to -1
	for(int i=0; i<256; i++) {
		for(int j=0; j<12; j++) {
			iNodes[i].directPointers[j] = -1;
		}
	}
	/*

	Write to disk

	*/

	FILE* diskFile = fopen(filename.c_str(), "wb"); // write in binary mode
	if(diskFile!=NULL) {

		//write SB to file (for array, replace 1 with size of array)
		fwrite(&SB, sizeof(SB), 1, diskFile);

		fseek (diskFile, blockSize, SEEK_SET); // move 1 block over
		fwrite(iNodeMap, sizeof(bool), 256, diskFile);
		perror("Blocklist");

		//fseek (diskFile, 0, SEEK_SET); // move to the beginning of file
		fseek (diskFile, blockSize*2, SEEK_SET); // move 2 blocks over
		fwrite(blockList, sizeof(bool), blockListSize, diskFile);
		perror("Blocklist");

		//fseek (diskFile, 0, SEEK_SET); // move to the beginning of file
		fseek (diskFile, blockSize*3, SEEK_SET); // move 3 blocks over
		fwrite(iNodes, sizeof(iNode), 256, diskFile);
		perror("Writing iNodes");

		fseek (diskFile, blockSize*4, SEEK_SET); // move 4 blocks over
		fwrite(charBuffer, sizeof(charBuffer), blockListSize, diskFile);

   }

	fclose(diskFile);



	return 0;
}
