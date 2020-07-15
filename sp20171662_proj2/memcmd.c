#include "20171662.h"

void print_mem(int start, int end){
	int i,j;
	int start_flag = start;

	//memory address (ex. 00070)
	printf("%04X0 ",start_flag/16);

	//if the start address is not _0, put blank space for addresses before
	if (start_flag%16 != 0){
		for (i=0;i<start_flag%16;i++) printf("   ");
	}//end of if

	//printing out the middle column: memory values
	for(i=start_flag;i<=end;i++){
		printf("%02X ",my_mem[i]);
		if(i%16==15){//if the address is pointing at the last address of the line
			printf("; ");
			//the rightmost column: ASCII code for each of the memory values
			for(j=i-15;j<=i;j++){
				//if the value is not in range 20~7E or if it's before the start address, print '.'
				if(j<start_flag || my_mem[j]==0 || my_mem[j]<0x20 || my_mem[j] > 0x7E) printf(".");
				else printf("%c",my_mem[j]);
			}//end of j loop
			printf("\n");
			//show 5-digit memory address untill it reaches the last line
			if(i+1<=end)printf("%04X0 ",(i+1)/16);
		}//end of if

	}//end of i loop

	//if the end address did not reach the rightmost address
	if(end%16!=15){
		//fill the blank space
		for(i=(end%16)+1;i<16;i++) printf("   ");
		printf("; ");

		//the rightmost column
		for(j=(end/0x10)*0x10;j<=end;j++){
			if(j<start_flag || my_mem[j]==0 || my_mem[j]<0x20 || my_mem[j]>0x7E) printf(".");
			else printf("%c",my_mem[j]);
		}
		//print '.' for the addresses out of given range
		for(j=end+1;j%16!=0;j++) printf(".");
		printf("\n");
	}
}

void onlydump() {
	//print 10 lines

	//if start/has to start from start
	if(end_index==0xFFFFF){
		print_mem(0,159);
		end_index=159;
	}


	//if out of boundary, print only to 0xFFFFF
	else if(end_index+160>0xFFFFF) {
		print_mem(end_index+1,0xFFFFF);
		end_index=0xFFFFF;
	}

	else {
		print_mem(end_index+1,end_index+160);
		end_index += 160;
	}
}

void dumpstart(int start){
	if(start+160-1 > 0xFFFFF) print_mem(start,0xFFFFF);
	else print_mem(start, start+160-1);
}

void dumpstartend(int start, int end){
	print_mem(start,end);
}

void editvalue(int address, int value){
	my_mem[address] = value;
}

void fillvalue(int start, int end, int value){
	int i;
	for(i=start;i<=end;i++){
		my_mem[i]=value;
	}
}

void reset_mem(){
	memset(my_mem,0,MEM_SIZE);
}
