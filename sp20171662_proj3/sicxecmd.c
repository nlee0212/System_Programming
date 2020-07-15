#include "20171662.h"

int assemble(char* filename){
	FILE*fp = fopen(filename,"r");
	unsigned int i;

	for(i=0;i<strlen(filename);i++){
		if(filename[i]=='.') break;
	}

	if(i==strlen(filename)){
		printf("%s is not an .asm file.\n",filename);
		return 0;
	}

	if(strcmp(filename+i,".asm")!=0){
		printf("%s is not an .asm file.\n",filename);
		return 0;
	}

	//if file does not exist
	if(!fp){
		printf("%s file does not exist.\n",filename);
		return 0;
	}

	if(!pass_1(filename)){
		remove("intermediate.txt");
		return 0;
	}
	
	else if(!pass_2("intermediate.txt")){
		remove("intermediate.txt");
		filename[i]='\0';
		strcat(filename,".lst");
		remove(filename);
		filename[i]='\0';
		strcat(filename,".obj");
		remove(filename);
		return 0;
	}

	remove("intermediate.txt");
	printf("Successfully assemble %s.\n",filename);

	return 1;
}

int pass_1(char* filename){
	FILE*fp = fopen(filename,"r");
	FILE*itmd = fopen("intermediate.txt","w");
	char asminput[101], input_save[101];
	char* tok_input[10];
	char opcode[30];
	int size=0;
	int i;
	int locctr = 0, locnow = 0;
	int err_line=5;
	int labelflag=0, baseflag=0, startflag=0;
	
	start_addr = 0;
	prog_len = 0;

	//if there is SYMTAB existing, free it and make a new one
	if(symhead->next!=NULL) {
		FreeSymbol();
		symhead=(SYMTAB*)malloc(sizeof(SYMTAB));
		symhead->next=NULL;
	}

	if(obhead->next!=NULL){
		FreeObj();
		obhead=(ObjectCode*)malloc(sizeof(ObjectCode));
		obhead->next=NULL;
	}

	//show filename
	fprintf(itmd,"%s\n",filename);

	//read first line
	while(fgets(asminput,100,fp)!=NULL){
		if(asminput[0]!='\n') break;
	}

	if(asminput==NULL){
		printf("ERROR: file is empty!\n");
		return 0;
	}
	
	//save input line for intermediate file
	strcpy(input_save,asminput);

	//remove white space
	for(i=0;;i++){
		if(i==0) tok_input[0] = strtok(asminput," \n\t");
		else tok_input[i] = strtok(NULL," \n\t");

		//end of one line
		if(tok_input[i] == NULL) break;
	}
	size = i;
	

	if(size>4 && tok_input[0][0]!='.'){
		printf("ERROR: Line %d.\n",err_line);
		return 0;
	}

	//if OPCODE = 'START'
	if(size>=2){
		if(!strcmp(tok_input[1],"START") && tok_input[0][0]!='.'){
			if(size == 3){
				//save #[OPERAND] as starting address
				start_addr = strtol(tok_input[2],NULL,16);
				//initialize LOCCTR to starting address
				locctr = start_addr;
				locnow = locctr;
				//write line to intermediate file
				fprintf(itmd,"%d\t%X\t%s",err_line,locnow,input_save);
				err_line+=5;
			}

			else{
				printf("ERROR: Line 5. 'START' line error.\n");
				return 0;
			}
		}//end if START

		else{
			fclose(fp);
			fp = fopen(filename,"r");
			locctr = 0;
			locnow = 0;
		}
	}

	else{
		//if no 'START', restart the file and skip 'START' process
		fclose(fp);
		fp = fopen(filename,"r");
		//initialize LOCCTR to 0
		locctr = 0;
		locnow = 0;
	}
	
	while(fgets(asminput,100,fp)!=NULL){
		if(asminput[0]=='\n') continue;

		//initialize flags
		labelflag=0;
		baseflag=0;
		locnow = locctr;
		
		//if this is a comment line
		if(asminput[0]=='.'){
			//write line to intermediate file
			asminput[0]='-';
			if(asminput[1]=='\n' || asminput[1]=='\0') {
				fprintf(itmd,"%d\t-2\t-\n",err_line);
				err_line+=5;
				continue;
			}
			strcpy(input_save,asminput);	
			fprintf(itmd,"%d\t-2\t-%s",err_line,input_save);
			err_line+=5;
			continue;
		}

		strcpy(input_save,asminput);

		//check if there is a symbol in the LABEL field
		if(asminput[0]!=' ' && asminput[0]!='\t') labelflag=1;
		
		//remove white space
		for(i=0;;i++){
			if(i==0) tok_input[0] = strtok(asminput," \n\t");
			else tok_input[i] = strtok(NULL," \n\t");
			
			//end of one line
			if(!tok_input[i]) break;
		}
		size = i;
		
		if(size>=2){
			if(tok_input[i-2][strlen(tok_input[i-2])-1]==',') strcat(tok_input[i-2],tok_input[i-1]);
		}

		if(size>4){
			printf("ERROR: Line %d. There are only 3 fields available.\n",err_line);
			return 0;
		}
		
		//if OPCODE = 'START'
		if(tok_input[1]!=NULL){
			if(!strcmp(tok_input[1],"START")){
				if(tok_input[0][0]=='.') break;
				//if START is in between valid codes
				if(startflag){
					printf("ERROR: Line %d. START in the middle.\n",err_line);
					return 0;
				}

				if(size == 3){
					startflag = 1;
					//save #[OPERAND] as starting address
					start_addr = strtol(tok_input[2],NULL,16);
					//initialize LOCCTR to starting address
					locctr = start_addr;
					locnow = locctr;
					//write line to intermediate file
					fprintf(itmd,"%d\t%X\t%s",err_line,locnow,input_save);
					err_line+=5;
				}

				else{
					printf("ERROR: Line %d. 'START' line error.\n",err_line);
					return 0;
				}

				continue;
			}		
		}//end if START

		//if OPCODE = END
		if(!strcmp(tok_input[0],"END")){
			//write last line to intermediate file
			fprintf(itmd,"%d\t%X\t-\t%s",err_line,locctr,input_save);
			//save (LOCCTR - starting address) as program length

			if(tok_input[1]==NULL){
				printf("ERROR: Line %d. END OPERAND field has to contain existing symbol.\n",err_line);
				return 0;
			}

			if(SearchSymbol(tok_input[1])==-1){
				printf("ERROR: Line %d. END OPERAND field has to contain existing symbol.\n",err_line);
				return 0;
			}

			prog_len = locctr - start_addr;
			fclose(fp);
			fclose(itmd);
			return 1;
		}

		startflag = 1;
		//if there is a symbol in the LABEL field
		if(labelflag){
			strcpy(opcode,tok_input[1]);
			//if found
			if(SearchSymbol(tok_input[0])>=0) {
				printf("ERROR: Line %d. Duplicated symbol.\n",err_line);
				return 0;
			}
			else insertSymbol(tok_input[0],locctr);
		}//end of if symbol
		else {
			memset(opcode,'\0',30);
			strcpy(opcode,tok_input[0]);
		}
		
		//if OPCODE found in OPCODE TABLE, add format number to LOCCTR
		if(search((int)opcode[0]%20,opcode)>=0){
			locctr+=searchformat((int)opcode[0]%20,opcode);
		}
		//if opcode has to be used in format 4
		else if(opcode[0]=='+'){
			if(search((int)opcode[1]%20,opcode+1)>=0) locctr+=4;
		}
		
		//if OPCODE = 'BASE'
		else if (!strcmp("BASE",opcode)) baseflag = 1;
		//if OPCODE = 'WORD'
		else if (!strcmp("WORD",opcode)) locctr+=3;
		//if OPCODE = 'RESW'
		else if (!strcmp("RESW",opcode)) {
			if(labelflag)locctr+=3*atoi(tok_input[2]);
			else locctr+=3*atoi(tok_input[1]);
		}
		//if OPCODE = 'RESB'
		else if (!strcmp("RESB",opcode)){
			if(labelflag) locctr+=atoi(tok_input[2]);
			else locctr+=atoi(tok_input[1]);
		}
		//if OPCODE = 'BYTE'
		else if (!strcmp("BYTE",opcode)){
			if(labelflag) i = 2;
			else i = 1;

			//if C(char)
			if(tok_input[i][0]=='C') {
				//remove C(1) and '(2)
				locctr+=strlen(tok_input[i])-3;

				//check '
				if(tok_input[i][strlen(tok_input[i])-1]!=39 || tok_input[i][1]!=39){
					printf("ERROR: Line %d. missing \'.\n",err_line);
					return 0;
				}

				//if character value starts with a number
				if(tok_input[i][2]>='0' && tok_input[i][2]<='9'){
					printf("ERROR: Line %d. C's first character cannot be a number.\n",err_line);
					return 0;
				}
			}

			//if X(hex)
			else if(tok_input[i][0]=='X') {
				//check '
				if(tok_input[i][strlen(tok_input[i])-1]!=39 || tok_input[i][1]!=39){
					printf("ERROR: Line %d. missing \'.\n",err_line);
					return 0;
				}

				//no. of half bytes = strlen() - X(1) - '(2) = strlen() - 3
				//if no. of half bytes is odd, ceil()
				if((strlen(tok_input[i])-3)%2==1) locctr+=(strlen(tok_input[i])-3)/2 + 1;
				else locctr+=(strlen(tok_input[i])-3)/2;
			}

			//if BYTE but not C or X
			else{
				printf("ERROR: Line %d. 'BYTE' can only have C and X.\n",err_line);
				return 0;
			}
		}//end of BYTE
		

		//invalid OPCODE
		else{
			printf("ERROR: Line %d. %s: Invalid operation code or empty OPERAND field.\n",err_line, opcode);
			return 0;
		}//end of if not comment

		if(!labelflag && !baseflag) {
			if(tok_input[1]!=NULL) fprintf(itmd,"%d\t%X\t-\t%s\t%s\n",err_line,locnow,tok_input[0], tok_input[1]);
			else fprintf(itmd,"%d\t%X\t-\t%s\t\n",err_line,locnow,tok_input[0]);
		}

		else if(baseflag) fprintf(itmd,"%d\t-1\t-\t%s\t%s\n",err_line,tok_input[0],tok_input[1]);

		else fprintf(itmd,"%d\t%X\t%s",err_line,locnow,input_save);
	
		err_line+=5;
	}//end .asm file input

	if(strcmp(tok_input[0],"END")!=0){
		printf("ERROR: Line %d. No END.\n",err_line-5);
		return 0;
	}
	
	prog_len = locctr-start_addr;
	fclose(fp);
	fclose(itmd);
	return 1;
}

int pass_2(char* filename){
	FILE* fp;
	FILE* lstfile;
	FILE* objfile;
	char lstfilename[30], objfilename[30];
	char intline[150];
	int linenum, locctr;
	char symbol[30], opcode[30], operand[30];
	char* temp[5];
	int startflag = 0;
	unsigned int i;
	int first_executable_instruction = start_addr;
	int objcode=0;
	char char_objcode[30];

	fp = fopen(filename,"r");

	if(!fp){
		printf("no file.\n");
		return 0;
	}
	
	fscanf(fp,"%s\n",intline);
	for(i=0;i<strlen(intline);i++) {
		if(intline[i]=='.'){
			intline[i]='\0';
			break;
		}
	}
	
	strcpy(lstfilename,intline);
	strcpy(objfilename,intline);

	//set listing file name
	strcat(lstfilename,".lst");

	//set object file name
	strcat(objfilename,".obj");
	
	//open intermediate file, listing file, object file
	lstfile = fopen(lstfilename,"w");
	objfile = fopen(objfilename,"w");

	fgets(intline,149,fp);

	//initialize base address for base relative
	base_addr = 0;
	
	startflag = 0;
	temp[0] = strtok(intline," \n\t");
	linenum = atoi(temp[0]);
		
	temp[1] = strtok(NULL," \n\t");
	locctr = strtol(temp[1],NULL,16);

	temp[2] = strtok(NULL," \n\t");
	strcpy(symbol,temp[2]);
	
	temp[3] = strtok(NULL," \n\t");
	if(temp[3]==NULL) opcode[0]='\0';
	else strcpy(opcode,temp[3]);

	temp[4] = strtok(NULL," \n\t");
	if(temp[4]==NULL) operand[0]='\0';
	else strcpy(operand,temp[4]);

	if(!strcmp("START",opcode)){
		startflag = 1;
		fflush(lstfile);
		fprintf(lstfile,"%d\t%04X\t%s\t%s\t%s\n",linenum,locctr,symbol,opcode,operand);
		
		first_executable_instruction = locctr;
		fprintf(objfile,"H%s",symbol);
		for(i=0;i<6-strlen(symbol);i++) fprintf(objfile," ");
		fprintf(objfile,"%06X%06X\n",start_addr,prog_len);
	}

	//if file's first line is not START, re-open file
	if(!startflag){
		fclose(fp);
		fp = fopen(filename,"r");
		fgets(intline,149,fp);
	}

	while(fgets(intline,149,fp)!=NULL){

		memset(char_objcode,'\0',30);

		temp[0] = strtok(intline," \n\t");
		linenum = atoi(temp[0]);

		temp[1] = strtok(NULL," \n\t");
		locctr = strtol(temp[1],NULL,16);

		temp[2] = strtok(NULL," \n\t");
		strcpy(symbol,temp[2]);

		//if comment line
		if(locctr == -2){
			temp[3] = strtok(NULL,"\n");
			if(temp[3]==NULL) fprintf(lstfile,"%d\n",linenum);
			else fprintf(lstfile,"%d\t\t%s\n",linenum,temp[3]);
			continue;
		}

		temp[3] = strtok(NULL," \n\t");
		if(temp[3]==NULL) opcode[0]='\0';
		else strcpy(opcode, temp[3]);

		temp[4] = strtok(NULL," \n\t");
		if(temp[4]==NULL) operand[0]='\0';
		else strcpy(operand, temp[4]);

		//if OPCODE = BASE
		if(locctr == -1){
			fprintf(lstfile,"%d\t\t\t%s\t%s\n",linenum,opcode,operand);
			base_addr = SearchSymbol(operand);

			//if BASE operand does not exist in SYMTAB
			if(base_addr==-1){
				printf("ERROR: Line %d. OPCODE 'BASE' error.\n",linenum);
				return 0;
			}
			continue;
		}

		if(!strcmp("START",opcode) && !startflag){
			startflag = 1;
			fprintf(lstfile,"%d\t%04X\t%s\t%s\t%s\n",linenum,locctr,symbol,opcode,operand);
		
			first_executable_instruction = locctr;
			fprintf(objfile,"H%s",symbol);
			for(i=0;i<6-strlen(symbol);i++) fprintf(objfile," ");
			fprintf(objfile,"%06X%06X\n",start_addr,prog_len);
			continue;
		}

		//if the program did not start with START, the first executable instruction would be the first line's LOCCTR
		if(!startflag){
			startflag = 1;
			fprintf(objfile,"H      %06X%06X\n",start_addr,prog_len);
			first_executable_instruction = locctr;
		}

		//printf("%d\t%X\t%s\t%s\t%s\n",linenum,locctr,symbol,opcode,operand);
		
		if(symbol[0]=='-') symbol[0]=' ';
		
		if(!strcmp("END",opcode)){
			fprintf(lstfile,"%d\t\t\t%s\t%s\n",linenum,opcode,operand);
			break;
		}


		//if this is a line to produce object code

		//if opcode not found in OPCODE TABLE
		if(search((int)opcode[0]%20,opcode)==-1 && opcode[0]!='+'){
			//if OPCODE = 'BYTE'
			if(!strcmp("BYTE",opcode)){
				//if Character
				if(operand[0]=='C'){
					temp[0] = strtok(operand,"C'");
					for(i=0;temp[0][i]!='\0';i++){
						sprintf(temp[1]+2*i,"%02X",(int)temp[0][i]);
					}
					strcpy(char_objcode,temp[1]);
				}
				
				//if X(hex)
				else if(operand[0]=='X'){
					//remove X and '
					temp[0] = strtok(operand,"X'");
					//object code has to have full bytes
					if(strlen(temp[0])%2==1) char_objcode[0]='0';
					//object code has to be the same as input
					strcat(char_objcode,temp[0]);

					//if input is not hexadecimal number
					if(!isHex(char_objcode)){
						printf("ERROR: Line %d. BYTE X needs hexadecimal number.\n",linenum);
						return 0;
					}
				}

				else{
					printf("ERROR: Line %d. OPCODE 'BYTE' needs C or X at operand field.",linenum);
					return 0;
				}
				
				//recover last "'" character that had been changed to '\0' by strtok
				strcat(operand,"'");
			}//end of BYTE
			
			//if OPCODE = 'WORD'
			else if(!strcmp("WORD",opcode)){
				if(operand==NULL){
					printf("ERROR: Line %d. Empty OPERAND field.\n",linenum);
					return 0;
				}
				objcode = atoi(operand);
				sprintf(char_objcode,"%06X",objcode);
			}//end of WORD

			else if(!strcmp("RESW",opcode) || !strcmp("RESB",opcode)) {
				if(operand==NULL) {
					printf("ERROR: Line %d. Empty OPERAND field.\n",linenum);
					return 0;
				}
				fprintf(lstfile,"%d\t%04X\t%s\t%s\t%s\n",linenum,locctr,symbol,opcode,operand);
				insertObj("\n\0",locctr,0);
				continue;
			}
			//printf("strlen(temp[0]): %d, temp[0]: %s.\n",(int)strlen(temp[0]),temp[0]);
			if(strlen(operand)>=8) fprintf(lstfile,"%d\t%04X\t%s\t%s\t%s\t%s\n",linenum,locctr,symbol,opcode,operand,char_objcode);
			else fprintf(lstfile,"%d\t%04X\t%s\t%s\t%s\t\t%s\n",linenum,locctr,symbol,opcode,operand,char_objcode);					

			insertObj(char_objcode,locctr,0);

			continue;
		}//end of valid OPERAND but not in OPCODE TABLE

		if(symbol[0]!=' ' && SearchSymbol(symbol)<0){
			printf("ERROR: Line %d. No symbol in SYMTAB.\n",linenum);
			return 0;
		}

		//find object code
		strcpy(char_objcode,findObj(operand,opcode,locctr));

		//object code not formed
		if (char_objcode[0]=='\0') {
			printf("ERROR: Line %d. Object Code cannot be made.\n",linenum);
			return 0;
		}
		
		//object code successfully made
		if(strlen(operand)>=8) fprintf(lstfile,"%d\t%04X\t%s\t%s\t%s\t%s\n",linenum,locctr,symbol,opcode,operand,char_objcode);
		else fprintf(lstfile,"%d\t%04X\t%s\t%s\t%s\t\t%s\n",linenum,locctr,symbol,opcode,operand,char_objcode);
	}

	if(!startflag) {
		first_executable_instruction = 0;
		fprintf(objfile,"H      %06X%06X\n",start_addr,prog_len);
	}
	//start writing Text Record at object program
	ObjectCode* cur = obhead->next;
	//for a line in the Object Program Text Record
	char record[61];
	//Length of object code in this record in bytes (hex)
	int record_len=0;
	//initialize record line
	memset(record,'\0',61);

	while(cur!=NULL){
		//if RESW or RESB, no object code - skip line
		if(cur->objectcode[0]=='\n') {
			cur = cur->next;
			continue;
		}

		//print starting address for object code in this record (hex)
		fprintf(objfile,"T%06X",cur->locctr);
		//object code, represented in hexadecimal(2columns per byte)
		while(cur!=NULL && cur->objectcode[0]!='\n'){
			//calculate length to see if it goes over 60)
			record_len+=strlen(cur->objectcode);
			//if it does go over 60, stop printing at this line
			if(record_len>60){
				record_len-=strlen(cur->objectcode);
				break;
			}
			//if available, print object code
			strcat(record,cur->objectcode);
			cur=cur->next;
		}
		//after deciding current line, print entire line at Object Program
		fprintf(objfile,"%02X%s\n",record_len/2,record);

		//initialize record line and record line's length
		memset(record,'\0',61);
		record_len=0;
	}

	//start writing Modification record at object program
	cur = obhead->next;
	while(cur!=NULL){
		if(cur->mod_flag==1) fprintf(objfile,"M%06X05\n",cur->locctr+1);
		cur = cur->next;
	}

	fprintf(objfile,"E%06X\n",first_executable_instruction);
	fclose(fp);
	fclose(lstfile);
	fclose(objfile);
	
	return 1;
}

void insertSymbol(char* label, int locctr){
	//create new node
	SYMTAB* node = (SYMTAB*)malloc(sizeof(SYMTAB));
	node->next=NULL;
	node->loc=locctr;
	strcpy(node->symbol,label);

	SYMTAB* cur = symhead->next;

	//if there is no symbol existing
	if(cur==NULL){
		symhead -> next = node;
		return;
	}

	//if the new symbol has to be at the first
	if(strcmp(node->symbol,cur->symbol) < 0){
		node->next = symhead->next;
		symhead->next = node;
		return;
	}

	while(cur->next != NULL){
		//compare if new node's symbol is alphabetically behind
		if(strcmp(node->symbol,(cur->next)->symbol) > 0) cur=cur->next;
		else{
			node->next = cur->next;
			cur->next = node;
			return;
		}
	}
	
	//if new symbol has to be at the last
	cur->next = node;
}

int SearchSymbol(char* label){
	SYMTAB* cur = symhead->next;
	//if SYMTAB is empty
	if(cur==NULL) return -1;

	else{
		while(cur!=NULL){
			//if symbol found in SYMTAB, return locctr 
			if(!strcmp(label,cur->symbol)) return cur->loc;
			cur=cur->next;
		}
	}

	//not found
	return -1;
}

void ShowSymbol(){
	SYMTAB* cur = symhead->next;
	//if SYMTAB is empty
	if(cur==NULL) {
		printf("No SYMTAB existing! Assemble first.\n");
		return;
	}

	else{
		while(cur!=NULL){
			printf("        %s\t%04X\n",cur->symbol,cur->loc);
			cur=cur->next;
		}
	}
}

void FreeSymbol(){
	//if SYMTAB is empty
	if(symhead->next == NULL){
		free(symhead);
		return;
	}

	SYMTAB* cur = symhead->next;
	while(cur!=NULL){
		SYMTAB* next = cur->next;
		free(cur);
		cur=next;
	}

	free(symhead);
}

void FreeObj(){
	//if Object Code Linked List is empty
	if(obhead->next == NULL){
		free(obhead);
		return;
	}

	ObjectCode* cur = obhead->next;
	while(cur!=NULL){
		ObjectCode* next = cur->next;
		free(cur);
		cur=next;
	}

	free(obhead);
}

char* findObj(char* operand,char* opmnemonic, int locctr){
	int pc;
	int n=0,i=0,x=0,b=0,p=1,e=0;
	int addr=0;
	char* objcode = (char*)malloc(sizeof(char)*30);
	int opcode=0;
	int format=0;
	char new_opcode[30], new_operand[30];
	int j;
	char *in_reg[2];
	char reg_num[10][2] = {{"A"},{"X"},{"L"},{"B"},{"S"},{"T"},{"F"},{"\0"},{"PC"},{"SW"}};
	int where_reg1=0, where_reg2=0;

	memset(objcode,'\0',30);
	memset(new_opcode,'\0',30);
	memset(new_operand,'\0',30);

	//set program counter(points to the next line's LOCCTR
	format = searchformat((int)opmnemonic[0]%20,opmnemonic);
	pc = locctr + format;

	//if there is no symbol in OPERAND field then store 0 as operand address
	if(operand[0]=='\0') {
		addr = 0;
		p=0;
	}
		
	//if there is a symbol in OPERAND field
	else{
		//store symbol value as operand address
		addr = SearchSymbol(operand);

		//if there is ',' in OPERAND field, it would mean x = 1
		for(j=0;operand[j+1]!='\0';j++){
			if(operand[j]==',' && operand[j+1]=='X') {
				x = 1;
				strcpy(new_operand,operand);
				new_operand[j]='\0';
				//save symbol's LOCCTR
				addr = SearchSymbol(new_operand);
			}
		}
	}//end of if symbol
	
	//if'+opmnemonic': format 4
	if(opmnemonic[0]=='+'){
		//searchformat returns 3 when mnemonic's format can be 3/4
		pc++;
		format = 4;
		for(j=1;opmnemonic[j]!='\0';j++) new_opcode[j-1]=opmnemonic[j];

		b = 0;
		p = 0;
		e = 1;
	}

	//if '#' at operand, it means immediate addressing
	if(operand[0]=='#'){
		n = 0;
		i = 1;
		b = 0;

		//if constant
		if(operand[1]>='0' && operand[1]<='9') p = 0;
		
		//pc relative
		else p = 1;

		//remove '#'
		for(j=1;operand[j]!='\0';j++) new_operand[j-1] = operand[j];

		if(p==1) addr = SearchSymbol(new_operand);
		else addr = atoi(new_operand);
	}

	//if '@' at operandm it means indirect addressing
	else if(operand[0]=='@'){
		n = 1;
		i = 0;

		//remove '@'
		for(j=1;operand[j]!='\0';j++) new_operand[j-1] = operand[j];

		addr = SearchSymbol(new_operand);
	}

	//if not above, typically it would be simple addressing
	else{
		n = 1;
		i = 1;
	}
	//printf("addr = %04X, pc = %04X\n",addr,pc); 
	//if symbol not found in SYMTAB
	if(format>=3 && addr == -1) return objcode;
	
	opcode = search((int)opmnemonic[0]%20,opmnemonic);
	if(new_opcode[0]!='\0') opcode = search((int)new_opcode[0]%20,new_opcode);

	//if format 1
	if(format==1){
		if(operand[0]!='\0') return objcode;
		sprintf(objcode,"%02X",opcode);
	}//end of format 1

	//if format 2
	else if(format==2){
		//if OPERAND FIELD empty: error
		if(operand[0]=='\0') return objcode;
		strcpy(new_operand,operand);
		//check which register
		in_reg[0] = strtok(new_operand,",");
		in_reg[1] = strtok(NULL,",");

		//find register's number
		for(j=0;j<10;j++){
			if(!strcmp(in_reg[0],reg_num[j])) {
				where_reg1 = j;
				break;
			}
		}

		//if cannot find register
		if(j==10) {
			//if OPCODE = SVC, n needed
			if(!strcmp("SVC",opmnemonic)){
				if(in_reg[0][0]>='1' && in_reg[0][0]<='9') where_reg1 = atoi(in_reg[0]) - 1;
				else return objcode;
			}
			else return objcode;
			j=0;
		}

		if(in_reg[1]==NULL) where_reg2 = 0;
		else{
			for(j=0;j<10;j++){
				if(!strcmp(in_reg[1],reg_num[j])) {
					where_reg2 = j;
					break;
				}
			}
		}

		if(j==10) {
			//if OPCODE = SHIFTL or SHIFTR
			if(!strcmp("SHIFTL",opmnemonic)||!strcmp("SHIFTR",opmnemonic)){
				if(in_reg[1][0]>='1' || in_reg[1][0]<='9') where_reg2 = atoi(in_reg[1]) - 1;
				else return objcode;
			}
			else return objcode;
		}

		sprintf(objcode,"%02X%X%X",opcode,where_reg1,where_reg2);
	}//end of format 2

	//if format 3 or 4, opcode's last 2 bits should change into n and i.
	opcode = (opcode/4)*4+2*n+i;

	//if format 3
	if(format==3){
		//if not constant value at operand field (when register needed)
		if(p==1){
			//disp = symbol locctr - pc address
			addr = addr - pc;

			//if base relative
			if(addr>2047 || addr<-2048){
				b = 1;
				p = 0;
				//if immediate addressing mode
				if((n==0&&i==1) || x==1) addr = SearchSymbol(new_operand) - base_addr;
				else addr = SearchSymbol(operand) - base_addr;
			}
		}

		if(addr<0) addr = (~(addr*(-1)) + 1) & 0x00000FFF;
		
		sprintf(objcode,"%02X%X%03X",opcode,8*x+4*b+2*p+e,addr);

	}//end of format 3

	//if format 4
	else if(format==4){
		if(p==1) p=0;
		sprintf(objcode,"%02X%X%05X",opcode,x*8+b*4+p*2+e,addr);
	}

	//if format is 4 and is not immediate addressing mode, modification needed
	if(format==4 && !(n==0 && i==1)) insertObj(objcode,locctr,1);
	else insertObj(objcode,locctr,0);

	return objcode;
}

void insertObj(char* res,int locctr, int mod){
	//make new node
	ObjectCode* node;

	//initialize new node
	node = (ObjectCode*)malloc(sizeof(ObjectCode));
	node->next = NULL;
	node->mod_flag=mod;

	strcpy(node->objectcode,res);
	node->locctr = locctr;

	//add node to existing list
	if(obhead->next==NULL) obhead->next=node;
	else{
		ObjectCode* cur = obhead->next;
		while(cur->next!=NULL) cur = cur->next;
		cur->next = node;
	}
}
