#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define MEM_SIZE 1<<20//2^20, 1megabyte memory
#define OPCODE "opcode.txt"
#define TABLE_SIZE 20//hashTable size

//for du[mp]. to save the last address printed
int end_index;

//for main input. gets the whole line
char* input;
//to save tokenized input
char* modified_input[100];

//to check if input is hex.
int isHex(char*);

//to change the tokenized input into a single line with correct input form
char* right_input_form(int,char**);

//=========================================
//===========FOR SHELL COMMANDS============
//=========================================

//***when d[ir]***
//-----------------------------------------

void printdir();

//-----------------------------------------

//***when hi[story]***
//-----------------------------------------

//Node for saving input
typedef struct Node{
	char* data;
	struct Node* next;
}Node;
//head of Linked List for hi[story]
Node* head;

//to add new node at the end of Linked List
void insertNode(Node*, char*);
//to print all nodes in the history list
void ShowAll(Node*);
//to free all memory allocated for history Linked List when q[uit]
void FreeAll(Node*);

//-----------------------------------------

//***when type filename***
//-----------------------------------------

int type(char*);


//=========================================
//===========FOR MEMORY COMMANDS===========
//=========================================

//***when du[mp] [start,end]***
//-----------------------------------------

//memory with size 2^20
int my_mem[MEM_SIZE];

//for du[mp]
void onlydump();
//for du[mp] start
void dumpstart(int);
//for du[mp] start, end
void dumpstartend(int,int);
//for printing out memory values of the given address range
void print_mem(int, int);

//-----------------------------------------

//***when e[dit] address, value***
//-----------------------------------------

//for e[dit] address, value. changes the value of the given memory address
void editvalue(int,int);

//-----------------------------------------

//***when f[ill] start, end, value***
//-----------------------------------------

//for f[ill] start, end, value. changes the memory value of given range to new value
void fillvalue(int, int, int);

//-----------------------------------------

//***when reset***
//-----------------------------------------

//for reset. changes all memory values to 0.
void reset_mem();

//-----------------------------------------


//=========================================
//===========FOR OPCODE COMMANDS===========
//=========================================

//***when opcode mnemonic***
//-----------------------------------------

//structure for Opcode hashTable
typedef struct Opcode{
	int opcodenum;//opcode
	char instruction_name[10];//instruction name
	char format[5];//format number
	struct Opcode* next;//points to the next node (Linked List)
}Opcode;

//hashTable for OPCODE TABLE.
Opcode** hashTable;

//for allocating memory for each hashTable[i]
void init();
//for freeing memory allocated for hashTable when q[uit]
void destructor();
//for searching for opcode of the given instruction
int search(int, char*);
//for searching for opcode format number
int searchformat(int, char*);
//for adding node to the hashTable[key]
void add(Opcode*, int);
//for opcodelist. prints out all contents of the hashTable
void show();

//-----------------------------------------


//=========================================
//========FOR ASSEMBLER COMMANDS===========
//=========================================

//***when assemble filename***
//-----------------------------------------

//SYMTAB - Linked List
typedef struct SYMTAB{
	char symbol[10];
	int loc;
	struct SYMTAB* next;
}SYMTAB;
SYMTAB* symhead;

typedef struct ObjectCode{
	char objectcode[30];
	int locctr;
	int mod_flag;
	struct ObjectCode* next;
}ObjectCode;
ObjectCode* obhead;

//program length
int prog_len;

//program starting address;
int start_addr;

//base address
int base_addr;

//insert LABEL, LOCCTR into SYMTAB
void insertSymbol(char*, int);

//search for symbol
int SearchSymbol(char*);

//main assembler
int assemble(char*);

//for Pass 1 of Assembler
int pass_1(char*);

//for Pass 2 of Assembler
int pass_2(char*);

//for object code
char* findObj(char*,char*,int);

//for inserting node at ObjectCode Linked List
void insertObj(char*, int, int);

//for Freeing Object Code Linked List
void FreeObj();

//***when symbol***
//----------------------------------------

//SYMTAB print
void ShowSymbol();

//Free SYMTAB
void FreeSymbol();
