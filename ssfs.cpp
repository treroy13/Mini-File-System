#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio> 
#include <string.h> 
#include <math.h>
#include <unistd.h>
#include <sstream>

#include "structs.h"

using namespace std;
#define d 256
static const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

char genRand(){
	return alpha[ rand() % (sizeof(alpha) -1)];

}

/*

Global variables

*/

SB SB;
bool iNodeMap[256];
iNode iNodes[256];

bool* blockList;
string DISKFILE;
char *userDataBlock;

/*

Disk Operations

*/

void create(string filename);
void deleteFile(string filename);
int physicalAddr(int index, int i);
void list();
void cat(string filename);
void write(string filename, char ch, int startByte, int numBytes);
void read(string filename, int startByte, int numBytes);
void import(string filename, string unixFilename);
void shutdown();
int main(int argc, char const *argv[]) {

	/*

	Check file system parameters

	*/

	if(argv[1]) DISKFILE = argv[1];
	else {
		cout << "Disk file not provided." << endl;
		exit(1);
	}



	FILE* diskFile =  fopen(DISKFILE.c_str(), "rb");

	if(diskFile!=NULL) {

		fread(&SB, sizeof(SB), 1, diskFile);

		fseek (diskFile, 0, SEEK_SET); // move to the beginning of file
		fseek (diskFile, sizeof(char)*SB.blkSize, SEEK_CUR); // move 1 block over
		fread(iNodeMap, sizeof(bool), 256, diskFile);

		blockList = new bool [SB.numBlks - 253] {false};

		fseek (diskFile, sizeof(char)*SB.blkSize*2, SEEK_CUR); // move 2 blocks over
		fread(blockList, sizeof(bool), (SB.numBlks - 253), diskFile);

		fseek (diskFile, sizeof(char)*SB.blkSize*3, SEEK_CUR); // move 3 blocks over
		fread(iNodes, sizeof(iNode), 256, diskFile);

		userDataBlock = new char [SB.numBlks- 256-2-1]; // subtract inode size, bitmap size, superblock]

		fseek (diskFile, sizeof(char)*SB.blkSize*4, SEEK_CUR); // move 4 blocks over
		fread(userDataBlock, 1, SB.numBlks- 256-2-1, diskFile);


	}

	fclose(diskFile);

	string line, command;
	string myFile = argv[2];
	ifstream infile(myFile);

	while(getline(infile, line)){

		stringstream s(line);

		if(getline(s, command, ' ')){

			//create command
			if(command == "CREATE"){
				stringstream ss(line);
				string createFile;
				if(getline(ss, createFile, ' ')){}
				if(getline(ss, createFile, '\n')){}	
				create(createFile);		
			}

			//import command
			else if(command == "IMPORT"){
				stringstream ss(line);
				string ssfsFile, unixFile;
				if(getline(ss, ssfsFile, ' ')){}
				if(getline(ss, ssfsFile, ' ')){}								
				if(getline(ss, unixFile, ' ')){}
				import(ssfsFile, unixFile);	
			}
			//cat command
			else if(command == "CAT"){		
				stringstream ss(line);
				string catFile;
				if(getline(ss, catFile, ' ')){}
				if(getline(ss, catFile, '\n')){}
				cat(catFile);
			}
			//delete command
			else if(command == "DELETE"){

				stringstream ss(line);
				string delFile;
				if(getline(ss, delFile, ' ')){}
				if(getline(ss, delFile, '\n')){}
				deleteFile(delFile);
			}
			//write command
			else if(command == "WRITE"){ //string filename, char ch, int startByte, int numByte

				stringstream ss(line);
				string ssfsFile, c, num1, num2;
				char ch;
				int startByte, numByte;

				if(getline(ss, ssfsFile, ' ')){}

				//name of ssfs file
				if(getline(ss, ssfsFile, ' ')){}

				//character ch to pass in							
				if(getline(ss, c, ' ')){stringstream sss(c);  sss >> ch;}

				//integer start byte
				if(getline(ss, num1, ' ')){ stringstream sss(num1);  sss >> startByte;}

				//integer number byte
				if(getline(ss, num2, '\n')){  stringstream sss(num2);  sss >> numByte; } 
				
				//call the write function
				write(ssfsFile, ch, startByte, numByte);
						
			}
			//read command
			else if(command == "READ"){
				stringstream ss(line);
				string ssfsFile, num1, num2;
				int startByte, numByte;

				if(getline(ss, ssfsFile, ' ')){}

				//name of ssfs file
				if(getline(ss, ssfsFile, ' ')){}

				//integer start byte
				if(getline(ss, num1, ' ')){ stringstream sss(num1);  sss >> startByte;}

				//integer number byte
				if(getline(ss, num2, '\n')){  stringstream sss(num2);  sss >> numByte; } 

				//call the read function		
				read(ssfsFile, startByte, numByte);			

			}
			//list command
			else if(command == "LIST"){
				list();

			}
			//shutdown command
			else if(command == "SHUTDOWN"){
				shutdown();

			}
		}	
		
	}

	
	srand(time(0));

	//BONUS
	for(int i =0; i <254; i++){
		string txt;		
		for(int k = 0; k < 10; k++){
			txt += genRand();
		}
		cout<<i<<": "<<txt<<endl;
		create(txt);
	}

	cout<<"254: apple"<<endl;
	create("apple");
	create("banana");
	list();
	deleteFile("apple");
	create("mike");
	list();

	return 0;
}
void create(string filename) {

	if(filename.length() > 32) {
		cout << "Filename length must be 32 characters or less." << endl;
		return;
	}

	int index;
	bool found = false;
	// find a free inode
	for(int i=0; i<256; i++) {
		if(iNodeMap[i] == false) {
			index = i;
			found = true;
			break;
		}
	}

	if(!found) {
		cout << "No more files can be created." << endl;
		return;
	}
	iNodeMap[index] = true;
	FILE* diskFile = fopen(DISKFILE.c_str(), "rb"); // read in binary mode

	fseek (diskFile, 0, SEEK_SET); // move to the beginning of file
	fseek (diskFile, SB.blkSize*3, SEEK_CUR); // move 3 blocks over
	fseek (diskFile, sizeof(iNode)*index, SEEK_CUR); // move to the ith iNode

	iNode in;
	fread(&in, sizeof(iNode), 1, diskFile);

	fclose(diskFile);

	diskFile = fopen(DISKFILE.c_str(), "rb+wb");

	fseek (diskFile, 0, SEEK_SET); // move to the beginning of file
	fseek (diskFile, SB.blkSize*3, SEEK_CUR); // move 3 blocks over
	fseek (diskFile, sizeof(iNode)*index, SEEK_CUR); // move to the ith iNode

	strcpy(in.fileName, filename.c_str());
	in.fileSize = 0;
	for(int i=0; i<12; i++){
		in.directPointers[i] = -1; // fix this
	}

	in.indirectPointer = -1;
	in.doubleIndirectPointer = -1;

	fwrite(&in, sizeof(iNode), 1, diskFile);
	fclose(diskFile);
}

int physicalAddr(int index, int i){

	FILE* diskFile = fopen(DISKFILE.c_str(), "rb");

	iNode in;

	//retrieve the inode
	fseek (diskFile, SB.blkSize*3 + SB.blkSize * i, SEEK_SET);

	if( index < 12 ){

		return in.directPointers[index];
	}

	else if( index < 12 + (SB.blkSize / 4) ){

	}

	return 0;
}

void deleteFile(string filename) {

		FILE* diskFile = fopen(DISKFILE.c_str(), "rb"); // read in binary mode
		fseek (diskFile, SB.blkSize*3, SEEK_CUR); // iNodes begin at 3 blocks down

		iNode in;
		int index=0;

		for(int i=0; i<256; i++) {
			fseek (diskFile, SB.blkSize*3+sizeof(iNode)*i, SEEK_SET); // move to the ith iNode
			fread(&in, sizeof(iNode), 1, diskFile);

			// compares char array and string
			if(strcmp(in.fileName, filename.c_str()) == 0) {
				index = i;
				break;
			}
		}

		int* idPtrs = new int [SB.blkSize/sizeof(int)];

		// Free data blocks that the pointers in the indirect pointer points to
		for(int i=0; i<12; i++) {
			if(in.directPointers[i] != -1) {
				blockList[i] = false;
			}
		}

		iNodeMap[index] = false;

		
}
void shutdown() {

	FILE* diskFile =  fopen(DISKFILE.c_str(), "rb+wb");
	if(diskFile!=NULL) {

		fseek (diskFile, SB.blkSize, SEEK_SET); // move 1 block over
		fwrite(iNodeMap, sizeof(bool), 256, diskFile);

		cout << "iNodeMap before shut down" << endl;
		for(int i=0; i<sizeof(iNodeMap)/sizeof(bool); i++) {
			cout << i << ": " << iNodeMap[i] << endl;
		}

		fseek (diskFile, SB.blkSize*2, SEEK_SET); // move 2 blocks over
		fwrite(blockList, sizeof(bool), (SB.numBlks - 253), diskFile);
	}

	fclose(diskFile);

	exit(0);

}

void list(){

	cout << "-------------------------listing files---------------------------" << endl;
	FILE* diskFile = fopen(DISKFILE.c_str(), "rb"); // read in binary mode
	iNode n;

	for(int i =0; i <256; i++) {
		fseek (diskFile, SB.blkSize*3+sizeof(iNode)*i, SEEK_SET); // iNodes begin at 3 blocks down
		
		fread(&n, sizeof(iNode), 1, diskFile);
		if(iNodeMap[i]) 
			printf("Filename: %s File Size: %d\n", n.fileName, n.fileSize);
		//i++;
	}
	fclose(diskFile);
	
}


void cat(string filename) {
	cout<<"\n--------------CAT---------------------------\n"<<endl;
	FILE* diskFile = fopen(DISKFILE.c_str(), "rb"); // read in binary mode

	iNode in;

	for(int i=0; i<256; i++) {
		fseek (diskFile, SB.blkSize*3 + sizeof(iNode)*i, SEEK_SET); // move to the ith iNode
		fread(&in, sizeof(iNode), 1, diskFile);

		// compares char array and string
		if(strcmp(in.fileName, filename.c_str()) == 0) {
			break;
		}
	}

	read(filename, 0, in.fileSize);
}

void write(string filename, char ch, int startByte, int numBytes) {

	FILE* diskFile = fopen(DISKFILE.c_str(), "rb+wb"); // read in binary mode

	int iNodeIndex;
	iNode in;

	for(int i=0; i<256; i++) {
		fseek (diskFile, SB.blkSize*3+ sizeof(iNode)*i, SEEK_SET); // move to the ith iNode
		fread(&in, sizeof(iNode), 1, diskFile);

		// compares char array and string
		if(strcmp(in.fileName, filename.c_str()) == 0) {
			iNodeIndex = i;
			break;
		}
	}

	bool finished = false;

	/*

	Find where the start byte is

	*/

	int blockIndex; // index into array of direct pointers
	int blockPtr;
	char* buffer = new char [SB.blkSize]; // data block content
	int location = 0; // where we are in the file (within a data block; resets to 0 when we go to a new data block)
	int bytesWritten = 0;

	blockIndex = startByte/SB.blkSize;
	blockPtr = in.directPointers[blockIndex];


	if(startByte < SB.blkSize*12) { // startByte is within the direct pointers

		while(bytesWritten < numBytes && blockIndex < 12) {

		// need to allocate data block
		if(blockPtr == -1) {
			cout << "need a new block! " << endl;
			for(int i=0; i<sizeof(blockList)/sizeof(bool); i++) {
				if(blockList[i] == false) { // obtain a free block!
					blockList[i] = true;
					in.directPointers[blockIndex] = i;
					blockPtr = i;
					cout << "blockPtr in new block: " << blockPtr << endl;
					break;
				}
			}
		}

			 // read in binary mode

			fseek (diskFile, SB.blkSize*3+sizeof(iNode)*256, SEEK_SET); // where the data blocks start

			fseek(diskFile, SB.blkSize*blockPtr, SEEK_CUR); // where startByte starts
			
			fread(buffer, sizeof(char), SB.blkSize, diskFile); // extracts entire data block

			location = startByte;

			for(int i=0; i<numBytes; i++) {

				buffer[location] = ch; // char

				in.fileSize++;
				bytesWritten++;
				if(bytesWritten == numBytes) break;
				location++;

				if(location == SB.blkSize) {
				

					 // write in binary mode
					fseek (diskFile, SB.blkSize*3+sizeof(iNode)*256,SEEK_SET); // where the data blocks start
					fseek(diskFile, SB.blkSize*blockPtr,SEEK_CUR); // where startByte starts
					fwrite(&buffer, sizeof(char), SB.blkSize, diskFile); // write entire data block

					blockIndex++;
					blockPtr = in.directPointers[blockIndex];
					cout << "blockPtr: " << blockPtr << endl;
					location = 0;
					break; // break out of current data block
				}
			}
		} // while end
		
		cout << "bytesWritten : " << bytesWritten << endl;


		if(bytesWritten == numBytes) {
			cout << "closed file bytesWritten == numBytes " << endl;
			fseek (diskFile, SB.blkSize*3+sizeof(iNode)*256, SEEK_SET); // where the data blocks start
			fseek(diskFile, SB.blkSize*blockPtr,SEEK_CUR); // where startByte starts
			for(int c = 0; c<SB.blkSize; c++)
			{
				cout<<buffer[c]<<" ";
			}
			cout<<endl;
			fwrite(buffer, sizeof(char), SB.blkSize, diskFile); // write entire data block

			finished = true;
		}

		fclose(diskFile);
	} else { // need indirect pointer ug
			diskFile = fopen(DISKFILE.c_str(), "rb+wb"); // write in binary mode

			// check if this file does not have an indirect pointer
			// if no... must allocate a data block that holds direct pointers
			if(in.indirectPointer == -1) {

				// find a free data block
				for(int i=0; i<sizeof(blockList)/sizeof(bool); i++) {
					if(blockList[i] == false) {
						blockList[i] = true;
						in.indirectPointer = i;
						break;
					}
				}

				int* dataBlockInit = new int [SB.blkSize/sizeof(int)];

				fseek (diskFile, SB.blkSize*3 + ceil(sizeof(iNodes)/SB.blkSize), SEEK_SET); // where the data blocks start
				fseek(diskFile, SB.blkSize*in.indirectPointer,SEEK_SET); // the data block that the indirect pointer

				// fill data block w/ -1
				for(int i=0; i<SB.blkSize/sizeof(int); i++) {
					dataBlockInit[i] = -1;
				}
				fwrite(dataBlockInit, sizeof(int), SB.blkSize/sizeof(int), diskFile);

			} // if no indirect pointer end

			fclose(diskFile); // exit write mode

			diskFile = fopen(DISKFILE.c_str(), "rb"); // read in binary mode
			location = 0; // reset location
			fseek (diskFile, SB.blkSize*3 + ceil(sizeof(iNodes)/SB.blkSize), SEEK_SET); // where the data blocks start
			fseek(diskFile, SB.blkSize*in.indirectPointer,SEEK_SET); // the data block that the indirect pointer

			int* dataBlockPtrs = new int [SB.blkSize/sizeof(int)];
			fread(dataBlockPtrs, sizeof(int), SB.blkSize/sizeof(int), diskFile);

			blockIndex = 0;

			while(bytesWritten < numBytes || blockIndex < SB.blkSize/sizeof(int)) {

					if(dataBlockPtrs[blockIndex] == -1) {
						// must find a free block!
						for(int i=0; i<sizeof(blockList)/sizeof(bool); i++) {
							if(blockList[i] == false) {
								blockList[i] = true;
								dataBlockPtrs[blockIndex] = i;
								break;
							}
						}
					}

					fseek (diskFile, SB.blkSize*3 + ceil(sizeof(iNodes)/SB.blkSize), SEEK_SET); // where the data blocks start
					fseek(diskFile, SB.blkSize*dataBlockPtrs[blockIndex],SEEK_SET);
					fread(&buffer, sizeof(char), SB.blkSize, diskFile); // extracts entire data block

					for(int i=0; i<numBytes; i++) {
						buffer[i] = ch; // char
						bytesWritten++;
						location++;

						if(location == SB.blkSize) {
							fclose(diskFile);

							diskFile = fopen(DISKFILE.c_str(), "rb+wb"); // write in binary mode
							fseek (diskFile, SB.blkSize*3 + ceil(sizeof(iNodes)/SB.blkSize), SEEK_SET); // where the data blocks start
							fseek(diskFile, SB.blkSize*dataBlockPtrs[blockIndex],SEEK_SET); // where startByte starts
							fwrite(&buffer, sizeof(char), SB.blkSize, diskFile); // extracts entire data block

							fclose(diskFile);

							blockIndex++;
							location = 0;
						}
					}

			}
		}


	/*

		Write iNode to disk

	*/
	
	if(finished) {
		
		diskFile = fopen(DISKFILE.c_str(), "rb+wb"); // read in binary mode
		fseek (diskFile, SB.blkSize*3, SEEK_SET); // iNodes begin at 3 blocks down
		fseek (diskFile, SB.blkSize*iNodeIndex, SEEK_CUR);
		fwrite(&in, sizeof(iNode), 1, diskFile);

		fclose(diskFile);
	}


	
}

void read(string filename, int startByte, int numBytes) {

	cout<<"\n------------------------READING------------------------\n"<<endl;
	
	FILE* diskFile = fopen(DISKFILE.c_str(), "rb+wb"); // read in binary mode

	int found = false; // if file exists
	iNode in;

	for(int i=0; i<256; i++) {
		fseek (diskFile, SB.blkSize*3 + sizeof(iNode)*i, SEEK_SET); // move to the ith iNode
		fread(&in, sizeof(iNode), 1, diskFile);

		// compares char array and string
		if(strcmp(in.fileName, filename.c_str()) == 0) {			
			found = true;
			break;
		}
	}

	if(!found) {
		cout << "File was not found" << endl;
		return;
	}

	
	int blockIndex; // index into array of direct pointers
	int blockPtr;
	char* buffer = new char [SB.blkSize]; // data block content
	int location = 0; // where we are in the file (within a data block; resets to 0 when we go to a new data block)
	int bytesRead = 0;

	blockIndex = startByte/SB.blkSize;
	blockPtr = in.directPointers[blockIndex];


	if(startByte < SB.blkSize*12) { // startByte is within the direct pointers

		while(bytesRead < numBytes && blockIndex < 12) {

			 // read in binary mode

			fseek (diskFile, SB.blkSize*3+sizeof(iNode)*256, SEEK_SET); // where thegzi data blocks start
			fseek(diskFile, SB.blkSize*blockPtr, SEEK_CUR); // where startByte starts
			
			fread(buffer, sizeof(char), SB.blkSize, diskFile); // extracts entire data block
			

			for(int i=0; i<numBytes; i++) {

				cout << buffer[location];

				in.fileSize++;
				bytesRead++;
				if(bytesRead == numBytes) break;
				location++;

				if(location == SB.blkSize) {
				
					fwrite(&buffer, sizeof(char), SB.blkSize, diskFile); // write entire data block
 
					blockIndex++;
					blockPtr = in.directPointers[blockIndex];
					location = 0;
					break; // break out of current data block
				}
			}
		} // while end

		cout << endl;

		if(bytesRead == numBytes) {
			cout << "closed file bytesWritten == numBytes " << endl;
			
			return;
		}
}}

void import(string filename, string unixFilename) {
	cout << "In import function" << endl;

	// check if file does not exist	
	FILE* diskFile = fopen(DISKFILE.c_str(), "rb+wb"); // read in binary mode

	int found = false; // if file exists
	iNode in;

	for(int i=0; i<256; i++) {
		fseek (diskFile, SB.blkSize*3 + sizeof(iNode)*i, SEEK_SET); // move to the ith iNode
		fread(&in, sizeof(iNode), 1, diskFile);

		// compares char array and string
		if(strcmp(in.fileName, filename.c_str()) == 0) {
			found = true;
			break;
		}
	}

	if(!found) {
		create(filename);
	}

	/*

	Get iNode of this file

	*/

	int iNodeIndex;

	for(int i=0; i<256; i++) {
		fseek (diskFile, SB.blkSize*3 + sizeof(iNode)*i, SEEK_SET); // move to the ith iNode
		fread(&in, sizeof(iNode), 1, diskFile);

		// compares char array and string
		if(strcmp(in.fileName, filename.c_str()) == 0) {
			iNodeIndex = i;
			break;
		}
	}
	
	int startByte = 0;

	// open unix file
	ifstream fileStream(unixFilename);

	char c; // char buffer
	//reads unix file char by char and use write function to write each char one at a time
	cout << "\n------------------------writing to file in import----------------------\n" << endl;
	while(fileStream.get(c)) {
		cout << c << endl;
		write(in.fileName, c, startByte, 1);
		startByte++;
	}

	fileStream.close();
	
	
}







