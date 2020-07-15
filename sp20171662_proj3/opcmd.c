#include "20171662.h"

//initialize hashTables to NULL
void init(){
	int i;
	for(i=0;i<TABLE_SIZE;i++){
		hashTable[i]=(Opcode*)malloc(sizeof(Opcode));
		hashTable[i]->next=NULL;
	}
}

//free hashTable
void destructor(){
	for(int i=0;i<TABLE_SIZE;i++){
		if(hashTable[i]!=NULL) free(hashTable[i]);
	}
	free(hashTable);
}

//search for the hashTable opcode instruction. 
int search(int key, char* instruction){
	Opcode* cur = hashTable[key]->next;
	while(cur!=NULL){
		if(!strcmp(instruction,cur->instruction_name))
			return cur->opcodenum;
		else cur=cur->next;
	}

	if(cur==NULL) return -1;
	else return -1;
}

int searchformat(int key, char* instruction){
	Opcode* cur = hashTable[key]->next;
	while(cur!=NULL){
		if(!strcmp(instruction,cur->instruction_name)){
			if(!strcmp("1",cur->format)) return 1;
			else if(!strcmp("2",cur->format)) return 2;
			else return 3;
		}
		else cur = cur->next;
	}

	if(cur==NULL) return -1;
	else return -1;
}


void add(Opcode* op_input, int key){
	op_input->next = hashTable[key]->next;
	hashTable[key]->next = op_input;
}

void show(){

	for(int i=0;i<TABLE_SIZE;i++){

		Opcode* cur = hashTable[i]->next;
		printf("%d : ",i);
		if(cur==NULL) printf("\n");
		while(cur!=NULL){
			printf("[%s,%X]",cur->instruction_name,cur->opcodenum);
			if(cur->next!=NULL) printf(" -> ");
			else printf("\n");
			cur=cur->next;
		}
	}

}
