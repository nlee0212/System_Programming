#include "20171662.h"

int loader(char* objfile, int file_num){
	unsigned int i;
	//check file type
	for(i=0;i<strlen(objfile);i++){
		if(objfile[i]=='.') break;
	}

	if(strcmp(objfile+i,".obj")){
		printf("ERROR: File is not .obj file.\n");
		return 0;
	}
	
	//first execute pass 1, and if it succeeds,
	if(linkload_p1(objfile,file_num)) return 1;

	//if pass 1 fails, return 0
	else return 0;
}

int linkload_p1(char* objfile,int filenum){
	//for saving current line's record type
	char rec_type;
	//for saving current program's name
	char sec_name[7];
	//for saving current program's starting address and length
	int sec_start_addr, sec_length;
	//for keeping track of each D record's symbol
	unsigned int d_index=1;
	//for symbol name in D record
	char d_sym[7];
	//for symbol address in D record
	int d_addr;

	//object file's maximum column number is 73 (at the class material Chapter 2)
	char* obj_line = (char*)malloc(sizeof(char)*80);

	//open object file
	FILE* fp = fopen(objfile,"r");
	//if file doesn't exist, return 0
	if(!fp){
		printf("ERROR: File does not exist.\n");
		return 0;
	}

	//get the header record
	fgets(obj_line,80,fp);

	//check for header record
	if(obj_line[0]!='H'){
		printf("ERROR: There is no header record in this object file.\n");
		fclose(fp);
		return 0;
	}

	//save record type, control section name,
	//     control section starting address, 
	//     control section length
	//at header record,
	//H^______^______^______
	//  name   staddr seclen
	sscanf(obj_line,"%c%6s%06X%06X",&rec_type,sec_name,&sec_start_addr,&sec_length);
	
	//search ESTAB for control section name
	if(search_secname(filenum,sec_name)){
		printf("ERROR: Control section with this name already exists.\n");
		fclose(fp);
		return 0;
	}

	//set CSLTH to control section length
	CSLTH = sec_length;

	//enter control section name into ESTAB with value CSADDR and CSLTH
	strcpy(ESTAB[filenum].cs_name,sec_name);
	ESTAB[filenum].addr = CSADDR;
	ESTAB[filenum].len = CSLTH;

	while(fgets(obj_line,80,fp)!=NULL){
		//if record type = 'E', break
		if(obj_line[0]=='E') break;

		//if record type = 'D'
		if(obj_line[0]=='D'){
			d_index = 1;

			while(d_index<strlen(obj_line)-1){
				//scan each symbols separately
				sscanf(obj_line+d_index,"%6s%06X",d_sym,&d_addr);
				
				//search ESTAB for symbol name
				if(search_symname(filenum,d_sym)){
					printf("ERROR: Symbol name already exists.\n");
					fclose(fp);
					return 0;
				}
				
				//enter symbol into ESTAB with value
				insert_essym(filenum,d_sym,d_addr+CSADDR);

				//point to next symbol
				d_index+=12;
			}//end of while 'symbol exists'
		}//end of record type = 'D'
	}//end of while !'E'

	//add CSLTH to CSADDR (starting address for next control section)
	CSADDR += CSLTH;

	//at the end of pass 1, ESTAB contains all external symbols 
	//defined in the set of control sections together with the address 
	//assigned to each.

	fclose(fp);
	return 1;
}

int linkload_p2(char* objfile, int filenum, int total_file_num){
	char obj_line[80];
	char rec_type;
	char sec_name[7];
	int sec_start_addr, sec_length;
	//for reading T record
	int t_len, t_start_addr, t_index, t_addr;
	//for reading R record
	int r_num, r_addr;
	unsigned int r_index=1;
	char r_sym[7];
	//for saving 1 byte from main memory
	int obj_byte;
	//for reading M record
	int m_addr, m_len, m_ref, m_code, m_tmp;
	//when format 4, save half byte containing 'xbpe'
	int save_value;
	//used for modifying int object code into text for convenience
	char m_code_txt[7];
	//for saving + or - in M record
	char operator;

	//structure for saving symbols and their address with reference numbers as index
	typedef struct REF{
		char sym_name[7];
		int sym_addr;
	}ref;
	ref ref_list[6];

	EXECADDR = progaddr;
	CSADDR = ESTAB[filenum].addr;

	FILE* fp = fopen(objfile,"r");
	
	//get the header record
	fgets(obj_line,80,fp);

	//for 'H' record
	//save record type, control section name,
	//     control section starting address, 
	//     control section length
	sscanf(obj_line,"%c%6s%06X%06X",&rec_type,sec_name,&sec_start_addr,&sec_length);
	
	//set CSLTH to control section length
	CSLTH = sec_length;

	while(fgets(obj_line,80,fp)!=NULL){
		if(obj_line[0]=='E'){
			//if an address is specified in End record then
			//set EXECADDR to (CSADDR+specified address)
			if(isHex(obj_line+1)) EXECADDR = CSADDR + strtol(obj_line+1,NULL,16);
			break;
		}

		//for 'R' record
		else if(obj_line[0]=='R'){
			r_index=1;

			//reference number 1 is for control section's address
			ref_list[1].sym_addr = CSADDR;

			//save reference number and symbol name with its address
			while(r_index<strlen(obj_line)-1){
				sscanf(obj_line+r_index,"%02X%6s",&r_num,r_sym);
				r_addr = search_extsym(r_sym,total_file_num);
				
				if(r_addr==-1){
					printf("ERROR: Symbol not found in ESTAB.\n");
					fclose(fp);
					return 0;
				}

				ref_list[r_num].sym_addr = r_addr;
				strcpy(ref_list[r_num].sym_name,r_sym);
				r_index+=8;
			}
		}

		//for 'T' record
		else if(obj_line[0]=='T'){
			sscanf(obj_line,"%c%06X%02X",&rec_type,&t_start_addr,&t_len);
			
			//original length was representing total bytes
			t_len *= 2;
			//include 'T'+start_address
			t_len += 9;

			//object code starts from index 9 in T record
			t_index = 9;

			//set location to CSADDR + specified address
			t_addr = t_start_addr + CSADDR;

			//move object code from record to location
			while(t_index<t_len){
				sscanf(obj_line+t_index,"%02X",&obj_byte);
				my_mem[t_addr] = obj_byte;
				t_addr++;
				t_index+=2;
			}

		}//end of if 'T'
		
		//for 'M' record
		else if(obj_line[0]=='M'){
			sscanf(obj_line,"%c%06X%02X%c%02X",&rec_type,&m_addr,&m_len,&operator,&m_ref);
		
			m_addr += CSADDR;

			//calculate code to modify
			sprintf(m_code_txt,"%02X%02X%02X",my_mem[m_addr],my_mem[m_addr+1],my_mem[m_addr+2]);
			sscanf(m_code_txt,"%06X",&m_code);
			save_value = m_code/0x100000;


			//add or subtract symbol value at location
			if(operator=='+') m_code += ref_list[m_ref].sym_addr;
			else if(operator=='-') m_code -= ref_list[m_ref].sym_addr;
			else{
				printf("ERROR: operator needs to be '+' or '-'.\n");
				fclose(fp);
				return 0;
			}

			
			sprintf(m_code_txt,"%08X",m_code);

			if(m_len==0x05){
				sscanf(m_code_txt,"%03X%1X%02X%02X",&m_tmp,&my_mem[m_addr],&my_mem[m_addr+1],&my_mem[m_addr+2]);
				my_mem[m_addr]+=save_value*0x10;
			}

			else sscanf(m_code_txt,"%02X%02X%02X%02X",&m_tmp,&my_mem[m_addr],&my_mem[m_addr+1],&my_mem[m_addr+2]);

		}//end of 'M'
	}//end of while not EOF

	CSADDR += CSLTH;


	fclose(fp);
	
	return 1;
}

void insert_essym(int filenum, char* sym_name, int sym_addr){
	//create new node for ESTAB symbol
	ESTAB_sym* node = (ESTAB_sym*)malloc(sizeof(ESTAB_sym));
	node->next = NULL;
	node->address = sym_addr;
	strcpy(node->symbol,sym_name);

	//set cursor to current control section's node's es_sym
	ESTAB_sym* cur = ESTAB[filenum].es_sym->next;
	
	//if there is no symbol in ESTAB for this control section, insert node
	if(cur==NULL) ESTAB[filenum].es_sym->next = node;

	//else find the end of the list and insert node
	else{
		while(cur->next!=NULL) cur = cur->next;
		cur->next = node;
	}
}

int search_extsym(char* sym_name, int file_num){
	int i;
	ESTAB_sym* cur;

	//search for symbol and return symbol's address
	for(i=0;i<file_num;i++){
		cur = ESTAB[i].es_sym->next;
		while(cur!=NULL){
			if(!strcmp(cur->symbol,sym_name)) return cur->address;
			cur = cur->next;
		}
	}
	
	//if symbol not found in ESTAB
	return -1;
}		

int search_symname(int filenum, char* sym_name){
	//symbol list of current control section
	ESTAB_sym* cur = ESTAB[filenum].es_sym->next;

	while(cur!=NULL){
		if(!strcmp(cur->symbol,sym_name)) return 1;
		cur = cur->next;
	}

	return 0;
}

int search_secname(int filenum,char* sec_name){
	int i;

	for(i=0;i<filenum;i++){
		if(!strcmp(ESTAB[i].cs_name,sec_name)) return 1;
	}

	return 0;
}

void show_loadmap(int file_num){
	int i;
	//for calculating total length of the program (sum of each section's length)
	int total_len=0;

	ES_TAB cur;
	ESTAB_sym* cur_sym;
	
	printf("control\tsymbol\taddress\tlength\n");
	printf("section\tname\n");
	printf("----------------------------------\n");

	for(i=0;i<file_num;i++){
		cur = ESTAB[i];
		//first print out current control section's data
		printf("%s\t\t%04X\t%04X\n",cur.cs_name,cur.addr,cur.len);
		//add current control section's length
		total_len+=cur.len;
		//set pointer to current contrl section's symbol list
		cur_sym=cur.es_sym->next;
		while(cur_sym!=NULL){
			//print out current control section's symbol list
			printf("\t%6s\t%04X\n",cur_sym->symbol,cur_sym->address);
			cur_sym = cur_sym->next;
		}
	}
	printf("----------------------------------\n");
	//print out total length of the whole program
	printf("\t  total length  %04X\n",total_len);
}

void show_bp(){
	bp* cur = bp_head->next;

	printf("\t\tbreakpoint\n");
	printf("\t\t----------\n");

	while(cur!=NULL){
		printf("\t\t%X\n",cur->loc);
		cur = cur->next;
	}
}

void save_bp(int bp_loc){
	bp* cur = bp_head->next;
	
	//create new node
	bp* node = (bp*)malloc(sizeof(bp));
	node->loc = bp_loc;
	node->next = NULL;

	//insert at the end of list
	if(cur==NULL) bp_head->next = node;
	else{
		while(cur->next!=NULL) cur = cur->next;
		cur->next = node;
	}

	printf("\t\t[ok] create breakpoint %X\n",bp_loc);
}

void clear_bp(){
	bp* cur = bp_head->next;

	if(cur==NULL){
		printf("\t\tno breakpoints existing!\n");
		return;
	}

	//free nodes in bp list except for the head node
	while(cur!=NULL){
		bp* next = cur->next;
		free(cur);
		cur = next;
	}

	bp_head->next = NULL;

	printf("\t\t[ok] clear all breakpoints\n");
}

void run(){
	//set start address
	static int cur_addr = 0;
	//save total length
	int run_len = ESTAB[0].len;
	//register values, A=0, X=1, L=2, B=3, S=4, T=5, PC=8, SW=9
	static int reg[10];
	//n,i,x,b,p
	int n=0,i=0,x=0,b=0,p=0;
	//code length
	int code_len;
	//first half byte
	int hbyte;
	//one full object code
	int objcode;
	//displacement, 12 bit (for format 3)
	int disp;
	//address, 20 bit (for format 4)
	int addr;
	//for register instructions
	int reg1, reg2;
	//for target address
	int target_addr=0;
	//for immediate addressing mode
	int im_value;
	//set opcode
	int opcode;
	//when debugging, do not stop at the same bp as last time (it starts from there)
	static int last_bp = -1;

	
	//if there was no bp before, set L = endpoint, PC = progaddr
	if(!in_debug){
		reg[2] = progaddr + run_len;
		reg[8] = cur_addr = EXECADDR;
	}

	//run through file until bp
	while(1){
		//if file reaches the last code
		if(reg[8]==progaddr+run_len){
			//print register values
			print_reg(reg);

			//initialize register values
			for(i=0;i<10;i++) reg[i]=0;

			printf("\t    End Program\n");

			//set debug flag to 0 to show no debugging is going on
			in_debug=0;

			return;
		}

		//if file reaches bp
		if(search_bp(reg[8]) && last_bp != reg[8]){
			//print register values
			print_reg(reg);

			printf("\t    Stop at checkpoint[%X]\n",reg[8]);

			last_bp = reg[8];

			//set debug flag to 1 to show debugging is going on
			in_debug=1;

			return;
		}

		//set current address to PC
		cur_addr = reg[8];

		//check first half byte of the code
		hbyte = my_mem[cur_addr]/0x10;
	
		//first half byte 9~B:format 2, else format 3/4 (no format 1 in this ex)
		//if format 2, 2 bytes
		if(hbyte>=0x9 && hbyte<=0xB) code_len=2;
		//if format 3 or 4, 3 or 4 bytes
		else code_len=3;
		
		//set opcode, get rid of n & i
		opcode = my_mem[cur_addr] & 0xFC;
		//printf("opcode: %X\n",opcode);

		//if format 2
		if(code_len==2){
			//PC += 2
			reg[8]+=2;

			//if CLEAR, r1<-0
			if(my_mem[cur_addr]==0xB4){
				reg1 = my_mem[cur_addr+1]/0x10;
				reg[reg1] = 0;
			}
			
			//if COMPR, (r1):(r2)
			else if(my_mem[cur_addr]==0xA0){
				//A0(r1)(r2)
				reg1 = my_mem[cur_addr+1]/0x10;
				reg2 = my_mem[cur_addr+1]%0x10;

				if(reg1<0 || reg1>9 || reg2<0 || reg2>9){
					printf("ERROR: register number is incorrect.\n");
					return;
				}

				if(reg[reg1]<reg[reg2]) reg[9]=-1;
				else if(reg[reg1]==reg[reg2]) reg[9]=0;
				else reg[9]=1;
			}

			//if TIXR, X<-X+1, X:r1
			else if(my_mem[cur_addr]==0xB8){
				//X<-X+1
				reg[1]++;
				reg1 = my_mem[cur_addr+1]/0x10;

				if(reg1<0||reg1>9){
					printf("ERROR: register number is incorrect.\n");
					return;
				}

				//X:r1
				if(reg[1]<reg[reg1]) reg[9]=-1;
				else if(reg[1]==reg[reg1]) reg[9]=0;
				else reg[9]=1;
			}

			else{
				printf("ERROR: invalid OPCODE in format 2.\n");
				return;
			}

			continue;
		}//end of format 2

		//check if format 4
		else{
			hbyte = my_mem[cur_addr+1]/0x10;
			//if 3rd half byte of object code is odd, e=1, format 4.
			if(hbyte%2) code_len = 4;
		}

		//if format 3
		if(code_len==3){
			//PC += 3
			reg[8]+=3;
			
			//set object code
			objcode = my_mem[cur_addr]*0x10000+my_mem[cur_addr+1]*0x100+my_mem[cur_addr+2];

			//set nixbp (since format 3, e=0)
			n = objcode & 0x020000;
			i = objcode & 0x010000;
			x = objcode & 0x008000;
			b = objcode & 0x004000;
			p = objcode & 0x002000;

			disp = objcode & 0x000FFF;

			//check sign bit(leftmost bit) of disp
			//if minus, set left bits to F
			if(disp & 0x000800) disp = disp | 0xFFFFF000;
			
			//if simple addressing mode, n=1, i=1
			if(n && i){
				//if pc relative, disp + PC
				if(p) target_addr = reg[8]+disp;
				//if base relative, disp + B
				else if(b) target_addr = disp+reg[3];
				//if direct addressing mode
				else if(!p && !b) target_addr = disp+progaddr;
				//if X 
				if(x) target_addr += reg[1];
				im_value = my_mem[target_addr]*0x10000+my_mem[target_addr+1]*0x100+my_mem[target_addr+2];
			}

			//if indirect addressing mode, n=1, i=0
			else if(n && !i){
				if(p) disp += reg[8];
				else if(b) disp += reg[3];
				else if(!p && !b) disp+=progaddr;
				if(x) disp += reg[1];

				//since this is indirect addressing mode,
				//target address = object code at updated disp
				target_addr = my_mem[disp]*0x10000+my_mem[disp+1]*0x100+my_mem[disp+2];
				
				im_value = my_mem[target_addr]*0x10000+my_mem[target_addr+1]*0x100+my_mem[target_addr+2];
			}

			//if immediate addressing mode, n=0, i=1
			else{
				//no need to set target addr
				im_value = disp;
				if(p) im_value += reg[8];
				else if(b) im_value += reg[3];
			}

			
			run_form34(opcode,im_value,code_len,target_addr,reg,x);
		}//end of format 3

		//if format 4
		else if(code_len==4){
			//for(int k=0;k<100;k++) printf("INFORM4");
			reg[8]+=4;

			//set object code
			objcode = my_mem[cur_addr]*0x1000000+my_mem[cur_addr+1]*0x10000+my_mem[cur_addr+2]*0x100+my_mem[cur_addr+3];

			
			//set n,i,x,b,p,(e=1)
			n = objcode & 0x02000000;
			i = objcode & 0x01000000;
			x = objcode & 0x00800000;
			b = objcode & 0x00400000;
			p = objcode & 0x00200000;

			//set address to rightmost 5 half bytes
			addr = objcode & 0x000FFFFF;

			if(addr & 0x00080000) addr = addr | 0xFFF00000;
			
			//if simple addressing mode, n=1, i=1
			if(n&&i){
				if(p) target_addr = addr + reg[8];
				else if(b) target_addr = addr + reg[2];
				else if(!p && !b) target_addr = addr + progaddr;
				if(x) target_addr += reg[1];

				im_value = my_mem[target_addr]*0x1000000+my_mem[target_addr+1]*0x10000+my_mem[target_addr+2]*0x100+my_mem[target_addr+3];
			}

			//if indirect mode, n = 1, i = 0
			else if(n&&!i){
				if(p) addr += reg[8];
				else if(b) addr += reg[3];
				if(x) addr += reg[1];

				target_addr = my_mem[target_addr]*0x1000000+my_mem[target_addr+1]*0x10000+my_mem[target_addr+2]*0x100+my_mem[target_addr+3];
				
				im_value = my_mem[target_addr]*0x1000000+my_mem[target_addr+1]*0x10000+my_mem[target_addr+2]*0x100+my_mem[target_addr+3];
			}

			//if immediate addressing mode, n=0, i=1
			else{
				im_value = addr;
				if(p) im_value += reg[8];
				else if(b) im_value += reg[3];
			}

			run_form34(opcode,im_value,code_len,target_addr,reg,x);

		}//end of format 4

		//print_reg(reg);
		//return;
	}//end of run before bp

	return;
}

void run_form34(int opcode, int im_value, int code_len, int target_addr, int* reg, int x){
	//LDA A<-m..m+2
	if(opcode==0x00) reg[0] = im_value;

	//LDB B<-m..m+2
	else if(opcode==0x68) reg[3] = im_value;

	//LDT T<-m..m+2
	else if(opcode==0x74) reg[5] = im_value;

	//LDCH A[rightmost byte] <- m
	else if(opcode==0x50){
		//erase rightmost byte
		reg[0] = reg[0] & 0xFFFF00;

		//put memory value in rightmost byte
		//if x, A<-m[x]
		if(x) reg[0] += im_value / 0x10000;
		else reg[0] += im_value & 0x0000FF;
	}

	//STA m..m+2<-A
	else if(opcode==0x0C) store_mem(reg[0],target_addr,code_len*2);

	//STX m..m+2<-X
	else if(opcode==0x10) store_mem(reg[1],target_addr,code_len*2);

	//STL m..m+2<-L
	else if(opcode==0x14) store_mem(reg[2],target_addr,code_len*2);

	//STCH m<-A[rightmost byte]
	else if(opcode==0x54) store_mem(reg[0]&0x000000FF,target_addr,code_len*2);

	//J PC<-m
	else if(opcode==0x3C) reg[8] = target_addr;

	//JSUB L<-PC; PC<-m
	else if(opcode==0x48){
		reg[2] = reg[8];
		reg[8] = target_addr;
		//for(int i=0;i<100;i++) printf("%06X\n",target_addr);
	}

	//JLT PC<-m if CC set to < (CC saved in SW reg)
	else if(opcode==0x38){
		if(reg[9]<0) reg[8] = target_addr;
	}

	//JEQ PC<-m if CC set to =
	else if(opcode==0x30){
		if(reg[9]==0) reg[8] = target_addr;
	}

	//RSUB PC<-L
	else if(opcode==0x4C) reg[8] = reg[2];

	//COMP A:m..m+2
	else if(opcode==0x28){
		if(reg[0]<im_value) reg[9] = -1;
		else if(reg[0]==im_value) reg[9] = 0;
		else reg[9] = 1;
	}

	//TD: continue after setting CC to <
	else if(opcode==0xE0) reg[9] = -1;

	//RD: continue after setting CC to =
	else if(opcode==0xD8) reg[9] = 0;

	//WD: skip
}

void store_mem(int value, int addr, int halfbyte){
	char txt_value[10];

	//if format 4, store 4 bytes starting from addr
	if(halfbyte==8){
		value = value & 0xFFFFFFFF;
		sprintf(txt_value,"%08X",value);
		sscanf(txt_value,"%02X%02X%02X%02X",&my_mem[addr],&my_mem[addr+1],&my_mem[addr+2],&my_mem[addr+3]);
	}

	//if format 3, store 3 bytes starting from addr
	else{
		//make sure there are only 3 bytes
		value = value & 0x00FFFFFF;
		sprintf(txt_value,"%06X",value);
		sscanf(txt_value,"%02X%02X%02X",&my_mem[addr],&my_mem[addr+1],&my_mem[addr+2]);
	}
}

void print_reg(int* reg){
	printf("A : %06X  X : %06X\n",reg[0],reg[1]);
	printf("L : %06X PC : %06X\n",reg[2],reg[8]);
	printf("B : %06X  S : %06X\n",reg[3],reg[4]);
	printf("T : %06X\n",reg[5]);
}

int search_bp(int bp_loc){
	bp* cur = bp_head->next;

	while(cur!=NULL){
		if(cur->loc == bp_loc) return 1;
		cur = cur->next;
	}

	return 0;
}
