#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_MACRO_NAME 30
#define MAX_MACRO_CODE 1000
#define MAX_PATH_LENGTH 255
#define MAX_LINE_LENGTH 1000
struct MacroTable{/*Info about a macro, used in a dynamic array*/
		char name[MAX_MACRO_NAME];
		char code[MAX_MACRO_CODE];
		int isEmpty;
};
int isMacroLocation(char line[],struct MacroTable macrotable[], int macroAmount)/*Checking whether this code line is a macro call*/
{
	char *ptr;
	int i;
	for(i=0;i<macroAmount;i++){
	    if(macrotable[i].name!="\0")
	        ptr=strstr(line,macrotable[i].name);
	    else
	        return 0;
	    if(ptr!=NULL){
	        if(strstr(ptr,"macro ")==NULL)
	            return 1;
	    }
	}
	return 0;
}
const char* getMacroName(char line[]){/*In case of finding a macro definement the function returns the macro's name*/
	char *ptr=strstr(line,"macro")+6;
	char arr[MAX_MACRO_NAME];
	int i=0;
	while(ptr[0]!='\n'){
	    arr[i]=ptr[0];
	    i++;
	    ptr++;
	}
	arr[i]='\n';
	ptr=arr;
	return ptr;
}
int main(){
	FILE *fp;
	FILE *macrofile;
	char c;
	printf("Enter a file path\n");
	char path[MAX_PATH_LENGTH];//file path
	fgets(path,MAX_PATH_LENGTH,stdin);
	fp = fopen(path,"r");
	int foundAMacro=0;
	int end = 1,macroExists=0,endmacroFound=0;/*"end" variable meaning EOF reached. all booleans*/
	char *ptr = path;
	while(ptr[0]!='.')
		ptr++;
	ptr[2] = 'm';
	macrofile = fopen(path,"w");
	char line[MAX_LINE_LENGTH];
	char macroName[MAX_MACRO_NAME];
	char macroCode[MAX_MACRO_CODE];
	int lineNum=0;
	int index=0;/*macroCode's copy tool.*/
	int i=1;//used to copy Macro code into macro call.
	int macroAmount=0;
	struct MacroTable *macroTable = malloc(sizeof(struct MacroTable));
	macroTable[0].isEmpty=1;
	struct MacroTable *helper;
	int macroLoc;
	int j;
	int lineI=0;
	while(!feof(fp)){
		fgets(line,MAX_LINE_LENGTH,(FILE *)fp);
		lineNum++;
		ptr=strstr(line,"macro ");/*searching if string "macro" is found in the file's current line.*/
		if(ptr!=NULL){/*if "macro" is found.*/
		    foundAMacro=1;
			strcpy(macroTable[macroAmount].name,getMacroName(line));
			if(macroTable[macroAmount].name=="mov"||macroTable[macroAmount].name=="cmp"||macroTable[macroAmount].name=="add"
			||macroTable[macroAmount].name=="sub"||macroTable[macroAmount].name=="lea"||macroTable[macroAmount].name=="clr"
			||macroTable[macroAmount].name=="not"||macroTable[macroAmount].name=="inc"||macroTable[macroAmount].name=="dec"
			||macroTable[macroAmount].name=="jmp"||macroTable[macroAmount].name=="bne"||macroTable[macroAmount].name=="jsr"
			||macroTable[macroAmount].name=="red"||macroTable[macroAmount].name=="prn"||macroTable[macroAmount].name=="rts"
			||macroTable[macroAmount].name=="stop"){
			    printf("Using invalid macro name found in line %d.\n",lineNum);
			}
			while(!endmacroFound){
				fgets(line,MAX_LINE_LENGTH,(FILE *)fp);
				ptr=strstr(line,"endm");
				if(ptr==NULL){/*if "endmacro" is not found.*/
    				while(line[lineI]!='\n'){/*copying macro content to macroCode.*/
    					macroCode[index]=line[lineI];
    					index++;
    					lineI++;
    				}
    				macroCode[index]='\n';
    				index++;
    				lineI=0;
				}
				else{
					endmacroFound=1;
					macroCode[index]='\0';/*signifies end of the macro code.*/
					strcpy(macroTable[macroAmount].code,macroCode);
					index=0;
				}
			}
			macroTable = realloc(macroTable,(macroAmount+2)*sizeof(struct MacroTable));/*allocating memory for 1 more label position.*/
			macroTable[macroAmount].isEmpty=0;
			macroAmount++;
			endmacroFound=0;//1 macro finished and then we continue on to the next ones.
		}
		else if(!feof(fp)){/*if "macro" is not found.*/
			macroLoc=isMacroLocation(line,macroTable,macroAmount);
			if(macroLoc==1&&foundAMacro==1){/*copying macro code to macro's call.*/
				for(j=0;j<macroAmount;j++){
				    ptr=strstr(line,macroTable[j].name);
				    if(ptr!=NULL){
				        break;
				    }
				}
				c=(macroTable[j].code)[0];
				i=1;
				while(c!='\0'){
					if(c!='\t')/*disregarding the '\t'(tab)*/
						fputc(c,macrofile);
					c=(macroTable[j].code)[i];
					i++;
				}
			}
			else{
				fputs(line,macrofile);
			}
		}
	}
	fclose(fp);
	fclose(macrofile);
	return 0;
}
