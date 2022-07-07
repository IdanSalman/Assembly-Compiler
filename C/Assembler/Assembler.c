#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#define MAX_LABEL_LENGTH 30
#define MAX_PATH_LENGTH 255
#define MAX_LINE_LENGTH 255
#define MAX_CODE_IMAGE 50
struct Label{/*Stores each Label's info, used in a dynamic array*/
	char name[MAX_LABEL_LENGTH];
	int loc;/*none 0 extern 1 entry 2*/
	int address;
};
struct Data{/*Stores each data type's info(.data or .string), used in a dynamic array*/
	unsigned int data:16;
};
struct Instruction{/*Stores each instruction's info, used in a dynamic array*/
	unsigned int opcode:16;
	unsigned int funct:4;
	unsigned int source_operand:2;
	unsigned int target_operand:2;
	unsigned int source_reg:4;
	unsigned int target_reg:4;
	unsigned int firstword:16;
	unsigned int secondword:16;
};
int baseAddress(int n){/*base address of a label while n is the label's address*/
    return 16*((int)n/16);
}
int nameCompare(char *name, char *line){/*Checking if a Label's name is found at the start of a line.*/
    char *ptr;
    int equal=1;
    int i=0;
    int j;
    ptr=line;
    while(ptr[0]==' '||ptr[0]=='\t'){
        ptr++;
    }
    while(ptr[i]!=':'&&i<254){
        i++;
    }
    for(j=0;j<i;j++){
        if(ptr[j]!=name[j])
            equal=0;
    }
    return equal;
}
/* target = 1 => search in source operand.
   target = 2 => search in target operand.
*/
int findLabelAddress(int lblAmount, struct Label labelTable[lblAmount+1], char *line, int target){/*Returns the address of a label in the wanted position*/
    int i;
    if(target==1){
        for(i=0;i<lblAmount;i++){
            if(strstr(line,labelTable[i].name)!=NULL)
                return labelTable[i].address;
        }
    }
    else{
        for(i=0;i<lblAmount;i++){
            if(strchr(line,',')!=NULL)
                if(strstr(strchr(line,','),labelTable[i].name)!=NULL)
                    return labelTable[i].address;
        }
        for(i=0;i<lblAmount;i++){
            if(strstr(line,labelTable[i].name)!=NULL)
                return labelTable[i].address;
        }
    }
    return -1;
}
int segLength(char *line){/*Returns the length of the data needed (in .data the amount of numbers, int .string the string length+1('\0')*/
    char *ptr;
    int counter=0;
    if(strstr(line,".data")!=NULL){
        ptr=strstr(line,".data");
        counter++;
        ptr=strchr(ptr,',');
        while(ptr!=NULL){
            counter++;
            ptr++;
            ptr=strchr(ptr,',');
        }
        return counter;
    }
    else{
        ptr=strchr(line,'\"');
        ptr++;
        counter++;/*pre calculating the 0 in the end.*/
        while(ptr[0]!='\"'){
            counter++;
            ptr++;
        }
    }
    if(strstr(line,".string")!=NULL)
        return counter;
    else
        return counter+1;
}
char hexa(int n){/*Converts a decimal number to hexa(base 16)*/
    if(n>=0&&n<=9)
        return n+'0';
    if(n==10)
        return 'a';
    if(n==11)
        return 'b';
    if(n==12)
        return 'c';
    if(n==13)
        return 'd';
    if(n==14)
        return 'e';
    if(n==15)
        return 'f';
}
int main()
{
    char func[MAX_LINE_LENGTH];
    int errorFound=0;
    int lineNum=0;/*Used in error finding to tell which line has an error*/
    int oneArgument=1;
    int commandLine=1;/*If a line is a command line or an empty line or a note*/
    int labelFound=0;/*In case that a label is found and attempted to be defined twice*/
    int entryLabelFound=0;
    int externalLabelFound=0;
    int funct;
    int i;
    int IC=0;
    int DC=0;
    int address=100;
    int lblAmount=0;
    int value;
    int numFind;
    FILE *fp;
    FILE *obj;
    char name[MAX_LABEL_LENGTH];
    char path[MAX_PATH_LENGTH];
    char originalPath[MAX_PATH_LENGTH];
    char line[MAX_LINE_LENGTH];
    struct Instruction instruction[MAX_CODE_IMAGE];
    struct Data data[MAX_CODE_IMAGE];
    struct Label *labelTable = malloc(sizeof(struct Label));
    fgets(originalPath,MAX_PATH_LENGTH,stdin);
    strcpy(path,originalPath);
    fp=fopen(path,"r");
    char *ptr = path;
	while(ptr[0]!='.')
		ptr++;
	ptr[1] = 'o';
	ptr[2] = 'b';
    while(!feof(fp)){
        fgets(line,MAX_LINE_LENGTH,(FILE *)fp);
        lineNum++;
        commandLine=1;
        if(line[0]==';')
            commandLine=0;
        ptr=line;
        while(ptr[0]==' '||ptr[0]=='\t'||ptr[0]=='\n'||ptr[0]=='\r'){
            ptr++;
        }
        if(ptr[0]=='\0')
            commandLine=0;
        if(commandLine){
            if((strstr(line,".entry")!=NULL||strstr(line,".extern")!=NULL)&&!feof(fp)){
                if(strstr(line,".extern")!=NULL){
                    ptr=line;
                    ptr+=8;
                    i=0;
                    while(ptr[i]!='\n')
                        i++;
                    ptr[i-1]='\0';
                    strcpy(labelTable[lblAmount].name,ptr);
                    labelTable[lblAmount].loc=1;
                    labelTable[lblAmount].address=0;
                }
                else{
                    ptr=line;
                    ptr+=7;
                    i=0;
                    while(ptr[i]!='\n'&&ptr[i]!=' '){
                        i++;
                    }
                    ptr[i-1]='\0';
                    for(i=0;i<lblAmount;i++){
                        if(strcmp(labelTable[i].name,ptr)==0){
                            labelFound=1;
                            labelTable[i].loc=2;
                        }
                    }
                    if(labelFound==0){
                        strcpy(labelTable[lblAmount].name,ptr);
                        labelTable[lblAmount].loc=2;
                        labelTable[lblAmount].address=0;
                    }
                }
                if(labelFound==0){
                    if(labelTable[lblAmount].name=="mov"||labelTable[lblAmount].name=="cmp"||labelTable[lblAmount].name=="add"||labelTable[lblAmount].name=="sub"
                        ||labelTable[lblAmount].name=="lea"||labelTable[lblAmount].name=="clr"||labelTable[lblAmount].name=="not"||labelTable[lblAmount].name=="inc"
                        ||labelTable[lblAmount].name=="dec"||labelTable[lblAmount].name=="jmp"||labelTable[lblAmount].name=="bne"||labelTable[lblAmount].name=="jsr"
                        ||labelTable[lblAmount].name=="red"||labelTable[lblAmount].name=="prn"||labelTable[lblAmount].name=="rts"||labelTable[lblAmount].name=="stop"){
                        errorFound=1;
                        printf("Used invalid label name in line %d.\n",lineNum);
                    }
                    labelTable = realloc(labelTable,(lblAmount+2)*sizeof(struct Label));/*allocating memory for 1 more label position.*/
                    lblAmount++;
                }
                labelFound=0;
            }
            ptr=line;
            ptr=strchr(line,':');
            if(ptr!=NULL&&!feof(fp)){/*found a label...*/
                strcpy(name,line);
                i=0;
                while(line[i]!=':'){
                    i++;
                }
                name[i]='\0';
                for(i=0;i<lblAmount;i++){
                    if(strcmp(labelTable[i].name,name)==0){
                        labelFound=1;
                        if(labelTable[i].loc==2){
                            labelTable[i].address=address;
                        }
                    }
                }
                if(labelFound==0){
                    strcpy(labelTable[lblAmount].name,name);
                    labelTable[lblAmount].loc=0;
                    labelTable[lblAmount].address=address;
                    if(labelTable[lblAmount].name=="mov"||labelTable[lblAmount].name=="cmp"||labelTable[lblAmount].name=="add"||labelTable[lblAmount].name=="sub"
                    ||labelTable[lblAmount].name=="lea"||labelTable[lblAmount].name=="clr"||labelTable[lblAmount].name=="not"||labelTable[lblAmount].name=="inc"
                    ||labelTable[lblAmount].name=="dec"||labelTable[lblAmount].name=="jmp"||labelTable[lblAmount].name=="bne"||labelTable[lblAmount].name=="jsr"
                    ||labelTable[lblAmount].name=="red"||labelTable[lblAmount].name=="prn"||labelTable[lblAmount].name=="rts"||labelTable[lblAmount].name=="stop"){
                        errorFound=1;
                        printf("Used invalid label name in line %d.\n",lineNum);
                    }
                    labelTable = realloc(labelTable,(lblAmount+2)*sizeof(struct Label));//allocating memory for 1 more label position.
                    lblAmount++;
                }
                labelFound=0;
                
            }
            if(!feof(fp)){
                ptr=line;
                ptr=strchr(line,':');
                if(ptr==NULL)
                    ptr=line;
                else
                    ptr++;
                while(ptr[0]==' '||ptr[0]=='\t'){
                    ptr++;
                }
            }
            if(strstr(line,".data")!=NULL||strstr(line,".string")!=NULL||strstr(line,".entry")!=NULL||strstr(line,".extern")!=NULL){
                if(strstr(line,".data")!=NULL||strstr(line,".string")!=NULL){
                    //------------------------Data-Array-----------------------------
                    struct Data temp;
                    if(strstr(line,".string")!=NULL){
                        ptr=strchr(line,'\"');
                        for(i=0;i<segLength(line)-1;i++){
                            ptr++;
                            temp.data=(int)ptr[0];
                            data[DC+i]=temp;
                        }
                        temp.data=0;
                        data[DC+segLength(line)-1]=temp;
                        DC+=segLength(line);
                        address+=segLength(line);
                    }
                    
                    else if(strstr(line,".data")!=NULL){
                        ptr=strstr(line,".data");
                        ptr+=5;//length of the word ".data"
                        for(i=0;i<segLength(line);i++){
                            ptr++;
                            while(ptr[0]==' '||ptr[0]=='\t'){
                                ptr++;
                            }//reaching the next number;
                            if(ptr[0]=='-'){
                                temp.data=-1*(ptr[1]-48);
                                if(isdigit(ptr[2])){
                                    temp.data*=10;
                                    temp.data-=ptr[2]-48;
                                    if(isdigit(ptr[3])){
                                        temp.data*=10;
                                        temp.data-=ptr[3]-48;
                                    }
                                }
                            }
                            else if(ptr[0]=='+'){
                                temp.data=(ptr[1]-48);
                                if(isdigit(ptr[2])){
                                    temp.data*=10;
                                    temp.data+=ptr[2]-48;
                                    if(isdigit(ptr[3])){
                                        temp.data*=10;
                                        temp.data+=ptr[3]-48;
                                    }
                                }
                            }
                            else{
                                temp.data=(ptr[0]-48);
                                if(isdigit(ptr[1])){
                                    temp.data*=10;
                                    temp.data+=ptr[1]-48;
                                    if(isdigit(ptr[2])){
                                        temp.data*=10;
                                        temp.data+=ptr[2]-48;
                                    }
                                }
                            }
                            data[DC+i]=temp;
                            ptr=strchr(ptr,',');
                        }
                        DC+=segLength(line);
                        address+=segLength(line);
                    }
                    //------------------------Data-Array-----------------------------
                }
            }
            else if(!feof(fp)){
                strcpy(func,ptr);
                func[4]='\0';
                if(strcmp(func,"stop")==0){/*special 4 letter function*/
                    value=15;
                    funct=0;
                    ptr+=4;
                }
                else{
                    func[3]='\0';
                    switch((int)func[0]){/*switch case being given the ascii value of the first letter of the function*/
                        case 109:/*'m'*/
                            if(strcmp(func,"mov")==0){
                                value=0;
                                funct=0;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 99:
                            if(strcmp(func,"cmp")==0){
                                value=1;
                                funct=0;
                                ptr+=3;
                            }
                            else if(strcmp(func,"clr")==0){
                                value=5;
                                funct=10;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 97:
                            if(strcmp(func,"add")==0){
                                value=2;
                                funct=10;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 115:
                            if(strcmp(func,"sub")==0){
                                value=2;
                                funct=11;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 110:
                            if(strcmp(func,"not")==0){
                                value=5;
                                funct=11;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 108:
                            if(strcmp(func,"lea")==0){
                                value=4;
                                funct=0;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 105:
                            if(strcmp(func,"inc")==0){
                                value=5;
                                funct=12;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 100:
                            if(strcmp(func,"dec")==0){
                                value=5;
                                funct=13;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 106:
                            if(strcmp(func,"jmp")==0){
                                value=9;
                                funct=10;
                                ptr+=3;
                            }
                            else if(strcmp(func,"jsr")==0){
                                value=9;
                                funct=12;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 98:
                            if(strcmp(func,"bne")==0){
                                value=9;
                                funct=11;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 112:
                            if(strcmp(func,"prn")==0){
                                value=13;
                                funct=0;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        case 114:
                            if(strcmp(func,"rts")==0){
                                value=14;
                                funct=0;
                                ptr+=3;
                            }
                            else if(strcmp(func,"red")==0){
                                value=12;
                                funct=0;
                                ptr+=3;
                            }
                            else{
                                value=16;
                                errorFound=1;
                                printf("Unknown command found in line %d.\n",lineNum);
                            }
                            break;
                        default:
                            value=16;
                            errorFound=1;
                            printf("Unknown command found in line %d.\n",lineNum);
                    }
                }
                instruction[IC].opcode=pow(2,value);
                instruction[IC].funct=funct;
                while(ptr[0]==' '||ptr[0]=='\t')
                    ptr++;
                if(strchr(line,'#')!=NULL&&strchr(strchr(line,'#'),',')!=NULL){/*immidiate type operand*/
                    instruction[IC].source_operand=0;
                    if(strchr(strchr(line,'#'),',')[1]=='-'){
                        instruction[IC].firstword=-1*(strchr(line,'#')[2]-48);
                        if(isdigit(strchr(line,'#')[3])){
                            instruction[IC].firstword*=10;
                            instruction[IC].firstword-=(strchr(line,'#')[3]-48);
                        }
                    }
                    else if(strchr(strchr(line,'#'),',')[1]=='+'){
                        instruction[IC].firstword=(strchr(line,'#')[2]-48);
                        if(isdigit(strchr(line,'#')[3])){
                            instruction[IC].firstword*=10;
                            instruction[IC].firstword+=(strchr(line,'#')[3]-48);
                        }
                    }
                    else{
                        instruction[IC].firstword=(strchr(line,'#')[1]-48);
                        if(isdigit(strchr(line,'#')[2])){
                            instruction[IC].firstword*=10;
                            instruction[IC].firstword+=(strchr(line,'#')[2]-48);
                        }
                    }
                    address++;
                    if(value>=5&&value<=15){
                        errorFound=1;
                        printf("This function doesn't allow 2 operands. Caused in line %d.\n",lineNum);
                    }
                    if(value>=3||value<=15){
                        errorFound=1;
                        printf("This function does not allow this type of source operand . Caused in line %d.\n",lineNum);
                    }
                    oneArgument=0;
                }
                else if(strchr(line,'[')!=NULL&&strchr(strchr(line,'['),',')!=NULL){/*access to list type operand*/
                    instruction[IC].source_operand=2;
                    instruction[IC].firstword=0;
                    numFind=(strchr(line,'[')[1]-48);
                    if(isdigit(strchr(line,'[')[2])){
                        numFind*=10;
                        numFind+=strchr(ptr,'[')[2]-48;
                        if(isdigit(strchr(line,'[')[3])){
                            numFind*=10;
                            numFind+=strchr(line,'[')[3]-48;
                        }
                    }
                    instruction[IC].source_reg=numFind;
                    address+=2;
                    if(value>=5&&value<=15){
                        errorFound=1;
                        printf("This function doesn't allow 2 operands. Caused in line %d.\n",lineNum);
                        printf("This function does not allow this type of source operand . Caused in line %d.\n",lineNum);
                    }
                    oneArgument=0;
                }
                else if(strchr(line,'r')!=NULL&&strchr(strchr(line,'r'),',')){/*register type operand*/
                    instruction[IC].source_operand=3;
                    instruction[IC].source_reg=(strchr(line,'r')[1]-48);
                    if(isdigit(strchr(line,'r')[2])){
                        instruction[IC].source_reg*=10;
                        instruction[IC].source_reg+=strchr(ptr,'r')[2]-48;
                    }
                    if(value>=5&&value<=15){
                        errorFound=1;
                    printf("This function doesn't allow 2 operands. Caused in line %d.\n",lineNum);
                    }
                    if(value>=3&&value<=15){
                        errorFound=1;
                        printf("This function does not allow this type of target operand . Caused in line %d.\n",lineNum);
                    }
                    oneArgument=0;
                }
                else if(strchr(line,',')!=NULL){/*label type operand OR one operand(meaning source operand not given)*/
                    if(ptr[0]!='\n'&&ptr[0]!='\0'){/*label type operand*/
                        instruction[IC].source_operand=1;
                        instruction[IC].firstword=0;
                        address+=2;
                        if(value>=5&&value<=15){
                            errorFound=1;
                            printf("This function doesn't allow 2 operands. Caused in line %d.\n",lineNum);
                            printf("This function does not allow this type of source operand . Caused in line %d.\n",lineNum);
                        }
                    }
                    else{/*one operand given*/
                        instruction[IC].source_operand=0;
                        instruction[IC].source_reg=0;
                        instruction[IC].firstword=0;
                        if(value>=0&&value<=13){
                            errorFound=1;
                            printf("This function requires operands, you have not entered any operands. Caused in line %d.\n",lineNum);
                        }
                    }
                }
                if(strchr(line,',')==NULL){/*Only one operand found, refered as the target operand*/
                    if(strchr(line,'#')!=NULL&&oneArgument==1){/*immidiate type operand*/
                        instruction[IC].target_operand=0;
                        instruction[IC].source_operand=0;
                        instruction[IC].firstword=0;
                        oneArgument=1;
                        if(strchr(line,'#')[1]=='-'){
                            instruction[IC].secondword=-1*(strchr(line,'#')[2]-48);
                            if(isdigit(strchr(line,'#')[3])){
                                instruction[IC].secondword*=10;
                                instruction[IC].secondword-=(strchr(line,'#')[3]-48);
                            }
                        }
                        else if(strchr(line,'#')[1]=='+'){
                            instruction[IC].secondword=(strchr(line,'#')[2]-48);
                            if(isdigit(strchr(line,'#')[3])){
                                instruction[IC].secondword*=10;
                                instruction[IC].secondword+=(strchr(line,'#')[3]-48);
                            }
                        }
                        else{
                            instruction[IC].secondword=(strchr(line,'#')[1]-48);
                            if(isdigit(strchr(line,'#')[2])){
                                instruction[IC].secondword*=10;
                                instruction[IC].secondword+=(strchr(line,'#')[2]-48);
                            }
                        }
                        address++;
                        if(value>=14&&value<=15){
                            errorFound=1;
                            printf("This function doesn't allow any operands. Caused in line %d.\n",lineNum);
                        }
                        if(value!=1&&value!=13){
                            errorFound=1;
                            printf("This function does not allow this type of target operand . Caused in line %d.\n",lineNum);
                        }
                    }
                    else if(strchr(line,'[')!=NULL&&oneArgument==1){/*access to list type operand*/
                        instruction[IC].target_operand=2;
                        instruction[IC].source_operand=0;
                        instruction[IC].firstword=0;
                        instruction[IC].secondword=0;
                        oneArgument=1;
                        numFind=(strchr(line,'[')[2]-48);
                        if(isdigit(strchr(line,'[')[3])){
                            numFind*=10;
                            numFind+=strchr(ptr,'[')[3]-48;
                            if(isdigit(strchr(line,'[')[4])){
                                numFind*=10;
                                numFind+=strchr(line,'[')[4]-48;
                            }
                        }
                        instruction[IC].target_reg=numFind;
                        address+=2;
                        if(value>=14&&value<=15){
                            errorFound=1;
                            printf("This function doesn't allow any operands. Caused in line %d.\n",lineNum);
                            printf("This function does not allow this type of target operand . Caused in line %d.\n",lineNum);
                        }
                    }
                    else if(strchr(line,'r')!=NULL&&oneArgument==1){/*register type operand*/
                        instruction[IC].target_operand=3;
                        instruction[IC].source_operand=0;
                        instruction[IC].firstword=0;
                        oneArgument=1;
                        instruction[IC].target_reg=strchr(line,'r')[1]-48;
                        if(isdigit(strchr(line,'r')[2])){
                            instruction[IC].target_reg*=10;
                            instruction[IC].target_reg+=strchr(ptr,'r')[2]-48;
                        }
                        if(value>=14&&value<=15){
                            errorFound=1;
                            printf("This function doesn't allow any operands. Caused in line %d.\n",lineNum);
                        }
                        if(value==9||value==14||value==15){
                            errorFound=1;
                            printf("This function does not allow this type of target operand . Caused in line %d.\n",lineNum);
                        }
                    }
                    else if(oneArgument==1){/*Meaning there is only one operand(Argument)*/
                        if(ptr[0]!='\n'&&ptr[0]!='\0'&&ptr[0]!='\r'){/*label type operand*/
                            instruction[IC].target_operand=1;
                            instruction[IC].source_operand=0;
                            instruction[IC].firstword=0;
                            instruction[IC].secondword=0;
                            address+=2;
                            if(value>=14&&value<=15){
                                errorFound=1;
                                printf("This function doesn't allow 2 operands. Caused in line %d.\n",lineNum);
                                printf("This function does not allow this type of source operand . Caused in line %d.\n",lineNum);
                            }
                        }
                        else{/*No operands given*/
                            instruction[IC].source_operand=0;
                            instruction[IC].source_reg=0;
                            instruction[IC].firstword=0;
                            instruction[IC].target_operand=0;
                            instruction[IC].target_reg=0;
                            instruction[IC].secondword=0;
                            address--;/*in the last section of this operands code we do address+=2 for opcode and for operands code(if reached here meaning no need for operands code)*/
                            if(value>=0&&value<=13){
                                errorFound=1;
                                printf("This function requires operands, you have not entered any operands. Caused in line %d.\n",lineNum);
                            }
                        }
                    }
                }
                if(strchr(line,',')!=NULL){/*Finding the target operand in case there are 2 operands*/
                    if(strchr(strchr(line,','),'#')!=NULL){/*immidiate type operand*/
                        instruction[IC].target_operand=0;
                        if(strchr(strchr(line,','),'#')[1]=='-'){
                            instruction[IC].secondword=-1*(strchr(strchr(line,','),'#')[2]-48);
                            if(isdigit(strchr(strchr(line,','),'#')[3])){
                                instruction[IC].secondword*=10;
                                instruction[IC].secondword-=(strchr(strchr(line,','),'#')[3]-48);
                            }
                        }
                        else if(strchr(strchr(line,','),'#')[1]=='+'){
                            instruction[IC].secondword=(strchr(strchr(line,','),'#')[2]-48);
                            if(isdigit(strchr(strchr(line,','),'#')[3])){
                                instruction[IC].secondword*=10;
                                instruction[IC].secondword+=(strchr(strchr(line,','),'#')[3]-48);
                            }
                        }
                        else{
                            instruction[IC].secondword=(strchr(strchr(line,','),'#')[1]-48);
                            if(isdigit(strchr(strchr(line,','),'#')[2])){
                                instruction[IC].secondword*=10;
                                instruction[IC].secondword+=(strchr(strchr(line,','),'#')[2]-48);
                            }
                        }
                        address++;
                        if(value>=14&&value<=15){
                            errorFound=1;
                            printf("This function doesn't allow any operands. Caused in line %d.\n",lineNum);
                        }
                        if(value!=1&&value!=13){
                            errorFound=1;
                            printf("This function does not allow this type of target operand . Caused in line %d.\n",lineNum);
                        }
                    }
                    else if(strchr(strchr(line,','),'[')){/*access to list type operand*/
                        instruction[IC].target_operand=2;
                        numFind=(strchr(strchr(line,','),'[')[2]-48);
                        if(isdigit(strchr(strchr(line,','),'[')[3])){
                            numFind*=10;
                            numFind+=strchr(strchr(line,','),'[')[3]-48;
                            if(isdigit(strchr(strchr(line,','),'[')[4])){
                                numFind*=10;
                                numFind+=strchr(strchr(line,','),'[')[4]-48;
                            }
                        }
                        instruction[IC].secondword=numFind;
                        address+=2;
                        if(value>=14&&value<=15){
                            errorFound=1;
                            printf("This function doesn't allow any operands. Caused in line %d.\n",lineNum);
                            printf("This function does not allow this type of target operand . Caused in line %d.\n",lineNum);
                        }
                    }
                    else if(strchr(strchr(line,','),'r')){/*register type operand*/
                        instruction[IC].target_operand=3;
                        instruction[IC].target_reg=(strchr(strchr(line,','),'r')[1]-48);
                        instruction[IC].secondword=0;
                        if(isdigit(strchr(line,'r')[2])){
                            instruction[IC].target_reg*=10;
                            instruction[IC].target_reg+=strchr(ptr,'r')[2]-48;
                        }
                        if(value>=14&&value<=15){
                            errorFound=1;
                            printf("This function doesn't allow any operands. Caused in line %d.\n",lineNum);
                        }
                        if(value==9||value==14||value==15){
                            errorFound=1;
                            printf("This function does not allow this type of target operand . Caused in line %d.\n",lineNum);
                        }
                    }
                    else{/*label type operand*/
                        instruction[IC].target_operand=1;
                        instruction[IC].secondword=0;
                        address+=2;
                        if(value>=14&&value<=15){
                            errorFound=1;
                            printf("This function doesn't allow any operands. Caused in line %d.\n",lineNum);
                            printf("This function does not allow this type of target operand . Caused in line %d.\n",lineNum);
                        }
                    }
                }
                oneArgument=1;
                IC++;
                address+=2;/*one for the function opcode and one for the operands code*/
            }
        }
    }//Part 1 finished!
    fclose(fp);/*
    for(i=0;i<lblAmount;i++){
        printf("%s",labelTable[i].name);
        printf(" ");
        printf("%d",labelTable[i].address);
        printf("\n");
    }*/
    if(!errorFound){
        obj=fopen(path,"w");
        fp=fopen(originalPath,"r");
    }
    IC=0;
    address=100;
    int temporary;
    FILE *ext;
    FILE *ent;
    char lineCode[100];
    int a_r_e;
    int x;
    char codeV2[13];/*Code in unique required base, the 1,4,7,10,13 slots are used for the numbers we get*/
    codeV2[0]='A';
    codeV2[2]='-';
    codeV2[3]='B';
    codeV2[5]='-';
    codeV2[6]='C';
    codeV2[8]='-';
    codeV2[9]='D';
    codeV2[11]='-';
    codeV2[12]='E';
    if(!errorFound){/*if an error was found we do not want to proceed*/
        while(!feof(fp)){
            fgets(line,MAX_LINE_LENGTH,fp);
            x=0;
            if(strstr(line,".data")!=NULL)
                x=1;
            else if(strstr(line,".string")!=NULL)
                x=1;
            else if(strstr(line,".entry")!=NULL)
                x=1;
            else if(strstr(line,".extern")!=NULL)
                x=1;
            if(x){
                for(i=0;i<lblAmount;i++){
                    if(nameCompare(labelTable[i].name,line)==1){
                        if(labelTable[i].loc==2){/*entry label type found, adding label info to .ent file*/
                            if(entryLabelFound==0){
                                strcpy(path,originalPath);
                            char *ptr = path;
                            	while(ptr[0]!='.')
                            		ptr++;
                            	ptr[1] = 'e';
                            	ptr[2] = 'n';
                            	ptr[3] = 't';
                            	ptr[4] = '\0';
                                ent=fopen(path,"w");
                                entryLabelFound=1;
                            }
                            fputs(labelTable[i].name,ent);
                            fputs(",",ent);
                            sprintf(lineCode,"%d",baseAddress(labelTable[i].address));/*Base address*/
                            fputs(lineCode,ent);
                            fputs(",",ent);
                            sprintf(lineCode,"%d",labelTable[i].address%16);/*Current offset*/
                            fputs(lineCode,ent);
                            fputs("\n",ent);
                        }
                    }
                }
            }
            else if(!feof(fp)){
                if(strchr(line,':')!=NULL){/*label found*/
                    for(i=0;i<lblAmount;i++){
                        if(nameCompare(labelTable[i].name,line)==1){
                            if(labelTable[i].loc==2){/*entry label type found, adding label info to .ent file*/
                                if(entryLabelFound==0){
                                    strcpy(path,originalPath);
                                    char *ptr = path;
                                	while(ptr[0]!='.')
                                		ptr++;
                                	ptr[1] = 'e';
                                	ptr[2] = 'n';
                                	ptr[3] = 't';
                                	ptr[4] = '\0';
                                    ent=fopen(path,"w");
                                    entryLabelFound=1;
                                }
                                fputs(labelTable[i].name,ent);
                                fputs(",",ent);
                                sprintf(lineCode,"%d",baseAddress(labelTable[i].address));/*Base address*/
                                fputs(lineCode,ent);
                                fputs(",",ent);
                                sprintf(lineCode,"%d",labelTable[i].address%16);/*Current offset*/
                                fputs(lineCode,ent);
                                fputs("\n",ent);
                            }
                        }
                    }
                }
                sprintf(lineCode,"%d",address);
                fputs(lineCode,obj);
                fputs(" ",obj);
                codeV2[1]=hexa(4);
                codeV2[4]=hexa((int)(instruction[IC].opcode/(int)pow(2,12)));
                codeV2[7]=hexa((int)(instruction[IC].opcode/(int)pow(2,8))%(int)pow(2,4));
                codeV2[10]=hexa((int)(instruction[IC].opcode/(int)pow(2,4))%(int)pow(2,4));
                codeV2[13]=hexa((int)(instruction[IC].opcode%(int)pow(2,4)));
                fputs(codeV2,obj);
                fputs("\n",obj);
                address++;
                if(instruction[IC].opcode!=pow(2,15)&&instruction[IC].opcode!=pow(2,14)){
                    sprintf(lineCode,"%d",address);
                    fputs(lineCode,obj);
                    fputs(" ",obj);
                    codeV2[1]=hexa(4);
                    codeV2[4]=hexa(instruction[IC].funct);
                    codeV2[7]=hexa(instruction[IC].source_reg);
                    codeV2[10]=hexa(instruction[IC].source_operand*4+(int)instruction[IC].target_reg/4);
                    codeV2[13]=hexa(instruction[IC].target_operand+4*(instruction[IC].target_reg%4));
                    fputs(codeV2,obj);
                    fputs("\n",obj);
                    address++;
                }
                if(instruction[IC].source_operand==1||instruction[IC].source_operand==2){
                    temporary=findLabelAddress(lblAmount,labelTable,line,1);
                    if(temporary==0){
                        if(externalLabelFound==0){
                            strcpy(path,originalPath);
                            char *ptr = path;
                        	while(ptr[0]!='.')
                        		ptr++;
                        	ptr[1] = 'e';
                        	ptr[2] = 'x';
                        	ptr[3] = 't';
                        	ptr[4] = '\0';
                            ext=fopen(path,"w");
                            externalLabelFound=1;
                        }
                        
                        for(i=0;i<lblAmount;i++){
                            if(strstr(line,labelTable[i].name)!=NULL)
                                if(strchr(strstr(line,labelTable[i].name),',')!=NULL)
                                    fputs(labelTable[i].name,ext);
                        }
                        fputs(" ",ext);
                        fputs("BASE",ext);
                        fputs(" ",ext);
                        sprintf(lineCode,"%d",address);
                        fputs(lineCode,ext);
                        fputs("\n",ext);
                        for(i=0;i<lblAmount;i++){
                            if(strstr(line,labelTable[i].name)!=NULL)
                                if(strchr(strstr(line,labelTable[i].name),',')!=NULL)
                                    fputs(labelTable[i].name,ext);
                        }
                        fputs(" ",ext);
                        fputs("OFFSET",ext);
                        fputs(" ",ext);
                        sprintf(lineCode,"%d",address+1);
                        fputs(lineCode,ext);
                        fputs("\n",ext);
                        fputs("\n",ext);
                    }
                    if(temporary!=0)
                        a_r_e=1;
                    else
                        a_r_e=0;
                    sprintf(lineCode,"%d",address);
                    fputs(lineCode,obj);
                    fputs(" ",obj);
                    codeV2[1]=hexa(1+a_r_e);
                    codeV2[4]=hexa((int)(baseAddress(temporary)/(int)pow(2,12)));
                    codeV2[7]=hexa((int)(baseAddress(temporary)/(int)pow(2,8))%(int)pow(2,4));
                    codeV2[10]=hexa((int)(baseAddress(temporary)/(int)pow(2,4))%(int)pow(2,4));
                    codeV2[13]=hexa((int)(baseAddress(temporary)%(int)pow(2,4)));
                    fputs(codeV2,obj);
                    fputs("\n",obj);
                    address++;
                    sprintf(lineCode,"%d",address);
                    fputs(lineCode,obj);
                    fputs(" ",obj);
                    codeV2[1]=hexa(1+a_r_e);
                    codeV2[4]=hexa(0);
                    codeV2[7]=hexa(0);
                    codeV2[10]=hexa(0);
                    codeV2[13]=hexa(temporary%16);
                    fputs(codeV2,obj);
                    fputs("\n",obj);
                    address++;
                }
                if(instruction[IC].firstword!=0){
                    sprintf(lineCode,"%d",address);
                    fputs(lineCode,obj);
                    fputs(" ",obj);
                    codeV2[1]=hexa(4);
                    codeV2[4]=hexa((int)(instruction[IC].firstword/(int)pow(2,12)));
                    codeV2[7]=hexa((int)(instruction[IC].firstword/(int)pow(2,8))%(int)pow(2,4));
                    codeV2[10]=hexa((int)(instruction[IC].firstword/(int)pow(2,4))%(int)pow(2,4));
                    codeV2[13]=hexa((int)(instruction[IC].firstword%(int)pow(2,4)));
                    fputs(codeV2,obj);
                    fputs("\n",obj);
                    address++;
                }
                if(instruction[IC].target_operand==1||instruction[IC].target_operand==2){
                    temporary=findLabelAddress(lblAmount,labelTable,line,2);
                    if(temporary==0){
                        if(externalLabelFound==0){
                            strcpy(path,originalPath);
                            char *ptr = path;
                        	while(ptr[0]!='.')
                        		ptr++;
                        	ptr[1] = 'e';
                        	ptr[2] = 'x';
                        	ptr[3] = 't';
                        	ptr[4] = '\0';
                            ext=fopen(path,"w");
                            externalLabelFound=1;
                        }
                        for(i=0;i<lblAmount;i++){
                            if(strstr(line,labelTable[i].name)!=NULL){
                                if(strstr(strchr(line,','),labelTable[i].name)!=NULL){
                                    fputs(labelTable[i].name,ext);
                                    strcpy(name,labelTable[i].name);
                                }
                            }
                        }
                        fputs(" ",ext);
                        fputs("BASE",ext);
                        fputs(" ",ext);
                        sprintf(lineCode,"%d",address);
                        fputs(lineCode,ext);
                        fputs("\n",ext);
                        fputs(name,ext);
                        fputs(" ",ext);
                        fputs("OFFSET",ext);
                        fputs(" ",ext);
                        sprintf(lineCode,"%d",address+1);
                        fputs(lineCode,ext);
                        fputs("\n",ext);
                        fputs("\n",ext);
                    }
                    if(temporary!=0)
                        a_r_e=1;
                    else
                        a_r_e=0;
                    sprintf(lineCode,"%d",address);
                    fputs(lineCode,obj);
                    fputs(" ",obj);
                    codeV2[1]=hexa(1+a_r_e);
                    codeV2[4]=hexa((int)(baseAddress(temporary)/(int)pow(2,12)));
                    codeV2[7]=hexa((int)(baseAddress(temporary)/(int)pow(2,8))%(int)pow(2,4));
                    codeV2[10]=hexa((int)(baseAddress(temporary)/(int)pow(2,4))%(int)pow(2,4));
                    codeV2[13]=hexa((int)(baseAddress(temporary)%(int)pow(2,4)));
                    fputs(codeV2,obj);
                    fputs("\n",obj);
                    address++;
                    sprintf(lineCode,"%d",address);
                    fputs(lineCode,obj);
                    fputs(" ",obj);
                    codeV2[1]=hexa(1+a_r_e);
                    codeV2[4]=hexa(0);
                    codeV2[7]=hexa(0);
                    codeV2[10]=hexa(0);
                    codeV2[13]=hexa(temporary%16);
                    fputs(codeV2,obj);
                    fputs("\n",obj);
                    address++;
                }
                if(instruction[IC].secondword!=0){
                    sprintf(lineCode,"%d",address);
                    fputs(lineCode,obj);
                    fputs(" ",obj);
                    codeV2[1]=hexa(4);
                    codeV2[4]=hexa((int)(instruction[IC].secondword/(int)pow(2,12)));
                    codeV2[7]=hexa((int)(instruction[IC].secondword/(int)pow(2,8))%(int)pow(2,4));
                    codeV2[10]=hexa((int)(instruction[IC].secondword/(int)pow(2,4))%(int)pow(2,4));
                    codeV2[13]=hexa((int)(instruction[IC].secondword%(int)pow(2,4)));
                    fputs(codeV2,obj);
                    fputs("\n",obj);
                    address++;
                }
                IC++;
            }
        }
        for(i=0;i<DC;i++){
            sprintf(lineCode,"%d",address);
            fputs(lineCode,obj);
            fputs(" ",obj);
            codeV2[1]=hexa(4);
            codeV2[4]=hexa((int)(data[i].data/(int)pow(2,12)));
            codeV2[7]=hexa((int)(data[i].data/(int)pow(2,8))%(int)pow(2,4));
            codeV2[10]=hexa((int)(data[i].data/(int)pow(2,4))%(int)pow(2,4));
            codeV2[13]=hexa((int)(data[i].data%(int)pow(2,4)));
            fputs(codeV2,obj);
            fputs("\n",obj);
            address++;
        }
    }
    free(labelTable);
    if(!errorFound){
        fclose(fp);
        fclose(obj);
        if(entryLabelFound!=0)
            fclose(ent);
        if(externalLabelFound!=0)
            fclose(ext);
    }
    return 0;
}