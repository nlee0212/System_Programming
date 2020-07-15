#include "20171662.h"

//available commands, has to be printed when h[elp]
char helpprint[] = "h[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end]\ne[dit] address, value\nf[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n";

int main(){
	int i;
	//flag to check for too many arguments
	int flag=0;
	//size of command (no. of arguments, ex. dump 4, 2E: size=3)
	int size = 1;
	//for saving address from command dump
	int startnum, endnum;
	//for saving input itself for history (char* input will be changed due to strtok)
	char* save_input = (char*)malloc(sizeof(char)*100);

	//to start with 0 when dump is first called
	end_index = 0xFFFFF;

	//initializing head node for history
	head = (Node*)malloc(sizeof(Node));
	head->next = NULL;
	
	//initializing hashTable for opcode
	hashTable = (Opcode**)malloc(sizeof(Opcode*)*TABLE_SIZE);
	init();

	//setting all memory to 0
	memset(my_mem,0,MEM_SIZE);

	//importing opcode.txt
	FILE* fp;
	char opcode_input[81];
	char* mod_opinput[3];
	fp = fopen(OPCODE,"r");
	while(fgets(opcode_input,80,fp)!=NULL){
			
		//tokenize opcode
		mod_opinput[0]=strtok(opcode_input," \n\t");
		mod_opinput[1]=strtok(NULL," \n\t");
		mod_opinput[2]=strtok(NULL," \n\t");
				
		//generate new node & initialize
		Opcode* op_mem = (Opcode*)malloc(sizeof(Opcode));
		op_mem->opcodenum = strtol(mod_opinput[0],NULL,16);
		strcpy(op_mem->instruction_name,mod_opinput[1]);
		strcpy(op_mem->arg_num,mod_opinput[2]);
				
		//setting the index into mod 20
		int index = (int)(op_mem->instruction_name[0])%20;
		add(op_mem,index);
		
	}//end of file reading
	fclose(fp);

	while(1){
		flag = 0;
		//allocate memory for the input variable
		input = (char*)malloc(sizeof(char)*100);
		printf("sicsim> ");
	
		fflush(stdin);
		fgets(input,100,stdin);
		//printf("%s",input);
		if(strlen(input)>100) printf("ERROR: INPUT TOO LONG");
		if(input[0]=='\n') continue;
		
		//removing white space
		size = 1;
		for(i=0;;i++){
			//returns the first token, adds '\0' at the end of the token at the string
			if (i==0)modified_input[i] = strtok(input," \n\t");
			else modified_input[i] = strtok(NULL," \n\t");

			//if too many arguments
			if (i>4) {
				printf("ERROR: TOO MANY ARGUMENTS\n");
				flag=1;
				break;
			}
			
			//if end of input	
			if(!modified_input[i]) break;
		}
		size = i;
		if(flag) continue;

		//for checking ','
		if(size>=3){
			for(i=1;i<size-1;i++){
				if(modified_input[i][strlen(modified_input[i])-1]!=','){
					printf("ERROR: Invalid Input. See h[elp].\n");
					flag=1;
					break;
				}
				else modified_input[i][strlen(modified_input[i])-1]='\0';
			}
			if(flag) continue;
		}//end of checking for ','

		//saving the input in the right form
		strcpy(save_input,right_input_form(size,modified_input));
		
		if(size==1){
		//if q[uit]
		if(!strcmp("quit",modified_input[0]) || !strcmp("q",modified_input[0])) {
			FreeAll(head);
			destructor();
			break;
		}
		
		//if h[elp]
		else if(!strcmp("help",modified_input[0]) || !strcmp("h",modified_input[0])) {
			printf("%s",helpprint);
			insertNode(head, save_input);
			continue;
		}

		//if d[ir]
		else if(!strcmp("dir",modified_input[0]) || !strcmp("d",modified_input[0])) {
			printdir();
			insertNode(head, save_input);
			continue;
		}
	
		//if hi[story]
		else if(!strcmp("history",modified_input[0]) || !strcmp("hi",modified_input[0])){
			insertNode(head, save_input);
			ShowAll(head);
			continue;
		}
		}

		//if du[mp] [start, end]
		if(!strcmp("dump",modified_input[0]) || !strcmp("du",modified_input[0])){
			switch(size){
				case 1:
					//boundary check
					if(end_index > 0xFFFFF) end_index=0xFFFFF;
					onlydump();
					insertNode(head,save_input);
					break;
				case 2:
					//check if address is in hex form.
					if(isHex(modified_input[1])==-1){
						printf("ERROR: address not in hex form.\n");
						continue;
					}

					//change the starting address from char to hex
					startnum = strtol(modified_input[1],NULL,16);
					
					//boundary check
					if(startnum > 0xFFFFF || startnum < 0){
						printf("ERROR: Start Address out of range.\n");
						continue;
					}

					dumpstart(startnum);
					insertNode(head,save_input);
					break;
				case 3:
					//check if address is in hex form.
					if(isHex(modified_input[1])==-1 || isHex(modified_input[2])==-1){
						printf("ERROR: address not in hex form.\n");
						continue;
					}

					//change start&end address from char to hex
					startnum = strtol(modified_input[1],NULL,16);
					endnum = strtol (modified_input[2],NULL,16);
					
					//boundary check
					if(startnum > 0xFFFFF || endnum > 0xFFFFF || startnum < 0 || endnum < 0){
						printf("ERROR: Address out of range.\n");
						continue;
					}
					else if(startnum > endnum){
						printf("ERROR: Range ERROR\n");
						continue;
					}

					dumpstartend(startnum,endnum);
					insertNode(head,save_input);
					break;
				default:
					printf("ERROR! TOO MANY ARGUMENTS\n");
					continue;
					//flag = 1;

				continue;
			}//end of switch
		}//end of du[mp] [start, end]
		
		//if e[dit] address, value
		else if (!strcmp("edit",modified_input[0]) || !strcmp("e",modified_input[0])){
			if (size == 3){
				
				//check if address is in hex form.
				if(isHex(modified_input[1])==-1 || isHex(modified_input[2])==-1){
					printf("ERROR: Address or Value not in hex form.\n");
					continue;
				}

				//change address and value from char to hex
				int add = strtol(modified_input[1],NULL,16);
				int val = strtol(modified_input[2],NULL,16);

				//boundary check
				if (add<0 || add>0xFFFFF) {
					printf("ERROR: Address out of range.\n");
					continue;
				}
				
				//check value (00~FF)
				else if(val<0x00 || val>0xFF){
					printf("ERROR: Value out of range.\n");
					continue;
				}

				else {
					editvalue(add,val);
					insertNode(head,save_input);
					continue;
				}
			}

			else{
				printf("ERROR: Input has to be e[dit] address, value. See h[elp].\n");
				continue;
			}

		}//end of e[dit] address, value

		//if f[ill] start, end, value
		else if(!strcmp("f",modified_input[0]) || !strcmp("fill",modified_input[0])){
			if(size==4){
				//check if address or value is in hex form.
				if(isHex(modified_input[1])==-1 || isHex(modified_input[2])==-1 || isHex(modified_input[3])==-1){
					printf("ERROR: Address or Value not in hex form.\n");
					continue;
				}

				//change start, end address and value from char to hex
				int start = strtol(modified_input[1],NULL,16);
				int end = strtol(modified_input[2],NULL,16);
				int value = strtol(modified_input[3],NULL,16);
				
				//boundary check
				if(start<0 || start>0xFFFFF || end<0 || end>0xFFFFF || start>end){
					printf("ERROR: Address out of range.\n");
					continue;
				}

				//value boundary check
				else if(value<0x00 || value>0xFF){
					printf("ERROR: Value out of range.\n");
					continue;
				}

				else{
					fillvalue(start,end,value);
					insertNode(head,save_input);
					continue;
				}
			}//end of size==4
			
			else{
				printf("ERROR: 4 arguments needed. See h[elp].\n");
				continue;
			}

		}//end of f[ill] start, end, value

		//if reset
		else if(!strcmp("reset",modified_input[0]) && size==1){
			reset_mem();
			insertNode(head,save_input);
			continue;
		}

		//if opcode mnemonic
		else if(!strcmp("opcode",modified_input[0]) && size == 2){
			
			//setting key to mod 20 of the first alphabet of the opcode
			int key = (int)(modified_input[1][0])%20;
			int result_opcode = search(key,modified_input[1]);

			if(result_opcode == -1){
				printf("ERROR: CANNOT FIND OPCODE. See opcodelist.\n");
				continue;
			}

			else {
				printf("opcode is %X\n",search(key,modified_input[1]));
				insertNode(head, save_input);
				continue;
			}

		}//end of opcode mnemonic

		//if opcodelist
		else if(!strcmp("opcodelist",modified_input[0]) && size == 1){
			insertNode(head,save_input);	
			show();

		}//end of opcodelist


		//if it is not an available input
		else {
			printf("ERROR! Not an available input. see h[elp].\n");
			continue;
		}
	}

	return 0;
}


int isHex(char* hex_input){
	unsigned int i;
	//flag to check for non-hex num.
	int hex_flag=1;
	
	for(i=0;i<strlen(hex_input);i++){
		//if each character is 0~9 or a~f or A~F
		if((hex_input[i]>='0' && hex_input[i]<='9') || (hex_input[i]>='a' && hex_input[i]<='f') || (hex_input[i]>='A' && hex_input[i]<='F')) hex_flag=1;
		
		//not a hex num
		else{
			hex_flag=-1;
			return -1;
		}

	}

	return hex_flag;
}

//to save the correct form for each command
char* right_input_form(int size, char** m_input){
	char* ret_input = (char*)malloc(sizeof(char)*100);
	int i;
	ret_input[0]='\0';

	for(i=0;i<size;i++){
		strcat(ret_input,m_input[i]);
		if(i==0 && size>1) strcat(ret_input," ");
		if(i>=1 && size>i+1){
			strcat(ret_input,", ");
		}

	}

	return ret_input;

}
