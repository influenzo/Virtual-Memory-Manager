// Matthew Selva & Lorenzo DeSimone - Virtual Memory Manager

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>


#define MaxMemory 64 // Maximum amount of system memory.
#define MaxProc 4    // Maximum number of processes.
#define MaxPage 4    // Maximum number of pages 

unsigned char memory[MaxMemory];

// Global Objects for user input
FILE *disk;
int freeList[MaxProc];
int lastEvict = 0;
int pids[MaxProc];
int perm[MaxProc][MaxPage];    // Handles readable/writable status
int isValid[MaxProc][MaxPage]; // Indicates if an address has been mapped
int isOnDisk[MaxProc][MaxPage + 1];
char* splitInput[100];


// Function prototypes to prevent compile warnings
int evict(int pid);
int swapToDisk(int page, int fromLine);


// |====================================| Helper Functions |====================================| \\

// findPage: returns the corresponding page nunmber of a Virtual Address
int findPage(int addr) {
    	if (addr >=  0 && addr < 16) return 0;
    	if (addr >= 16 && addr < 32) return 1;
    	if (addr >= 32 && addr < 48) return 2;
    	if (addr >= 48 && addr < 64) return 3;
}

// findAddr: returns the lowest address on a Page
int findAddr(int page) {
    	if (page == 0) return 0;
	if (page == 1) return 16;
    	if (page == 2) return 32;
    	if (page == 3) return 48;
}

// write: writes a value to memory
int write(int start, char* val) {
	int bytes = strlen(val);
    	int isRoom = start%16;
    	int count = 0;

    	if (isRoom + bytes >= 15) {
        	return -1;
    	} for(int i = start; i < (16 + start); i++) {
       		if(val[i - start] != 0){
            		memory[i] = val[i - start];
            		count = i;
        	} else break;
    	}
    	return (count - start);
}

// storeDisk: puts a page on the disk.txt file
int storeDisk(char page[16]) {
    	int line = -1;
    	char curr;
    	int count = 0;

	// Opening the 'disk' text file to update values, checking for NULL disk
    	disk = fopen("disk.txt", "r+");
    	if(disk == NULL) {
        	printf("ERROR: Cannot open disk in storeDisk.\n");
        	return -1;
    	} do {
        	curr = fgetc(disk);
            	count++;
		
		// If the end of the file is reached, write the page to the disk
        	if(feof(disk)) {
        		line++;
            		for(int i = 0; i < 16; i++) {
                		fputc(page[i], disk);
            		}
            		fputc('\n', disk);
            		break;
        	} else {

			// If the end of the line is reached, write to the next time
            		if(curr == '!' && count == 16) {
            		line++;
                	fseek(disk, -16, SEEK_CUR);
                		for(int i = 0; i < 16; i++) {
                    			fputc(page[i], disk);
                		} break;
            		} else if(count == 16) {
                		line++;
                		count = 0;
    	}}}

    	while(curr != EOF);
    	fclose(disk);
    	return line;
}

// readMem: reads memory to return a value
int readMem(int start) {
    	int val;
    	char buf[4];

    	for(int i = 0; i < 4; i++) {
        	if( (memory[start + i] != '*') && (i != 3) ) {
            		buf[i] = memory[start + i];
        	} else {
            		buf[i]= '\0';
            		break;
    	}}

    	if(buf[0] == '\0') {
        	return -1;
    	} else {
        	sscanf(buf, "%d", &val);
        	return val;
}}

// getDisk: gets a page from the disk.txt file
int getDisk(char (*bookmark)[16], int fromLine) {
    	int line = -1;
    	char currChar;
    	int count = 0;

	// checking for an empty disk, returning an error if that is the case
    	disk = fopen("disk.txt", "r+");
    	if(disk == NULL) {
    		printf("ERROR: Cannot open disk in getDisk.\n");
        	return -1;
    	} do {
        	currChar = fgetc(disk);
            	count++;

		// Checking if the disk is empty, containing no pages
        	if(feof(disk) && line == -1) {
            		printf("ERROR: Cannot get page from empty disk.\n");
            		fclose(disk);
            		return -1;
        } else {
            	if(count == 16) {
                	line++;
            	}

		// Processes a line, replaces with '!'
            	if(line == fromLine && count == 16) {
                	fseek(disk, -16, SEEK_CUR);
                	for(int i = 0; i < 16; i++) {
                    		currChar = fgetc(disk);
                    		(*bookmark)[i] = currChar;
                    		fseek(disk, -1, SEEK_CUR);
                    		fputc('!', disk);
                }
                fputc('\n', disk);
                fclose(disk);
                return 0;

            	} else if(count > 16) {
                	count = 0;
    	}}}

    	while(currChar != EOF);
    	fclose(disk);
    	return -1;
}

// StringSplitter: breaks User Input down into commands separated by commas
char** StringSplitter(char str[]) {
	char* dst = (char *) malloc(strlen(str) + 1);
	strcpy(dst, str);
		
	char* tok = strtok(dst, ",");
	int i = 0;	
	while (tok != NULL) {
		i++;
		splitInput[i-1] = tok;
		tok = strtok(NULL, ",");
	}
	return splitInput;
}

// VtoP: takes in a Virtual Address and returns its cooresponding Physical Address
int VtoP(int pid, int virt){
    	int phys = -1;
    	int Vpage = findPage(virt);
    	int offset = Vpage*16;
    	int start = pids[pid];
    	int cur = start;

    	for (int i = 0; i < 16; i++) {
        	if(memory[cur] == ','){
            		if(memory[cur - 1] - '0' == findPage(virt)){
                		return findAddr(memory[cur + 1] - '0') + virt - offset;
        }}
        	cur++;
    	}
    	return phys;
}

// createTable: creates table of virtual pages
int createTable(int pid) {
    	int isFree = -1;

	// creating a page table
    	for(int i = 0; i < 4; i++) {
        	if (freeList[i] == -1) {
            		freeList[i] = 0;
            		pids[pid] = findAddr(i);
            		isFree = 1;
            		printf("Put page table for PID %d into physical frame %d\n", pid, i);
            		break;
        }}

	// if the current page table needs to be swapped for a new one
    	if(isFree == -1) {
        	int evicted = evict(pid);
        	swapToDisk(evicted, -1);
        	int Ppage = lastEvict;
        	pids[pid] = findAddr(Ppage);
        	printf("Put page table for PID %d into physical frame %d\n", pid, Ppage);
}}


// |====================================| Main Functions |====================================| \\

// map: handles physical page allocation and the readable/writeable status of Virtual Addresses
int map(int pid, int Vaddr, int val) {
    	int Vpage = findPage(Vaddr);
    	int Ppage;
    	int table = pids[pid];
    	int isFree = -1;
    	char fullAddr[16] = "";
    	char newEntry[10];
    	char buff[10];

    	// If there isn't a page table, make one
    	if (table == -1 && isOnDisk[pid][0] == -1) createTable(pid);

    	// If the page exists, but needs to be updated
    	if (isValid[pid][Vpage] == 1) {
        	if (perm[pid][Vpage] == val) printf("ERROR: virtual page %d is already mapped with rw_bit=%d\n", Vpage, val);
        	else {
            		printf("Updating permissions for virtual page %d (frame %d)\n", Vpage, findPage(VtoP(pid, findAddr(Vpage))));
            		perm[pid][Vpage] = val;

	// Mapping an address in memory, indicating that the address is a valid and readable by default
        }} else {
        	for(int i = 0; i < 4; i++) {
            		if (freeList[i] == -1) {
                		freeList[i] = 0;
                		perm[pid][Vpage] = val;
                		isValid[pid][Vpage] = 1;
                		isFree = 1;
                		Ppage = i;
                		int writeTo = pids[pid];

				// Writing to the disk file
                		for(int j = 0; j < 16; j++) {
                    			if (memory[writeTo] == '*') {
                       				sprintf(buff, "%d", Vpage);
                        			strcat(fullAddr, buff);
                        			strcat(fullAddr, ",");
                        			sprintf(buff, "%d", Ppage);
                        			strcat(fullAddr, buff);
                        			writeTo += write(writeTo, fullAddr);
                        			break;
                    			}
                    			writeTo++;
                		}
                		printf("Mapped virtual address %d (page %d) into physical frame %d\n", Vaddr, Vpage, Ppage);
                		break;
        	}}

		// If the address is being swapped out
        	if (isFree == -1) {
            		int evicted = evict(pid);
            		swapToDisk(evicted, -1);
            		perm[pid][Vpage] = val;
            		isValid[pid][Vpage] = 1;
            		isFree = 1;
            		Ppage = lastEvict;
            		int writeTo = pids[pid];

			// Writing to the disk file
            		for(int j = 0; j < 16; j++) {
                		if(memory[writeTo] == '*') {
                    			sprintf(buff, "%d", Vpage);
                    			strcat(fullAddr, buff);
                    			strcat(fullAddr, ",");
                    			sprintf(buff, "%d", Ppage);
                    			strcat(fullAddr, buff);
                    			writeTo += write(writeTo, fullAddr);
                    			break;
                		}
                		writeTo++;
            		}
            		printf("Mapped virtual address %d (page %d) into physical frame %d\n", Vaddr, Vpage, Ppage);
    }}
    return 0;
}

// mapFix: changes the address of a page that is swapped
int mapFix(int pid, int Vpage, int Ppage) {
	int isFree = -1;
	int writeTo = pids[pid];
	char fullAddr[16] = "";
	char buff[10];

	for(int i = 0; i < 16; i++) {
		if(memory[writeTo] == ',') {
			if(memory[writeTo - 1] == Vpage) {
				memory[writeTo+1] = Ppage;
		}}
		writeTo++;
	}
	printf("Remapped virtual page %d into physical frame %d\n", Vpage, Ppage);
	return 0;
}

// store: instructs the memory manager to write the given value into the physical memory location associated with the provided virtual address, performing translation and page swapping as necessary
int store(int pid, int Vaddr, int val) {
	char buff[10] = "";

	// Checking if the process needs to be swapped to disk
	if (isOnDisk[pid][0] != -1) { 
		int evicted = evict(pid);
		swapToDisk(evicted, isOnDisk[pid][0]);
	}

	// Storing the value in the memory array
	int Paddr = VtoP(pid, Vaddr);
	int Vpage = findPage(Vaddr);
	if (perm[pid][Vpage] == 1) {
		if (isValid[pid][Vpage] == 1) {
			if (isOnDisk[pid][Vpage+1] != -1) {
				int evicted = evict(pid);
				swapToDisk(evicted, isOnDisk[pid][Vpage + 1]);
			}
			sprintf(buff, "%d", val);
			int bytesWritten = write(Paddr, buff);

			if(bytesWritten == -1)  printf("\nERROR: Too many bytes written, value cannot be stored.\n");
			else printf("\nStored value %d at virtual address %d (physical address %d)\n", val, Vaddr, Paddr);
		
		} else printf("\nERROR: Virtual page %d has not been allocated for process %d!\n", Vpage, pid);

	} else printf("\nERROR: Virtual address %d is not writeable\n", Vaddr);

	return 0;
}

// load: instructs the memory manager to return the byte stored at the memory location specified by the virtual address
void load(int pid, int Vaddr) {
	int Vpage = findPage(Vaddr);

	// Swapping the process
	if (isOnDisk[pid][0] != -1) {
		int evicted = evict(pid);
		swapToDisk(evicted, isOnDisk[pid][0]);
		isOnDisk[pid][0] = -1;
	}

	// Swapping the page table entry
	if(isOnDisk[pid][Vpage + 1] != -1) {
        	int evicted = evict(pid);
        	swapToDisk(evicted, isOnDisk[pid][Vpage + 1]);
	}

	// The process of loading and printing
   	int Paddr = VtoP(pid, Vaddr);
	int val = readMem(Paddr);
	if(val == -1) printf("\nERROR: No value stored at virtual address %d (physical address %d)\n", Vaddr, Paddr);
	else printf("\nThe value %d is at virtual address %d (physical address %d)\n", val, Vaddr, Paddr);
}

// evict: evicts a physical page based on round robin selection
int evict(int pid){
	int table = findPage(pids[pid]);
	int nextEvict = lastEvict + 1;

	if (nextEvict >= 4)     nextEvict=0;
	if (nextEvict == table) nextEvict++;
        if (nextEvict >= 4)     nextEvict=0;

	lastEvict = nextEvict;
	return nextEvict;
	}

// swapToDisk: moves a page table entry from memory to disk
int swapToDisk(int page, int fromLine) {
	int start = findAddr(page);
	char pTemp[16];
	char gTemp[16];
	int repMem = -1;
	int putLine = -1;
	int flag = -1;

	// Intializing the process ID list according to the spage specified by the user at fuction call
	for(int i = 0; i < MaxProc; i++) {
		if(start == pids[i]) {
			pids[i] = -1;
			flag = i;
			break;
	}}

	// Initializing an array with changed values from memory
	for(int i = 0; i < 16; i++) {
		pTemp[i] = memory[start + i];
	}

	// When swapping is being done, write values from the disk to the memory that is being replaced
	if(fromLine != -1) {
		repMem = getDisk(&gTemp, fromLine);
		freeList[page] = 0;
	}

	// If nothing has been stored, error. Else, swap accordingly
	putLine = storeDisk(pTemp);
	if(putLine == -1) {
        	printf("ERROR: Could not put page to disk.\n");
        	return -1;
	} else if(repMem != -1) {
		printf("Swapped disk slot %d into frame %d\n", fromLine, findPage(start));
        	for(int i = 0; i < 16; i++) {
            		memory[start+i] = gTemp[i];
	}} else {
		for(int i = 0; i < 16; i++) {
			memory[start+i]='*'; 
	}}

	// Swapping and indicating that a process is in memory, not on the disk
	if(fromLine != -1) {
		for(int i = 0; i < MaxProc; i++) {
			for(int j = 0; j < MaxProc + 1; j++) {
				if(isOnDisk[i][j] == fromLine) {
					isOnDisk[i][j] = -1;
					if(j != 0) {
						mapFix(i, j - 1, page);
					} else if(j==0&&flag==-1) {
						pids[i] = start;
					} break;
	}}}}

	// Remapping by initializaing removal values
 	if(flag != -1) {
        	int rmPID = -1;
        	int rmVpage = -1;
        	for (int i = 0; i < MaxProc; i++) {
            		if(pids[i] != -1) {
                		int curAddr = pids[i];
                		for(int j = 0; j < 16; j++) {
                    			printf("%c", memory[curAddr]);
                    			if(memory[curAddr] == ',') {
                        			if(memory[curAddr + 1] - '0' == page) {
                            				rmPID = i;
                            				rmVpage = memory[curAddr - 1] - '0';
					}}
                    		curAddr++;
		}}}

		// Remapping a page table entry when the process is remapped
	    	int curTable = -1;
	    	if(rmPID == -1) {
	        	for(int i = 0; i < MaxProc; i++) {
	            		curTable=isOnDisk[i][0];
	            		if(curTable != -1){
	                		repMem = getDisk(&gTemp, curTable);
	                		for(int j = 0; j < 16; j++) {
	                    			if(gTemp[j] == ',') {
	                        			if(gTemp[j + 1] - '0' == page) {
	                            				rmPID = j;
	                            				rmVpage = gTemp[j - 1] - '0';
				}}}}
	            		if (rmPID != -1) break;
		}}

		// Checking if the indicated page exists
		if(rmPID != -1 && rmVpage != -1) {
	        	isOnDisk[rmPID][rmVpage + 1] = putLine;
		} else {
	        	printf("ERROR: Page that is swapped out doesn't exist?\n");
		}} else {
			isOnDisk[flag][0]=putLine;
	}

	printf("Swapped frame %d to disk at swap slot %d\n", page, putLine);
	if (fromLine!=-1) printf("Swapped disk slot %d into frame %d\n", fromLine, page);
	if (flag!=-1) printf("Put page table for PID %d into swap slot %d\n", flag, putLine);
	return putLine;
}


int main(int argc, char *argv[]){
	char** temp; // Temporary array to handle input
	char* input = (char*)malloc(sizeof(char*));
	int pid = 0;
	char* inst = (char*)malloc(sizeof(char*));
	int Vaddr = 0;
	bool isValidin = true;

	// Cleaning the disk
	disk = fopen("disk.txt", "w");
	if(disk == NULL) {
		printf("ERROR: Cannot open disk in main.");
		return -1;
	} else fclose(disk);

	// Initialize the page table, free and write lists
	for (int i = 0; i < MaxProc; i++){
        	pids[i] = -1;
        	freeList[i] = -1;
        	for (int j = 0; j < MaxPage + 1; j++){
			if (j < 4){
				perm[i][j] = 0;
				isValid[i][j] = 0;
			}
			isOnDisk[i][j] = -1;
		}}

	// Initialize physical memory
	for (int i = 0; i < MaxMemory; i++) {
		memory[i] = '*';
	}

	while(1) {
		printf("\n===============================================================\n");
        	printf("\nInstruction? "); 
        	
		if ((fgets(input, 15, stdin)==NULL)) {
			printf("End of file.\n");
			exit(1);
		}
		
		printf("\n");
		temp = StringSplitter(input);

		// Assign user input to variables
		int pid = atoi(temp[0]);
		inst = temp[1];
		int virt = atoi(temp[2]);
		int value = atoi(temp[3]);

		// Checking to make sure input is valid
		if (pid > 3 || pid < 0) {
			printf("\nERROR: %d is an Invalid Process ID.\n", pid);
			isValidin = false;	
		}

		if ( (strcmp(inst, "map") != 0) && (strcmp(inst, "store") != 0) && (strcmp(inst, "load") != 0) ) {
			printf("\nERROR: %s is an Invalid Instruction.\n", inst);
			isValidin = false;	
		}

		if (virt > 63 || virt < 0) {
			printf("\nERROR: %d is an Invalid Virtual Address.\n", virt);
			isValidin = false;
		}

		if (value > 255 || value < 0) {
			printf("\nERROR: %d is an Invalid Value.\n", value);
			isValidin = false;
		} 

		// Run functions if input is valid
		if(isValidin) {
			if(strcmp("map", inst) == 0)   map(pid, virt, value);
			if(strcmp("store", inst) == 0) store(pid, virt, value);
			if(strcmp("load", inst) == 0)  load(pid, virt);
}}}
