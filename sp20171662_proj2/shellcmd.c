#include "20171662.h"


//for d[ir]
void printdir(){
	DIR *dir_ptr = NULL;
	struct dirent *file = NULL;
	struct stat buf;

	if((dir_ptr = opendir("."))==NULL) printf("ERROR: CANNOT READ DIRECTORY.\n");

	while ((file = readdir(dir_ptr)) != NULL){
		
		//to avoid falling into the infinite loop,
		//skipping '.' and '..' is necessary.
		//('.' refers to the current directory)
		if(!strcmp(file->d_name,".") || !strcmp(file->d_name,"..")) continue;

		//print file/directory name first
		printf("\t\t%s",file->d_name);
		stat(file->d_name,&buf);
		//if the file is a directory, S_ISDIR()=1
		if(S_ISDIR(buf.st_mode)) printf("/");
		//.out would be a file that the user has the right to execute
		else if((buf.st_mode & S_IXUSR)) printf("*");
		printf("\n");
	}
	closedir(dir_ptr);
}

//for hi[story]
void insertNode(Node* root, char* saved_input){
	//initialize new node
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->next = NULL;
	newNode->data = (char*)malloc(sizeof(char)*100);
	strcpy(newNode->data,saved_input);
	
	if(root->next==NULL) root->next=newNode;
	
	else{
		Node* cur = root->next;
		while(cur->next!=NULL) cur=cur->next;
		cur->next = newNode;
	}
}

void ShowAll(Node* root){
	int index = 1;
	Node* cur = root->next;
	if(cur==NULL) return;
	else{
		while(cur!=NULL){
			printf("\t\t%d\t%s\n",index++,cur->data);
			cur=cur->next;
		}
	}
}

void FreeAll(Node* root){
	if (root->next==NULL) return;
	Node* cur = root->next;
	while(cur!=NULL){
		Node* next=cur->next;
		free(cur);
		cur=next;
	}
	free(root);
}

int type(char* input){
	FILE* fp;
	char* input_str = (char*)malloc(sizeof(char)*200);
	//read input file
	fp = fopen(input,"r");

	//if no such file
	if(!fp){
		printf("No such file. See d[ir].\n");
		return -1;
	}
	
	//print every line
	while(fgets(input_str,200,fp)!=NULL){
		printf("%s",input_str);
	}

	fclose(fp);
	return 1;

}
