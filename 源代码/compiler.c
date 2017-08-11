#include <stdio.h>
#include <string.h>
//lexical analyzer of c0 compiler
//@2016.11.8
//by XFPlus

//parsing
//@2016.11.16
//by XFPlus

//asm generator
//@2016.11.23
//by XFPlus

//print char
//@2016.12.3
//to be continued.(error)

//@2016.12.12
//fix a bug about vsize which ignored para before.

#define NORW 13 //no. of reserved word
#define NLMAX 10 //max length of number, cause the largest 32-bit number is 4294967296
#define ILMAX 10 //max length of identifier
#define STRMAX 256
#define TMAX 256
#define STMAX 64
#define BTMAX 128
#define ATMAX 128
#define CLMAX 1024
#define ASMLMAX 128

#define VAR_AND_FUN 1
#define VAR_ONLY 2

//ERROR NO. DEFINE
#define INVALID_FUN_NAME    0
#define MISSING_MAIN        1
#define UNDEFINED_IDENT     2
#define MISSING_SEMI        3
#define INCOMPLETE_FILE     4
#define INVALID_CHAR        5
#define FATAL_STR_LENTH     6

#define FATAL_TABLES        9
#define INVALID_CONST_TYPE  10
#define EXPECT_INTEGER      11
#define EXPECT_BECOME       12
#define EXPECT_CHARACTER    13
#define EXPECT_INT_CHAR     14
#define EXPECT_IDENT        15
#define MISSING_RBRACK      16
#define EXPECT_USINTEGER    17
#define INVALID_CODE        18
#define MISSING_RPAREN      19
#define MISSING_LPAREN      20
#define EXPECT_MAIN         21
#define MISSING_RBRACE      22
#define MISSING_LBRACE      23
#define MISSING_STATEMENGT  24
#define EXPECT_IF           25
#define EXPECT_WHILE        26
#define EXPECT_SCANF        27
#define EXPECT_PRINTF       28
#define EXPECT_SWITCH       29
#define EXPECT_NUM          30
#define EXPECT_INT_USINT_CHAR 31
#define MISSING_COLON       32
#define EXPECT_BECOME_LBRACK  33
#define INVALID_VOID_FUN    34
#define INVALID_ASS_CONST   35
#define MISMATCH_PARA       36
#define EMPTY_PRINT         37
#define EMPTY_CASETABLE     38
#define DUP_CASE_VALUE      39
#define REDEC               40
#define EXPECT_FACTOR       41
#define EXPECT_PARA_VAR     42
#define INVALID_RET         43
#define FATAL_CTABLES       44
#define FATAL_ATABLES       45

typedef enum {nul,ident,usinteger,integer,plus,minus,times,slash,//invalid character,identifier,number,+,-,*,/  0-7
            lssym,leqsym,grtsym,geqsym,neqsym,eqlsym,//<,<=,>,>=,!=,==                                          8-13
            lparen,rparen,lbrack,rbrack,lbrace,rbrace,comma,colon,semicolon,becomes,//(,),[,],{,}, , ,;,=       14-23
            intsym,charsym,constsym,voidsym,mainsym,ifsym,whilesym,switchsym,casesym,defaultsym,//int,char,const,void,main,if,while,switch,case,default
            scanfsym,printfsym,returnsym,stringsym,chsym}symbol;//scanf,printf,return,string "x"

typedef enum {inttyp,chartyp,voidtyp}types;
typedef enum {constant,variable,function,parameter,array}object;
typedef enum {add,sub,mult,div,dec,assign,label,j,beq,bne,ble,blt,bgt,bge,read,write,ret,para}operate;

typedef struct {
    char name[ILMAX];
    object obj;
    types type;
    int link;
    int ref;
    int adr;
} record;

typedef struct {
    int size;
    types typ;
    //value
} arecord;

typedef struct {
    int last;
    int lastpar;
    int psize;
    int vsize;
} blockrecord;

typedef struct {
    //ti>0:tables index
    //ti==0:null
    //ti==-1:temp var
    //ti==-2:label
    //ti==-3:integer
    //ti==-4:string
    //ti==-5:char
    operate op;
    char arg1[ILMAX];
    int ti1;
    char arg2[ILMAX];
    int ti2;
    char result[ILMAX];
    int ti3;
} quadruple;

/*
const int norw = 13; //no. of reserved words.
const int nlmax = 10; //max length of number, cause the largest 32-bit number is 4294967296
const int ilmax = 10; //max length of identifier
const int strmax = 256;
*/
char ch=' ';
char id[ILMAX];
char word[NORW][ILMAX];
char str[STRMAX];
char line[STRMAX]="";
char tempi1[ILMAX];
char tempi2[ILMAX];
char tempi3[ILMAX];
char tempt1[ILMAX]="t";
char tempt2[ILMAX]="t";
char tempt3[ILMAX]="t";

int ischar[1024]={0};
symbol wsym[NORW];
symbol ssym[128];
symbol sym;
types typ;
int num=0,//读取到的数字的值
    ec=0,//error count
    cc=0,//char count
    lc=1,//line count
    err=0,//error no.
    ti=0,//table index
    bi=0,//block table index
    ai=0,//array table index
    ci=0,//const table index
    ii=0,//intercode index
    si=0,//str table index
    labno=0,
    glvi=-1,
    tempvar=0,
    base=-1,
    offset=0,
    varfinish=0,
    fundecable=1,
    retflag=0,
    decflag=0;//declaration flag


int lastuse[1024]={0};
int treg[8]={0};
int skipflag[64]={0};

record tables[TMAX];
arecord atables[ATMAX];
blockrecord blocktables[BTMAX];
int constable[TMAX];
char strtables[STMAX][STRMAX];
quadruple intercode[CLMAX];

char data[CLMAX][2*ASMLMAX]={0};
char text[CLMAX][ASMLMAX]={0};

FILE *fin;

void info();
char* itoa(int i,char *res);
int atoi(char *a);

void getsym();
void getch();

int loc(char name[ILMAX]);
void getsemi();

void condec();//<常量说明>
void condefine();//<常量定义>

void vardec();//<变量说明>
void vardefine();//<变量定义>
void mainfun();//<主函数>
void fundec();//<函数定义> 包括有/无返回值函数,对参数的识别和填表，在该函数内分析完常量/变量声明
void cpdstatement();//<复合语句>
void statementlist();//<语句列>
void statement();//<语句>
void ifstatement();//<条件语句>
void whilestatement();//<循环语句>
void funcall();//<有/无返回值函数调用语句>
void assignstatement();//<赋值语句>
void readstatement();
void writestatement();
void condstatement();//<情况语句>
void expression();//<表达式>
void term();//<项>
void factor();//<因子>

void enter(char name[ILMAX],object obj);//填符号表
void enterconst(int value);//填常量表
void enterarray(int size);//填数组表

void printtables();
void printintercode();

void error();//错误处理

void skip();//进行跳读
void enterquadruple(operate op,char arg1[ILMAX],int ti1,char arg2[ILMAX],int ti2,char res[ILMAX],int ti3);
void optimize();//对中间代码进行优化
void exasm();//由中间代码导出目标汇编码
int fetchreg(int t,int i);//返回一个寄存器用于存储临时变量"t"+i
int findreg(int t);//找到临时变量在临时寄存器中的位置

int main()// <程序>
{
    int i,count,mainflag=0;
    char filename[64];
    printf("Please input source file's name(limited length:64):");
    scanf("%s",filename);
    if((fin=fopen(filename,"rt+"))==NULL){
        printf("open file \"%s\" failed.\n",filename);
        return 0;
    }
    //printf("initialize wsym...\n");
    wsym[0]=casesym;
    wsym[1]=charsym;
    wsym[2]=constsym;
    wsym[3]=defaultsym;
    wsym[4]=ifsym;
    wsym[5]=intsym;
    wsym[6]=mainsym;
    wsym[7]=printfsym;
    wsym[8]=returnsym;
    wsym[9]=scanfsym;
    wsym[10]=switchsym;
    wsym[11]=voidsym;
    wsym[12]=whilesym;

    //printf("initialize word...\n");
    strcpy(word[0],"case");
    strcpy(word[1],"char");
    strcpy(word[2],"const");
    strcpy(word[3],"default");
    strcpy(word[4],"if");
    strcpy(word[5],"int");
    strcpy(word[6],"main");
    strcpy(word[7],"printf");
    strcpy(word[8],"return");
    strcpy(word[9],"scanf");
    strcpy(word[10],"switch");
    strcpy(word[11],"void");
    strcpy(word[12],"while");

    //printf("initialize ssym...\n");
    ssym[-1]=1;
    for(i=0;i<128;i++)
        ssym[i]=nul;
    ssym['+']=plus;
    ssym['-']=minus;
    ssym['*']=times;
    ssym['/']=slash;
    ssym['(']=lparen;
    ssym[')']=rparen;
    ssym['[']=lbrack;
    ssym[']']=rbrack;
    ssym['{']=lbrace;
    ssym['}']=rbrace;
    ssym[',']=comma;
    ssym[':']=colon;
    ssym[';']=semicolon;
    ssym['=']=becomes;
    ssym['<']=lssym;
    ssym['>']=grtsym;

    skipflag[rbrace]+=1;

    getsym();
    if(sym==constsym){
        condec();
    }
    if(sym==intsym||sym==charsym){
        vardec();
    }
    //info();
    if(sym==voidsym){
        typ=voidtyp;
        skipflag[rbrace]+=1;

        getsym();
        if(sym==ident){
            enter(id,function);
            enterquadruple(j,"main",0,"",0,"",0);
            if(base==-1)base=ti-1;
            if(glvi==-1)glvi=ti-1;
            getsym();
            fundec();
        }
        else if(sym==mainsym){
            mainflag=1;
            mainfun();
            //finish
        }
        else {
            //error 无效的函数名 void后面需要跟ident或main
            error(INVALID_FUN_NAME);
            skip();
        }

        skipflag[rbrace]-=1;
    }
    fundecable=0;
    if(!mainflag){
        if(sym==mainsym){
            mainfun();
        }
        else{
            //error 应该为main或函数声明
            error(MISSING_MAIN);
        }

    }
    fclose(fin);

    //printtables();

    //printintercode();

    if(ec){
        return 0;
    }
    exasm();

    skipflag[rbrace]-=1;
    return 0;
}

int loc(char name[ILMAX])
{
    int i=blocktables[bi].last;
    strcpy(tables[0].name,name);
    while(strcmp(tables[i].name,name)!=0){
        i=tables[i].link;
    }
    if(i==0){
        //error 未定义的标识符 第一次出现在该位置
        error(UNDEFINED_IDENT);
        enter(name,variable);
    }
    return i;
}

void getsemi()
{
    if(sym==semicolon){
        //right
        getsym();
    }
    else {
        //error 应该是个 ;
        //printf("sym:%d\n",sym);
        error(MISSING_SEMI);
//        skipflag[semicolon]+=1;
//        skip();
//        skipflag[semicolon]-=1;
    }
}

void getch()
{
//    while(line[cc]=='\n'||cc==STRMAX||line[cc]=='\0'){//循环跳过\n
//        lc = line[cc]=='\n'?lc+1:lc;
//        if(fgets(line,STRMAX,fin)==NULL){
//            error 已经读到文件末尾
//            error(INCOMPLETE_FILE);
//            exit(0);
//        }
//        cc = 0;
//    }
//    ch=line[cc++];
    if(line[cc]=='\0'&&cc==0&&lc!=1){
        error(INCOMPLETE_FILE);
        exit(0);
    }
    if(line[cc]=='\0'){
        if(fgets(line,STRMAX,fin)==NULL){
            //error(INCOMPLETE_FILE);
            //exit(0);
        }
        cc=0;
    }
    ch=line[cc++];
    if(ch=='\n')lc++;
}

void getsym()
{
    int i,j,k;//
    char a[10];
    while(ch==' '||ch=='\t'||ch=='\n'){
        getch();
    }
    if(ch=='_'||('a'<=ch&&ch<='z')||('A'<=ch&&ch<='Z')){//identifier or reserved words
        k=0;
        do{
            if(k<ILMAX)//暂时省略标识符中超出长度的部分，防止溢出，但又不能不读多出来的部分，免得影响后面的词法分析
                a[k++]=ch;
            getch();
        }while(ch=='_'||('a'<=ch&&ch<='z')||('A'<=ch&&ch<='Z')||('0'<=ch&&ch<='9'));
        a[k]='\0';

        strcpy(id,a);
        for(i=0,j=NORW-1;i<=j;){
            k=(i+j)/2;
            if(strcmp(id,word[k])<=0)
                j=k-1;
            if(strcmp(id,word[k])>=0)
                i=k+1;
        }
        if(i-1>j)
            sym=wsym[k];
        else
            sym=ident;
        return;
    }
    else if('0'<=ch&&ch<='9'){
        num=0;
        if(ch=='0'){//integer 会有多余的0
            sym=integer;
            getch();
            return;
        }
        k=0;
        do{//usinteger
            k++;
            num=num*10+ch-'0';
            getch();
        }while('0'<=ch&&ch<='9'&&k<=NLMAX);
        sym=usinteger;
        return;
    }
//    else if(ch=='+'||ch=='-'){
//        k=ch;
//        id[0]=ch;
//        id[1]='\0';
//        getch();
//        if('1'<=ch&&ch<='9'){//integer
//            num=0;
//            do{
//                num=num*10+ch-'0';
//                getch();
//            }while('0'<=ch&&ch<='9');
//            if(k=='-')
//                num=-num;
//            sym=integer;
//            return;
//        }
//        sym=(k=='+'? plus:(k=='-' ? minus:nul));//minus,plus
//    }
    else if(ch=='<'){
        k=ch;
        id[0]='<';
        id[1]=id[2]='\0';
        getch();
        if(ch=='='){
            sym=leqsym;
            id[1]='=';
            getch();
        }
        else sym=lssym;
    }
    else if(ch=='>'){
        k=ch;
        id[0]='>';
        id[1]=id[2]='\0';
        getch();
        if(ch=='='){
            sym=geqsym;
            id[1]='=';
            getch();
        }
        else sym=grtsym;
    }
    else if(ch=='!'){
        k=ch;
        id[0]='!';
        id[1]=id[2]='\0';
        getch();
        if(ch=='='){
            id[1]='=';
            sym=neqsym;
            getch();
        }
        else sym=nul;
    }
    else if(ch=='='){
        k=ch;
        id[0]='=';
        id[1]=id[2]='\0';
        getch();
        if(ch=='='){
            id[1]='=';
            sym=eqlsym;
            getch();
        }
        else sym=becomes;
    }
    else if(ch=='"'){//如果超长怎么办
        k=0;
        str[0]='\0';
        getch();
        while(ch!='"'&&k<STRMAX){
            if(ch!=32&&ch!=33&&!(ch<=126&&ch>=35)){
                //error 无效的字符
                //printf("invalid character.\n");
                //continue;
                error(INVALID_CHAR);
                getch();
            }
            if(ch=='\\'){//处理\不产生转义
                str[k++]=ch;
            }
            str[k++]=ch;
            getch();
        }
        if(ch=='"')getch();
		else {
			//error 超出长度
			error(FATAL_STR_LENTH);
			while(ch!='\"')getch();
			getch();
		}
        str[k]='\0';
        //printf("str :%s\n",str);
        sym=stringsym;
    }
    else if(ch=='\''){//超过两个字符怎么办
        getch();
        if(ch=='_'||('a'<=ch&&ch<='z')||('A'<=ch&&ch<='Z')||('0'<=ch&&ch<='9')||ch=='+'||ch=='-'||ch=='*'||ch=='/')
            num=ch;
        else {
            error(INVALID_CHAR);
        }
        getch();
        if(ch=='\''){
            sym=chsym;
            getch();
        }
        else sym=ssym['\''];
        return;
    }
    else if(ch==-1){//EOF
        sym=nul;
        id[0]=ch;
        id[1]='\0';
    }
    else {
        sym=ssym[ch];
        id[0]=ch;
        id[1]='\0';
        getch();
    }
}
//
//int dechead(int mode)//暂用于变量声明和函数声明
//{
//    if(mode==VAR_AND_FUN){//函数声明和变量声明
//        if(sym==intsym||sym==charsym||sym==voidsym){
//            typ=(sym==intsym?inttyp:(sym==charsym?chartyp:voidtyp));
//            getsym();
//            if(sym==ident){
//                //var/function
//                getsym();
//                if(sym==lparen){//
//                    fundec();
//                }
//            }
//            else if(sym==mainsym){//主函数
//                if(typ==voidtyp){
//                    getsym();
//                    mainfun();
//                }
//                else{
//                    //error main函数只能是void main
//                }
//            }
//            else {
//                //error 应该是ident或者void main
//            }
//        }
//    }
//    else if(mode==VAR_ONLY){
//
//    }
//}

void enter(char name[ILMAX],object obj)
{
    if(ti==TMAX){
        //error 溢出
        error(FATAL_TABLES);
        return;
    }

    int i=obj==function?base:ti;
    strcpy(tables[0].name,name);
    while(strcmp(tables[i].name,name)!=0){
        i=tables[i].link;
    }
    if(i!=0&&i>glvi){
        error(REDEC);
        //return;
    }

    strcpy(tables[++ti].name,name);
    tables[ti].obj=obj;
    tables[ti].link=ti-1;
    tables[ti].type=typ;
}

void enterconst(int value)
{
    if(ci==TMAX){
        error(FATAL_CTABLES);
        return;
    }
    constable[++ci]=value;
}

void enterarray(int size)
{
    if(ai==ATMAX){
        error(FATAL_ATABLES);
        return;
    }
    atables[++ai].size=size;
    atables[ai].typ=typ;
}

void condec()
{
    skipflag[constsym]+=1;
    while(sym==constsym){
        getsym();
        condefine();
        if(sym==semicolon){
            //right
            getsym();
        }
        else {
            //error 应该是;
            error(MISSING_SEMI);
            skipflag[semicolon]+=1;
            skip();
            skipflag[semicolon]-=1;
        }
    }
    skipflag[constsym]-=1;
    //printf("常量声明\n");
}

void condefine()
{
    skipflag[semicolon]+=1;
    skipflag[intsym]+=1;
    skipflag[charsym]+=1;
    if(sym!=intsym&&sym!=charsym){
        //error 需要是常量定义
        error(INVALID_CONST_TYPE);
        skip();
    }
    if(sym==intsym){
        typ=inttyp;
        getsym();
        if(sym==ident){
            //填表
            enter(id,constant);

            getsym();
            if(sym==becomes){//=
                getsym();
                int sign=1;
                if(sym==minus||sym==plus){
                    if(sym==minus)sign=-1;
                    getsym();
                }
                if(sym==integer||sym==usinteger){//整数
                    enterconst(num*sign);
                    tables[ti].adr=ci;

                    enterquadruple(dec,itoa(num*sign,tempi1),-3,"",0,tables[ti].name,ti);

                    getsym();
                }
                else{
                    //error 需要是个整数
                    error(EXPECT_INTEGER);
                    skip();
                }
            }
            else{
                //error 需要赋值
                error(EXPECT_BECOME);
                skip();
            }
        }
        else {
            error(EXPECT_IDENT);
        }
        while(sym==comma){
            getsym();
            if(sym==ident){
                //填表
                enter(id,constant);

                getsym();
                int sign=1;

                if(sym==becomes){//=
                    getsym();

                    if(sym==minus||sym==plus){
                        if(sym==minus)sign=-1;
                        getsym();
                    }
                    if(sym==integer||sym==usinteger){//整数
                        enterconst(num*sign);
                        tables[ti].adr=ci;

                        enterquadruple(dec,itoa(num*sign,tempi1),-3,"",0,tables[ti].name,ti);
                        getsym();
                    }
                    else{
                        //error 需要是个整数
                        error(EXPECT_INTEGER);
                        skip();
                    }
                }
                else{
                    //error 需要赋值
                    error(EXPECT_BECOME);
                    skip();
                }
            }
            else {
                error(EXPECT_IDENT);
            }
        }
    }
    else if(sym==charsym){
        typ=chartyp;
        getsym();
        if(sym==ident){
            //填表
            enter(id,constant);

            getsym();
            if(sym==becomes){//=
                getsym();
                if(sym==chsym){//字符
                    enterconst(num);
                    tables[ti].adr=ci;

                    strcpy(tempi1,"' '");
                    tempi1[1]=num;
                    enterquadruple(dec,tempi1,-5,"",0,tables[ti].name,ti);
                    getsym();
                }
                else{
                    //error 需要是个字符
                    error(EXPECT_CHARACTER);
                    skip();
                }
            }
            else{
                //error 需要赋值
                error(EXPECT_BECOME);
                skip();
            }
        }
        else {
            error(EXPECT_IDENT);
        }
        while(sym==comma){
            getsym();
            if(sym==ident){
                //填表
                enter(id,constant);

                getsym();
                if(sym==becomes){//=
                    getsym();
                    if(sym==chsym){//字符
                        enterconst(num);
                        tables[ti].adr=ci;

                        strcpy(tempi1,"' '");
                        tempi1[1]=num;
                        enterquadruple(dec,tempi1,-5,"",0,tables[ti].name,ti);
                        getsym();
                    }
                    else{
                        //error 需要是个整数
                        error(EXPECT_CHARACTER);
                        skip();
                    }
                }
                else{
                    //error 需要赋值
                    error(EXPECT_BECOME);
                    skip();
                }
            }
            else {
                error(EXPECT_IDENT);
            }
        }
    }
    skipflag[semicolon]-=1;
    skipflag[intsym]-=1;
    skipflag[charsym]-=1;
}

void vardec()
{
    skipflag[intsym]+=1;
    skipflag[charsym]+=1;
    while(sym==intsym||sym==charsym){
        vardefine();
        if(varfinish){
            //标记在第一次遇到函数声明即可恢复
            varfinish=0;
            //return;
        }
        //else
        if(sym==semicolon){
            getsym();
        }
        else if(sym!=mainsym){
            //vardefine调用fundec最终以main结尾
            error(MISSING_SEMI);
            skipflag[semicolon]+=1;
            skip();
            skipflag[semicolon]-=1;
        }
    }
    skipflag[intsym]-=1;
    skipflag[charsym]-=1;
    //printf("变量声明\n");
}

void vardefine()
{
    skipflag[intsym]+=1;
    skipflag[charsym]+=1;
    skipflag[semicolon]+=1;
    if(sym!=intsym&&sym!=charsym){
        //error 应该是类型
        error(EXPECT_INT_CHAR);
        skip();
    }
    else if(sym==intsym){
        typ=inttyp;
        getsym();
        if(sym!=ident){
            //error 应该是个标识符
            error(EXPECT_IDENT);
            skipflag[ident]+=1;
            skip();
            skipflag[ident]-=1;
        }
        //enter
        enter(id,variable);
        tables[ti].adr=offset;

        getsym();
        if(sym==lparen){
            if(fundecable){
                varfinish=1;
                if(base==-1)base=ti-1;

                enterquadruple(j,"main",0,"",0,"",0);
                if(glvi==-1)glvi=ti-1;
                fundec();
                return;
            }
            else {
                error(MISSING_SEMI);
            }
        }
        else if(sym==lbrack){//array
            tables[ti].obj=array;
            getsym();
            if(sym==usinteger){
                //enter array tables
                enterarray(num);
                tables[ti].ref=ai;
                offset=offset+4*num;
                blocktables[bi].vsize+=4*num;

                enterquadruple(dec,"",0,"",0,tables[ti].name,ti);

                getsym();
                if(sym==rbrack){
                    //正确读完这个数组声明
                    getsym();
                }
                else{
                    //error 应该是个右中括号
                    error(MISSING_RBRACK);
                    skipflag[rbrack]+=1;
                    skip();
                    skipflag[rbrack]-=1;
                }
            }
            else{
                //error 应该是个无符号数
                error(EXPECT_USINTEGER);
                skipflag[usinteger]+=1;
                skip();
                skipflag[usinteger]-=1;
            }
        }
        else if(sym==comma||sym==semicolon){
            //
            enterquadruple(dec,"",0,"",0,tables[ti].name,ti);
            offset+=4;
            blocktables[bi].vsize+=4;
        }
        else {
            //error 应该是( [ , ;  EXPECT_SEMI
            error(MISSING_SEMI);
            skip();
        }
    }
    else if(sym==charsym){
        typ=chartyp;

        getsym();
        if(sym!=ident){
            //error 应该是个标识符
            error(EXPECT_IDENT);
            skipflag[ident]+=1;
            skip();
            skipflag[ident]-=1;
        }
        //enter
        enter(id,variable);
        tables[ti].adr=offset;

        getsym();
        if(sym==lparen&&fundecable){
            varfinish=1;
            if(base==-1)base=ti-1;

            enterquadruple(j,"main",0,"",0,"",0);
            if(glvi==-1)glvi=ti-1;
            fundec();
            return;
        }
        else if(sym==lbrack){//array
            tables[ti].obj=array;
            getsym();
            if(sym==usinteger){
                //enter array tables
                enterarray(num);
                tables[ti].ref=ai;
                offset+=4*num;
                blocktables[bi].vsize+=4*num;

                enterquadruple(dec,"",0,"",0,tables[ti].name,ti);

                getsym();
                if(sym==rbrack){
                    //正确读完这个数组声明
                    getsym();
                }
                else{
                    //error 应该是个右中括号
                    error(MISSING_RBRACK);
                    skipflag[rbrack]+=1;
                    skip();
                    skipflag[rbrack]-=1;
                }
            }
            else{
                //error 应该是个无符号数
                error(EXPECT_USINTEGER);
                skipflag[usinteger]+=1;
                skip();
                skipflag[usinteger]-=1;
            }
        }
        else if(sym==comma||sym==semicolon){
            //
            enterquadruple(dec,"",0,"",0,tables[ti].name,ti);
            offset+=4;
            blocktables[bi].vsize+=4;
        }
        else {
            //error 应该是( [ , ;
            error(MISSING_SEMI);
            skip();
        }
    }
    while(sym==comma){
        getsym();
        //printf("after comma :%s\n",id);
        if(sym!=ident){
            //error 应该是个标识符
            error(EXPECT_IDENT);
            skipflag[ident]+=1;
            skip();
            skipflag[ident]-=1;
        }
        //enter
        enter(id,variable);
        tables[ti].adr=offset;

        getsym();
        if(sym==lbrack){//array
            tables[ti].obj=array;
            getsym();
            if(sym==usinteger){
                //enter array tables
                enterarray(num);
                tables[ti].ref=ai;
                blocktables[bi].vsize+=4*num;
                offset+=4*num;

                enterquadruple(dec,"",0,"",0,tables[ti].name,ti);

                getsym();
                if(sym==rbrack){
                    //正确读完这个数组声明
                    getsym();
                }
                else{
                    //error 应该是个右中括号
                    error(MISSING_RBRACK);
                    skipflag[rbrack]+=1;
                    skip();
                    skipflag[rbrack]-=1;
                }
            }
            else{
                //error 应该是个无符号数
                error(EXPECT_USINTEGER);
                skipflag[usinteger]+=1;
                skip();
                skipflag[usinteger]-=1;
            }
        }
        else {
            enterquadruple(dec,"",0,"",0,tables[ti].name,ti);
            blocktables[bi].vsize+=4;
            offset+=4;
        }
    }
    skipflag[intsym]-=1;
    skipflag[charsym]-=1;
    skipflag[semicolon]-=1;
}

void mainfun()//begin with main
{
    skipflag[rbrace]+=1;
    enter("main",function);
    if(glvi==-1)glvi=ti-1;
    if(base==-1)base=ti-1;
    tables[ti].link=base;
    base=ti;
    blocktables[++bi].last=ti;
    blocktables[bi].lastpar=ti;
    blocktables[bi].vsize=8;
    tables[ti].ref=bi;
    retflag=0;

    enterquadruple(label,"main",ti,"",0,"",0);

    if(sym==mainsym){
        getsym();
        if(sym==lparen){
            getsym();
            if(sym==rparen){
                getsym();
                if(sym==lbrace){
                    getsym();
                    cpdstatement();
                    if(sym==rbrace){
                        if(!retflag){
                            enterquadruple(ret,"",0,"",0,"",0);
                        }
                        //main over
						do {
							ch=fgetc(fin);
						}while(ch==' '&&ch=='\t'&&ch=='\n');
						if(ch!=EOF){
							//error 或许后面还有东西呢
							error(INVALID_CODE);
						}
                        return;
                    }
                    else {
                        //error missing rbrace
                        error(MISSING_RBRACE);
                        skip();
                    }
                }
                else {
                    //error missing lbrace
                    error(MISSING_LBRACE);
                    skip();
                }
            }
            else {
                //error 应该是)
                error(MISSING_RPAREN);
                skip();
            }
        }
        else {
            //error 应该是(
            error(MISSING_LPAREN);
            skip();
        }
    }
    else {
        //error 应该是main
        error(EXPECT_MAIN);
        skip();
    }
    skipflag[rbrace]-=1;
    //printf("主函数\n");
}

void fundec()//begin with ( and this function has been recorded in tables
{
    if(glvi==-1)glvi==ti-1;
    skipflag[rbrace]+=1;
    tables[ti].obj=function;
    tables[ti].link=base;
    tables[ti].adr=-4;
    base=ti;
    retflag=0;

    enterquadruple(label,tables[ti].name,ti,"",0,"",0);
    enterquadruple(dec,"",0,"",0,tables[ti].name,ti);
    //还应该保存$31,放到翻译里面

    blocktables[++bi].last=ti;
    blocktables[bi].lastpar=ti;
    blocktables[bi].vsize=8;
    offset=0;
    tables[ti].ref=bi;

    //printf("fundec start :%s\n",id);
    if(sym==lparen){
        getsym();
        if(sym==intsym||sym==charsym){
            typ=sym==intsym?inttyp:chartyp;
            getsym();
            if(sym==ident){
                //参数列表
                enter(id,parameter);
                blocktables[bi].vsize+=4;

                tables[ti].adr=offset;
                offset+=4;
                enterquadruple(dec,"",0,"",0,tables[ti].name,ti);

                getsym();
                while(sym==comma){
                    //参数列表
                    getsym();
                    if(sym==intsym||sym==charsym){
                        typ=sym==intsym?inttyp:chartyp;

                        getsym();
                        if(sym==ident){
                            //enter para
                            enter(id,parameter);
                            blocktables[bi].vsize+=4;
                            tables[ti].adr=offset;
                            offset+=4;
                            enterquadruple(dec,"",0,"",0,tables[ti].name,ti);

                            getsym();
                        }
                        else {
                            //error 应该是个标识符
                            error(EXPECT_IDENT);
                            skipflag[comma]+=1;
                            skip();
                            skipflag[comma]-=1;
                        }
                    }
                    else {
                        //error 应该是个int/char
                        error(EXPECT_INT_CHAR);
                        skipflag[comma]+=1;
                        skip();
                        skipflag[comma]-=1;
                    }
                }
                blocktables[bi].last=ti;
                blocktables[bi].lastpar=ti;

            }
        }
        offset+=8;//return address  last base
        if(sym==rparen){
            getsym();
            if(sym==lbrace){
                fundecable=0;
                getsym();
                cpdstatement();
                if(sym==rbrace){
                    fundecable=1;
                    //right
                    //if(!retflag)

                        enterquadruple(ret,"",0,"",0,"",0);

                    getsym();
                    if(sym==intsym||sym==charsym||sym==voidsym){
                        typ=sym==intsym?inttyp:(sym==charsym?chartyp:voidtyp);
                        getsym();
                        if(sym==ident){
                            enter(id,function);
                            getsym();
                            if(sym==lparen){
                                fundec();
                            }
                            else {
                                //error 应该是函数声明
                                error(MISSING_LPAREN);
                                skip();
                            }
                        }
                        else if(sym==mainsym&&typ==voidtyp){//main函数
                            return;
                        }
                        else {
                            //error 应该是函数声明或者main函数
                            error(MISSING_MAIN);
                        }
                    }
                    else {
                        //返回去当作main处理

                    }
                }
                else {
                    //error 应该是}
                    error(MISSING_RBRACE);
                    skip();
                }
            }
            else {
                //error 应该是{
                error(MISSING_LBRACE);
                skip();
            }
        }
    }
    else {
        //函数声明少了(
        error(MISSING_LPAREN);
    }
    skipflag[rbrace]-=1;
    //printf("函数声明\n");
    //printf("fundec finish :%s\n",id);
}

void cpdstatement()
{
    skipflag[semicolon]+=1;
    //printf("cpd\tid:%s\n",id);
    if(sym==constsym){
        condec();
    }
    if(sym==intsym||sym==charsym){
        vardec();
        if(sym==semicolon){
            getsym();
        }
    }

    blocktables[bi].last=ti;

    //printf("cpd\tid:%s\n",id);
    statementlist();
    skipflag[semicolon]-=1;
    //printf("复合语句\n");
}

void statementlist()
{
    skipflag[ifsym]+=1;
    skipflag[whilesym]+=1;
    skipflag[lbrace]+=1;
    skipflag[ident]+=1;
    skipflag[scanfsym]+=1;
    skipflag[printfsym]+=1;
    skipflag[switchsym]+=1;
    skipflag[returnsym]+=1;
    skipflag[semicolon]+=1;

    while(sym==ifsym||sym==whilesym||sym==lbrace||sym==ident||
          sym==scanfsym||sym==printfsym||sym==switchsym||sym==returnsym||sym==semicolon){
        statement();
    }
    skipflag[ifsym]-=1;
    skipflag[whilesym]-=1;
    skipflag[lbrace]-=1;
    skipflag[ident]-=1;
    skipflag[scanfsym]-=1;
    skipflag[printfsym]-=1;
    skipflag[switchsym]-=1;
    skipflag[returnsym]-=1;
    skipflag[semicolon]-=1;
    //printf("语句列\n");
}

void statement()
{
    int i;
    if(sym==ifsym){
        ifstatement();
    }
    else if(sym==whilesym){
        whilestatement();
    }
    else if(sym==lbrace){
        getsym();
        statementlist();
        if(sym==rbrace){
            //right
            getsym();
        }
        else {
            //error 应该是个}
            error(MISSING_RBRACE);
        }
    }
    else if(sym==ident){
        i=loc(id);

        if(tables[i].obj==function){
            funcall();
            getsemi();
        }
        else {
            assignstatement();
            getsemi();
        }
    }
    else if(sym==scanfsym){
        readstatement();
        getsemi();
    }
    else if(sym==printfsym){
        writestatement();
        getsemi();
    }
    else if(sym==switchsym){
        condstatement();
    }
    else if(sym==returnsym){
        getsym();
        if(sym==semicolon){
            //return over
            retflag=1;
            enterquadruple(ret,"",0,"",0,"",0);
        }
        else if(sym==lparen){
            getsym();
            expression();

            if(tables[base].type==voidtyp){
                error(INVALID_RET);
            }

            enterquadruple(ret,strcat(tempt1,itoa(tempvar,tempi1)),-1,"",0,"",0);
            tempt1[1]='\0';
            retflag=1;
            if(sym==rparen){
                getsym();
            }
            else {
                //error 应该是)
                error(MISSING_RPAREN);
            }
        }
        else {
            //error 应该是(表达式)
            error(MISSING_LPAREN);
        }
        getsemi();
        //printf("返回语句\n");
    }
    else if(sym==semicolon){
        getsemi();
    }
    else {
        //error 应该有语句
        error(MISSING_STATEMENGT);
    }
    //printf("刚刚是个语句\n");
}

void ifstatement()
{
    int finadr,startadr,left;
    char lab[ILMAX]="lb";
    strcat(lab,itoa(++labno,tempi1));
    operate tempop;
    //printf("if start :%s\n",id);
    if(sym!=ifsym){
        //error 应该是if
        error(EXPECT_IF);
    }
    getsym();
    if(sym==lparen){
        getsym();
        expression();
        left=tempvar;
        if(sym!=rparen){
            //relation op
            switch(sym){
                case lssym:tempop=bge;break;
                case leqsym:tempop=bgt;break;
                case grtsym:tempop=ble;break;
                case geqsym:tempop=blt;break;
                case neqsym:tempop=beq;break;
                case eqlsym:tempop=bne;break;
            }
            getsym();
            //
            expression();

            enterquadruple(tempop,strcat(tempt1,itoa(left,tempi1)),-1,strcat(tempt2,itoa(tempvar,tempi2)),-1,lab,-2);
            startadr=ii;
            tempt1[1]='\0';
            tempt2[1]='\0';
        }
        else {
            enterquadruple(beq,strcat(tempt1,itoa(left,tempi1)),-1,"0",-3,lab,-2);
            tempt1[1]='\0';
            startadr=ii;
        }
        getsym();
        //printf("test: %s\n",id);
        //relation confirm
        statement();
        enterquadruple(label,lab,-2,"",0,"",0);
    }
    else {
        //error 应该是(
        error(MISSING_LPAREN);
    }
    //printf("if finish :%s\n",id);
    //printf("条件语句\n");
}

void whilestatement()
{
    int finadr,startadr,left;
    char labs[ILMAX]="lb";
    char labf[ILMAX]="lb";
    strcat(labs,itoa(++labno,tempi1));
    strcat(labf,itoa(++labno,tempi1));
    operate tempop;
    //printf("while start :%s\n",id);
    if(sym!=whilesym){
        //error 应该是while
        error(EXPECT_WHILE);
    }
    enterquadruple(label,labs,-2,"",0,"",0);
    getsym();
    if(sym==lparen){
        getsym();
        expression();
        left=tempvar;
        if(sym!=rparen){
            //relation op
            switch(sym){
                case lssym:tempop=bge;break;
                case leqsym:tempop=bgt;break;
                case grtsym:tempop=ble;break;
                case geqsym:tempop=blt;break;
                case neqsym:tempop=beq;break;
                case eqlsym:tempop=bne;break;
                default:break;
            }
            getsym();
            //
            expression();

            enterquadruple(tempop,strcat(tempt1,itoa(left,tempi1)),-1,strcat(tempt2,itoa(tempvar,tempi1)),-1,labf,-2);
            startadr=ii;
            tempt1[1]='\0';
            tempt2[1]='\0';
            if(sym==rparen){
                //right
                getsym();
            }
        }
        else if(sym==rparen){
            //
            enterquadruple(beq,strcat(tempt1,itoa(left,tempi1)),-3,"0",-3,labf,-2);
            tempt1[1]='\0';
            startadr=ii;
            getsym();
        }
        //relation confirm
        statement();
        enterquadruple(j,labs,-2,"",0,"",0);
        enterquadruple(label,labf,-2,"",0,"",0);
    }
    else {
        //error 应该是(
        error(MISSING_LPAREN);
    }
    //printf("循环语句\n");
    //printf("while finish :%s\n",id);
}

void funcall()//四元式怎么处理参数
{
    int i=loc(id);
    int k=1;
    getsym();
    if(sym==lparen){
        //para
        getsym();
        if(sym==ident||sym==lparen||sym==chsym||sym==plus||sym==minus||sym==usinteger||sym==integer){
            //参数不光是以ident开头
            expression();
            //para
            if(tables[i+k].obj!=parameter){
                //error mismatch para
                error(MISMATCH_PARA);
                skipflag[rparen]+=1;
                skip();
                skipflag[rparen]-=1;
            }
            k++;
            enterquadruple(para,strcat(tempt1,itoa(tempvar,tempi1)),-1,"",0,"",0);
            tempt1[1]='\0';
            while(sym==comma){
                getsym();
                expression();
                if(tables[i+k].obj!=parameter){
                    //error mismatch para
                    error(MISMATCH_PARA);
                    skipflag[rparen]+=1;
                    skip();
                    skipflag[rparen]-=1;
                }
                enterquadruple(para,strcat(tempt1,itoa(tempvar,tempi1)),-1,"",0,"",0);
                tempt1[1]='\0';
                k++;
            }
        }
        if(sym==rparen){
            //funcall over
            if(tables[i+k].obj==parameter){
                //error mismatch pa
                error(MISMATCH_PARA);
                skipflag[rparen]+=1;
                skip();
                skipflag[rparen]-=1;
            }

            enterquadruple(j,tables[i].name,i,"",0,"",0);
            getsym();
        }
        else {
            error(MISSING_RPAREN);
        }
    }
    else {
        error(MISSING_LPAREN);
    }
    //printf("函数调用语句\n");
}

void readstatement()
{
    int i;
    if(sym!=scanfsym){
        //error 应该是scanf
        error(EXPECT_SCANF);
    }
    getsym();
    if(sym==lparen){
        getsym();
        if(sym==ident){
            i=loc(id);
            if(tables[i].obj!=variable&&tables[i].obj!=parameter){
                error(EXPECT_PARA_VAR);
            }
            enterquadruple(read,"",0,"",0,tables[i].name,i);

            getsym();
            while(sym==comma){
                getsym();
                if(sym==ident){
                    //读取
                    i=loc(id);
                    if(tables[i].obj!=variable&&tables[i].obj!=parameter){
                        error(EXPECT_PARA_VAR);
                    }
                    enterquadruple(read,"",0,"",0,tables[i].name,i);

                    getsym();
                }
                else {
                    //error 应该是标识符
                    error(EXPECT_IDENT);
                }
            }
        }
        else {
            //error 应该是标识符
            error(EXPECT_IDENT);
        }
        //getsym();
        if(sym==rparen){
            //right
            getsym();
        }
        else {
            //error 应该是）
            error(MISSING_RPAREN);
        }
    }
    else {
        error(MISSING_LPAREN);
    }
    //printf("读语句\n");
}

void writestatement()
{
    //printf("write start :%s\n",id);
    char tem[STRMAX]="__STR__";
    char t[NLMAX]="";
    if(sym!=printfsym){
        //error 应该是printf
        error(EXPECT_PRINTF);
    }
    getsym();
    //printf("write start :%s\n",id);
    if(sym==lparen){
        getsym();
        //printf("write :%s\n",id);
        if(sym==stringsym){
            //string
            strcpy(strtables[++si],str);//填表

            //printf("string ack.   id:%s\n",id);
            getsym();

            enterquadruple(write,"",0,"",0,strcat(tem,itoa(si,t)),-4);
            tem[7]='\0';
            if(sym==comma){
                getsym();
                expression();

                enterquadruple(write,"",0,"",0,strcat(tempt1,itoa(tempvar,tempi1)),-1);
                tempt1[1]='\0';
            }

            if(sym==rparen){
                //printf over
                getsym();
            }
        }
        else if(sym==rparen){
            error(EMPTY_PRINT);
        }
        else {

            expression();

            enterquadruple(write,"",0,"",0,strcat(tempt1,itoa(tempvar,tempi1)),-1);
            tempt1[1]='\0';
            if(sym==rparen){
                //printf over
                getsym();
            }
            else {
                error(MISSING_LPAREN);
            }
        }
    }
    else {
        error(MISSING_LPAREN);
    }
    //printf("写语句\n");
    //printf("write finish :%s\n",id);
}

void condstatement()
{
    //printf("condition start :%s\n",id);
    int casecount=0;
    int i=0,k,svalue;
    char finlab[ILMAX]="lb";
    char curlab[ILMAX]="lb";
    int casevalue[64]={0};
    strcat(finlab,itoa(++labno,tempi1));
    if(sym!=switchsym){
        //error 应该是switch
        error(EXPECT_SWITCH);
    }
    getsym();
    if(sym==lparen){
        getsym();
        //printf("condition :%s\n",id);
        expression();
        //
        svalue=tempvar;
        if(sym!=rparen){
            //error 应该是)
            error(MISSING_RPAREN);
            skipflag[lbrace]+=1;
            skip();
            skipflag[lbrace]-=1;
        }
        getsym();
        if(sym==lbrace){
            getsym();
            if(sym==casesym){
                do{
                    strcat(curlab,itoa(++labno,tempi1));
                    getsym();
                    int sign=1;
                    if(sym==minus||sym==plus){
                        if(sym==minus)sign=-1;
                        getsym();
                        if(sym==integer||sym==usinteger){
                            num*=sign;
                            //
                            getsym();
                        }
                        else {
                            //error 应该是个数字
                            error(EXPECT_INTEGER);

                        }
                    }
                    else if(sym==integer||sym==usinteger||sym==chsym){
                        //
                        getsym();
                    }
                    else {
                        //error case后面只能跟整数，无符号整数，char
                        error(EXPECT_INT_USINT_CHAR);
                    }

                    for(k=0;k<i;k++){
                        if(num==casevalue[k]){
                            error(DUP_CASE_VALUE);
                        }
                    }
                    casevalue[i++]=num;
                    enterquadruple(bne,strcat(tempt1,itoa(svalue,tempi1)),-1,itoa(num,tempi2),-3,curlab,0);//目标地址留空，之后补充
                    tempt1[1]='\0';
                    if(sym!=colon){
                        //error 应该是:
                        error(MISSING_COLON);
                        skip();
                    }
                    getsym();
                    statement();

                    enterquadruple(j,finlab,-2,"",0,"",0);//跳出switch
                    enterquadruple(label,curlab,-2,"",0,"",0);
                    curlab[2]='\0';
                    //printf("sym :%d\n",sym);
                }while(sym==casesym);
            }
            else {
                error(EMPTY_CASETABLE);
            }
            if(sym==defaultsym){
                //default
                getsym();
                if(sym==colon){
                    getsym();
                    if(sym==rbrace){
                        //switch over
                        getsym();
                    }
                    else {
                        statement();
                        if(sym==rbrace){
                            //switch over
                            getsym();
                        }
                    }
                }
            }
            else if(sym==rbrace){
                getsym();
            }
            else {
                error(MISSING_LBRACE);
            }
            enterquadruple(label,finlab,-2,"",0,"",0);
        }
        else {
            error(MISSING_LBRACE);
        }
    }
    else {
        error(MISSING_LPAREN);
        skipflag[semicolon]+=1;
        skip();
        skipflag[semicolon]-=1;
    }
    //printf("情况语句\n");
    //printf("condition finish :%s\n",id);
}

//void returnstate()
//{
//    if(sym!=returnsym){
//        //error 应该是return
//    }
//    getsym();
//    if(sym==semicolon){
//        //over
//    }
//    else if(sym==lparen){
//        expression();
//        if(sym!=rparen){
//            //error 应该是)
//        }
//    }
//    else {
//        //error 应该是(表达式)
//    }
//    printf("返回语句\n");
//}

void assignstatement()
{
    int i,tvar;
    if(sym!=ident){
        //error 多余的
        error(EXPECT_IDENT);
    }
    i=loc(id);
	if(tables[i].obj==constant){
        //error 不能给常量赋值
        error(INVALID_ASS_CONST);
        skipflag[semicolon]+=1;
        skip();
        skipflag[semicolon]-=1;
	}
    getsym();
    if(sym==becomes){
        getsym();
        expression();

        enterquadruple(assign,strcat(tempt1,itoa(tempvar,tempi1)),-1,"",0,tables[i].name,i);
        tempt1[1]='\0';
        //over
    }
    else if(sym==lbrack){
        //array
        getsym();
        //index
        expression();
        tvar=tempvar;
        if(sym!=rbrack){
            //error 应该是]
            error(MISSING_RBRACK);
        }
        getsym();
        if(sym!=becomes){
            //error 应该是=
            error(EXPECT_BECOME);
        }
        getsym();
        //value
        expression();

        enterquadruple(assign,strcat(tempt1,itoa(tempvar,tempi1)),-1,strcat(tempt2,itoa(tvar,tempi2)),-1,tables[i].name,i);
        tempt1[1]='\0';
        tempt2[1]='\0';
    }
    else {
        //error 应该是=或者[
        error(EXPECT_BECOME);
    }
    //printf("赋值语句\n");
}

void expression()
{
    int op=1;
    int temres=tempvar;
    operate opp;
    if(sym==plus||sym==minus){
        op=(sym==plus?1:-1);
        getsym();
    }
    term();
    //计算第一项的值，之后每次都加上新一项的值并保存到一个新的临时变量中
    if(op==-1){
        enterquadruple(mult,strcat(tempt1,itoa(tempvar,tempi1)),-1,"-1",-3,strcat(tempt2,itoa(tempvar+1,tempi3)),-1);
        tempvar+=1;
    }
    tempt1[1]='\0';
    tempt2[1]='\0';
    temres=tempvar;

    while(sym==plus||sym==minus){
        opp=sym==plus?add:sub;
        getsym();
        term();
        //op
        enterquadruple(opp,strcat(tempt1,itoa(temres,tempi1)),-1,strcat(tempt2,itoa(tempvar,tempi2)),-1,strcat(tempt3,itoa(tempvar+1,tempi3)),-1);
        tempvar+=1;
        tempt1[1]='\0';
        tempt2[1]='\0';
        tempt3[1]='\0';
        temres=tempvar;
    }
    //printf("刚刚是个表达式\n");
}

void term()
{
    int temres;
    operate opp;
    factor();
    temres=tempvar;
    while(sym==times||sym==slash){
        opp=sym==times?mult:div;
        getsym();
        factor();
        //op
        enterquadruple(opp,strcat(tempt1,itoa(temres,tempi1)),-1,strcat(tempt2,itoa(tempvar,tempi2)),-1,strcat(tempt3,itoa(tempvar+1,tempi3)),-1);
        tempvar+=1;
        tempt1[1]='\0';
        tempt2[1]='\0';
        tempt3[1]='\0';
        temres=tempvar;
    }
    //printf("刚刚是个项\n");
}

void factor()
{
    int i;
    if(sym==ident){
        i=loc(id);
        if(tables[i].obj==function&&tables[i].type!=voidtyp){
            funcall();

            enterquadruple(assign,tables[i].name,i,"",0,strcat(tempt1,itoa(tempvar+1,tempi1)),-1);
            tempvar+=1;
            tempt1[1]='\0';
            //用一个临时变量保存函数返回值
        }
        else if(tables[i].type==voidtyp&&tables[i].obj==function){
            //error 无返回值不能当作项
            error(INVALID_VOID_FUN);
        }
        else {
            getsym();
            if(sym==lbrack){
                //array
                getsym();
                //index
                expression();

                enterquadruple(assign,tables[i].name,i,strcat(tempt1,itoa(tempvar,tempi1)),-1,strcat(tempt2,itoa(tempvar+1,tempi2)),-1);
                tempvar+=1;
                tempt1[1]='\0';
                tempt2[1]='\0';
                if(sym!=rbrack){
                    //error 应该是]
                    error(MISSING_RBRACK);
                }
                getsym();
            }
            else{
                enterquadruple(assign,tables[i].name,i,"",0,strcat(tempt1,itoa(tempvar+1,tempi1)),-1);
                tempvar+=1;
                tempt1[1]='\0';
            }
            //identifier
        }
    }
    else if(sym==lparen){
        getsym();
        expression();

        //最新的temvar保存的就是结果
        if(sym!=rparen){
            //error 应该是)
            error(MISSING_LPAREN);
        }
        getsym();
    }
    else if(sym==integer||sym==usinteger){
        //整数
        enterquadruple(assign,itoa(num,tempi1),-3,"",0,strcat(tempt1,itoa(tempvar+1,tempi2)),-1);
        tempvar+=1;
        tempt1[1]='\0';
        getsym();
    }
    else if(sym==minus||sym==plus){
        int sign=1;
        if(sym==minus)sign=-1;
        getsym();
        if(sym==integer||sym==usinteger){
            num*=sign;

            enterquadruple(assign,itoa(num,tempi1),-3,"",0,strcat(tempt1,itoa(tempvar+1,tempi2)),-1);
            tempvar+=1;
            tempt1[1]='\0';
            getsym();
        }
        else {
            //error 应该是个数字
            error(EXPECT_INTEGER);
        }
    }
    else if(sym==chsym){
        //character
        //printf("***\t\tnum:%d   %c\n",num,num);
        sprintf(tempi1,"%d",num);
        enterquadruple(assign,tempi1,-5,"",0,strcat(tempt1,itoa(tempvar+1,tempi2)),-1);
        tempvar+=1;
        tempt1[1]='\0';
        getsym();
    }
    else {
        error(EXPECT_FACTOR);
    }
    //printf("刚刚是个因子\n");
}

void enterquadruple(operate op,char arg1[ILMAX],int ti1,char arg2[ILMAX],int ti2,char res[ILMAX],int ti3)
{
    int i;
    //char opp[17][10]={"add","sub","mult","div","dec","assign","label","j","beq","bne","ble","blt","bgt","bge","read","write","ret"};
    //printf("intermediate code:\n");
    if(ii>CLMAX){
        //error 溢出
    }
    if(op==assign&&ti3==-1){
        //只有为assign a,,b且ti3是临时变量时才会传递类型
        if(ti1>0){
            i=ti1;
            if(tables[i].type==chartyp){
                ischar[atoi(res+1)]=1;
            }
        }
        else if(ti1==-1){
            ischar[atoi(res+1)]=ischar[atoi(arg1+1)];
        }
        else if(ti1==-5){
            ischar[atoi(res+1)]=1;
        }
    }
    intercode[++ii].op=op;
    strcpy(intercode[ii].arg1,arg1);
    strcpy(intercode[ii].arg2,arg2);
    strcpy(intercode[ii].result,res);

    intercode[ii].ti1=ti1;
    intercode[ii].ti2=ti2;
    intercode[ii].ti3=ti3;

    //printf("***info:lc:%d\tcc:%d\tid:%s\tsym:%d\t%s,%s,%s,%s\n",lc,cc,id,sym,opp[intercode[ii].op],intercode[ii].arg1,intercode[ii].arg2,intercode[ii].result);
    //puts(line);
}


void info()
{
    //printf("line:%s\nlc:%d\ncc:%d\nsym:%d\n",line,lc,cc,sym);
}

char* itoa(int i,char* res)
{
    int j=0,k=0;
    char t;
    if(i<0){
        res[k++]='-';
        i=-i;
        j=1;
    }
    else if(i==0){
        res[0]='0';
        res[1]='\0';
        return res;
    }
    while(i>0){
        res[k++]=i%10+'0';
        i=i/10;
    }
    res[k]='\0';
    for(k=k-1;j<k;j++,k--){
        t=res[j];
        res[j]=res[k];
        res[k]=t;
    }
    return res;
}

int atoi(char *a)
{
    int res,i=0,j,sign=1;
    if(a[0]=='-'||a[0]=='+'){
        i=1;
        if(a[0]=='-'){
            sign=-1;
        }
    }
    for(j=strlen(a),res=0;i<j;i++){
        res=res*10+a[i]-'0';
    }
    return res*sign;
}

void printtables()
{
    int i;
    printf("tables:\nid\tname\tlink\tobj\ttyp\tref\tadr\n");
    for(i=0;i<=ti;i++){
        printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\n",i,tables[i].name,tables[i].link,tables[i].obj,tables[i].type,tables[i].ref,tables[i].adr);
    }

    printf("btables:\nid\tlast\tlastpar\tvsize\n");
    for(i=0;i<=bi;i++){
        printf("%d\t%d\t%d\t%d\n",i,blocktables[i].last,blocktables[i].lastpar,blocktables[i].vsize);
    }
    printf("constables:\nid\tvalue\n");
    for(i=0;i<=ci;i++){
        printf("%d\t%d\n",i,constable[i]);
    }
    printf("arraytables:\nid\ttype\tsize\n");
    for(i=0;i<=ai;i++){
        printf("%d\t%d\t%d\n",i,atables[i].typ,atables[i].size);
    }
}

void printintercode()
{
    int i;
    FILE *fout;
    fout=fopen("quadruple.txt","wt");
    char op[18][10]={"add","sub","mult","div","dec","assign","label","j","beq","bne","ble","blt","bgt","bge","read","write","ret","para"};
    printf("intermediate code:\n");
    for(i=1;i<=ii;i++){
        fprintf(fout,"%d:%s,\t%s,%d\t%s,%d\t%s,%d\n",i,op[intercode[i].op],intercode[i].arg1,intercode[i].ti1,
               intercode[i].arg2,intercode[i].ti2,
               intercode[i].result,intercode[i].ti3);
    }
    fclose(fout);
}

void error(int err)
{
    ec+=1;
    char errinfo[64][STRMAX]={
        "invalid function identifier.",
        "missing main function declaration.",
        "undefined identifier.",
        "missing ';'.",
        "incomplete file.",
        "invalid character.",
        "FATAL ERROR:TOO LONG STRING.",
        "",
        "",
        "FATAL ERROR:TOO MANY IDENT.",
        "invalid type of constant,expect int or char.",
        "expect an integer here.",
        "expect '='.",
        "expect character.",
        "expect int or char.",
        "expect identifier.",
        "missing ']'.",
        "expect unsigned integer.",
        "invalid part of source.",
        "missing ')'.",
        "missing '('.",
        "expect main function declaration.",
        "missing '}'.",
        "missing '{'.",
        "expect a statement.",
        "expect if statement.",
        "expect while statement.",
        "expect scanf statement.",
        "expect printf statement.",
        "expect switch statement.",
        "expect an integer or unsigned integer.",
        "expect integer, unsigned integer or character.",
        "missing ':'.",
        "expect '=' or '['.",
        "wrong function call:void function.",
        "left operand must be l-value.",
        "parameter mismatch.",
        "no string or expression to print.",
        "expect case statement.",
        "duplicate case value.",
        "redeclaration.",
        "expect a factor.",
        "expect variable or parameter.",
        "invalid return statement for void-type function.",
        "FATAL ERROR: TOO MANY CONST.",
        "FATAL ERROR: TOO MANY ARRAY."
    };
    printf("line %d:%s",lc,line);
    printf("        %*c\n",cc,'^');
    printf("error %d:%s\n",err,errinfo[err]);
}

void skip()
{
    //持续读取symbol直到读到合法符号
    while(skipflag[sym]==0){
        getsym();
    }
}

int fetchreg(int t,int i)
{
    int k;
    for(k=0;k<8;k++){
        if(lastuse[treg[k]]<i){
            treg[k]=t;
            return k;
        }
    }
    printf("**have no free reg.\n");
    return 0;
}

int findreg(int t)
{
    int k;
    for(k=0;k<8;k++){
        if(treg[k]==t)return k;
    }
    //printf("**not found t%d in treg.\n",t);
    return -1;
}

void exasm()
{
    FILE* fp;
    int i,index,x,z;
    index=0;

    char strlabel[256]="__STR__";
    char titoa[NLMAX];

    if((fp=(fopen("output.asm","wt")))==NULL){
        printf("create file output.asm failed.\n");
        exit(0);
    }
    strcpy(data[0],".data");
    for(i=1;i<=si;i++){
        strcpy(data[i],strcat(strcat(strcat(strcat(strlabel,itoa(i,titoa)),": .asciiz \""),strtables[i]),"\""));
        strlabel[7]='\0';
    }
    strcpy(text[index],".text");
    for(i=1;i<=ii;i++){
        if(intercode[i].ti1==-1){
            lastuse[atoi(intercode[i].arg1+1)]=i;
        }
        if(intercode[i].ti2==-1){
            lastuse[atoi(intercode[i].arg2+1)]=i;
        }
        if(intercode[i].ti3==-1){
            lastuse[atoi(intercode[i].result+1)]=i;
        }
    }
    //add,sub,mult,div,dec,assign,label,j,beq,bne,ble,blt,bgt,bge,read,write,ret

    //printf("glvi:%d\n",glvi);
    sprintf(text[++index],"move $k0,$sp");
    for(i=1;i<=ii;i++){
        switch(intercode[i].op){
            case add:{
                //add num1,num2,result
                sprintf(text[++index],"add $t%d,$t%d,$t%d",
                        findreg(atoi(intercode[i].result+1))==-1?fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)),
                        findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)));
                break;
            }
            case sub:{
                //sub num1,num2,result
                sprintf(text[++index],"sub $t%d,$t%d,$t%d",
                        findreg(atoi(intercode[i].result+1))==-1?fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)),
                        findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)));
                break;
            }
            case mult:{
                //mult num1,num2,result
                //mult num1,1,result
                //mult num1,-1,result
                if(intercode[i].ti1==-1&&intercode[i].ti2==-1){
                    sprintf(text[++index],"mul $t%d,$t%d,$t%d",
                            findreg(atoi(intercode[i].result+1))==-1?fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)),
                        findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)));
                }
                else if(intercode[i].ti2==-3&&atoi(intercode[i].arg2)==1){
                    sprintf(text[++index],"add $t%d,$t%d,$0",
                        findreg(atoi(intercode[i].result+1))==-1?fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)),
                        findreg(atoi(intercode[i].arg1+1)));
                }
                else if(intercode[i].ti2==-3&&atoi(intercode[i].arg2)==-1){
                    sprintf(text[++index],"sub $t%d,$0,$t%d",
                        findreg(atoi(intercode[i].result+1))==-1?fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)),
                        findreg(atoi(intercode[i].arg1+1)));
                }
                break;
            }
            case div:{
                //div num1,num2,result
                sprintf(text[++index],"div $t%d,$t%d",
                        findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)));
                sprintf(text[++index],"mflo $t%d",findreg(atoi(intercode[i].result+1))==-1?fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)));
                break;
            }
            case dec:{
                break;
            }
            case assign:{
                if(intercode[i].ti3>0){
                    //assign value,index,array|assign value,,ident
                    //x=loc(intercode[i].result);
                    x=intercode[i].ti3;
                    if(tables[x].obj==variable||tables[x].obj==parameter){
                        //暂时只有assign t,,var缺少常量赋值
                        sprintf(text[++index],x<=glvi?"sw $t%d,%d($gp)":"sw $t%d,-%d($k0)",findreg(atoi(intercode[i].arg1+1)),tables[x].adr);
                    }
                    else if(tables[x].obj==function){
                        //invalid
                        sprintf(text[++index],"move $v0,$t%d",findreg(atoi(intercode[i].arg1+1)));
                    }
                    else if(tables[x].obj==array){
                        //assign value,index,array
                        sprintf(text[++index],"mul $t%d,$t%d,-4",findreg(atoi(intercode[i].arg2+1)),findreg(atoi(intercode[i].arg2+1)));
                        sprintf(text[++index],"sub $t%d,$t%d,%d",findreg(atoi(intercode[i].arg2+1)),findreg(atoi(intercode[i].arg2+1)),tables[x].adr);
                        sprintf(text[++index],x<=glvi?"sub $t%d,$gp,$t%d":"add $t%d,$k0,$t%d",findreg(atoi(intercode[i].arg2+1)),findreg(atoi(intercode[i].arg2+1)));
                        sprintf(text[++index],"sw $t%d,($t%d)",findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)));
                    }
                    else if(tables[x].obj==constant){
                        //error 不能给常量赋值
                        printf("**error: cant assign to constant  %s\n",tables[x].name);
                    }
                }
                else if(intercode[i].ti3==-1){
                    //assign to tempvar
                    if(intercode[i].ti1>0){
                        //array to tempvar,function to tempvar,var to tempvar
                        //x=loc(intercode[i].arg1);
                        x=intercode[i].ti1;
                        if(tables[x].obj==variable||tables[x].obj==parameter){
                            sprintf(text[++index],x<=glvi?"lw $t%d,%d($gp)":"lw $t%d,-%d($k0)",
                                    findreg(atoi(intercode[i].result+1))==-1?
                                    fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)),tables[x].adr);
                        }
                        else if(tables[x].obj==function){
                            sprintf(text[++index],"move $t%d,$v0",findreg(atoi(intercode[i].result+1))==-1?
                                    fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)));
                        }
                        else if(tables[x].obj==constant){
                            sprintf(text[++index],"li $t%d,%d",findreg(atoi(intercode[i].result+1))==-1?
                                    fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)),constable[tables[x].adr]);
                        }
                        else if(tables[x].obj==array){
                            //assign array,index,target
                            sprintf(text[++index],"mul $t%d,$t%d,-4",findreg(atoi(intercode[i].arg2+1)),findreg(atoi(intercode[i].arg2+1)));
                            sprintf(text[++index],"sub $t%d,$t%d,%d",findreg(atoi(intercode[i].arg2+1)),findreg(atoi(intercode[i].arg2+1)),tables[x].adr);
                            sprintf(text[++index],x<=glvi?"sub $t%d,$gp,$t%d":"add $t%d,$k0,$t%d",findreg(atoi(intercode[i].arg2+1)),findreg(atoi(intercode[i].arg2+1)));
                            sprintf(text[++index],"lw $t%d,($t%d)",findreg(atoi(intercode[i].result+1))==-1?fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1))
                                    ,findreg(atoi(intercode[i].arg2+1)));
                        }
                    }
                    else if(intercode[i].ti1==-3||intercode[i].ti1==-5){
                        sprintf(text[++index],"li $t%d,%d",findreg(atoi(intercode[i].result+1))==-1?
                                    fetchreg(atoi(intercode[i].result+1),i):findreg(atoi(intercode[i].result+1)),atoi(intercode[i].arg1));
                    }
                }
                else {
                    //error
                }
                break;
            }
            case label:{
                if(intercode[i].ti1==-2){
                    sprintf(text[++index],"%s:",intercode[i].arg1);
                }
                else if(intercode[i].ti1>0){
                    z=loc(intercode[i].arg1);
                    sprintf(text[++index],"%s:",tables[z].name);

                    sprintf(text[++index],"add $sp,$sp,-4");
                    sprintf(text[++index],"sw $ra,($sp)");
                    //save return address
                    sprintf(text[++index],"add $sp,$sp,-4");
                    sprintf(text[++index],"sw $k0,($sp)");
                    //save last base
                    sprintf(text[++index],"la $k0,%d($sp)",8+tables[blocktables[tables[z].ref].lastpar].adr);
                    //new base
                    sprintf(text[++index],"add $sp,$sp,-%d",(tables[blocktables[tables[z].ref].last].adr-tables[blocktables[tables[z].ref].lastpar].adr)==0?0:
                            (tables[blocktables[tables[z].ref].last].adr-tables[blocktables[tables[z].ref].lastpar].adr)-8);

                    sprintf(text[++index],"sw $t0,-4($sp)");
                    sprintf(text[++index],"sw $t1,-8($sp)");
                    sprintf(text[++index],"sw $t2,-12($sp)");
                    sprintf(text[++index],"sw $t3,-16($sp)");
                    sprintf(text[++index],"sw $t4,-20($sp)");
                    sprintf(text[++index],"sw $t5,-24($sp)");
                    sprintf(text[++index],"sw $t6,-28($sp)");
                    sprintf(text[++index],"sw $t7,-32($sp)");
                    sprintf(text[++index],"add $sp,$sp,-32");
                }
                break;
            }
            case ret:{
                if(intercode[i].ti1==-1){
                    sprintf(text[++index],"move $v0,$t%d",findreg(atoi(intercode[i].arg1+1)));
                }
                //恢复现场

                sprintf(text[++index],"add $sp,$sp,32");
                sprintf(text[++index],"lw $t0,-4($sp)");
                sprintf(text[++index],"lw $t1,-8($sp)");
                sprintf(text[++index],"lw $t2,-12($sp)");
                sprintf(text[++index],"lw $t3,-16($sp)");
                sprintf(text[++index],"lw $t4,-20($sp)");
                sprintf(text[++index],"lw $t5,-24($sp)");
                sprintf(text[++index],"lw $t6,-28($sp)");
                sprintf(text[++index],"lw $t7,-32($sp)");

                sprintf(text[++index],"lw $ra,-%d($k0)",tables[blocktables[tables[z].ref].lastpar].adr+4);//restore $ra
                sprintf(text[++index],"add $sp,$sp,%d",blocktables[tables[z].ref].vsize);//restore $sp
                sprintf(text[++index],"lw $k0,-%d($k0)",tables[blocktables[tables[z].ref].lastpar].adr+8);

                sprintf(text[++index],"jr $ra");
                break;
            }
            case para:{
                sprintf(text[++index],"add $sp,$sp,-4");
                sprintf(text[++index],"sw $t%d,($sp)",findreg(atoi(intercode[i].arg1+1)));
                break;
            }
            case j:{
                if(intercode[i].ti1==-2)
                    sprintf(text[++index],"j %s",intercode[i].arg1);
                else{
//                    sprintf(text[++index],"add $s0,$t0,$zero");
//                    sprintf(text[++index],"add $s1,$t1,$zero");
//                    sprintf(text[++index],"add $s2,$t2,$zero");
//                    sprintf(text[++index],"add $s3,$t3,$zero");
//                    sprintf(text[++index],"add $s4,$t4,$zero");
//                    sprintf(text[++index],"add $s5,$t5,$zero");
//                    sprintf(text[++index],"add $s6,$t6,$zero");
//                    sprintf(text[++index],"add $s7,$t7,$zero");

                    sprintf(text[++index],"jal %s",intercode[i].arg1);

//                    sprintf(text[++index],"add $t0,$s0,$zero");
//                    sprintf(text[++index],"add $t1,$s1,$zero");
//                    sprintf(text[++index],"add $t2,$s2,$zero");
//                    sprintf(text[++index],"add $t3,$s3,$zero");
//                    sprintf(text[++index],"add $t4,$s4,$zero");
//                    sprintf(text[++index],"add $t5,$s5,$zero");
//                    sprintf(text[++index],"add $t6,$s6,$zero");
//                    sprintf(text[++index],"add $t7,$s7,$zero");
                }
                break;
            }
            case beq:{
                if(intercode[i].ti2==-3){
                    sprintf(text[++index],"beq $t%d,%s,%s",findreg(atoi(intercode[i].arg1+1)),intercode[i].arg2,intercode[i].result);
                }
                else
                    sprintf(text[++index],"beq $t%d,$t%d,%s",findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)),intercode[i].result);
                break;
            }
            case bne:{
                if(intercode[i].ti2==-3){
                    sprintf(text[++index],"bne $t%d,%s,%s",findreg(atoi(intercode[i].arg1+1)),intercode[i].arg2,intercode[i].result);
                }
                else
                    sprintf(text[++index],"bne $t%d,$t%d,%s",findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)),intercode[i].result);
                break;
            }
            case ble:{
                if(intercode[i].ti2==-3){
                    sprintf(text[++index],"ble $t%d,%s,%s",findreg(atoi(intercode[i].arg1+1)),intercode[i].arg2,intercode[i].result);
                }
                else
                    sprintf(text[++index],"ble $t%d,$t%d,%s",findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)),intercode[i].result);
                break;
            }
            case blt:{
                if(intercode[i].ti2==-3){
                    sprintf(text[++index],"blt $t%d,%s,%s",findreg(atoi(intercode[i].arg1+1)),intercode[i].arg2,intercode[i].result);
                }
                else sprintf(text[++index],"blt $t%d,$t%d,%s",findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)),intercode[i].result);
                break;
            }
            case bge:{
                if(intercode[i].ti2==-3){
                    sprintf(text[++index],"bge $t%d,%s,%s",findreg(atoi(intercode[i].arg1+1)),intercode[i].arg2,intercode[i].result);
                }
                else sprintf(text[++index],"bge $t%d,$t%d,%s",findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)),intercode[i].result);
                break;
            }
            case bgt:{
                if(intercode[i].ti2==-3){
                    sprintf(text[++index],"bgt $t%d,%s,%s",findreg(atoi(intercode[i].arg1+1)),intercode[i].arg2,intercode[i].result);
                }
                else sprintf(text[++index],"bgt $t%d,$t%d,%s",findreg(atoi(intercode[i].arg1+1)),findreg(atoi(intercode[i].arg2+1)),intercode[i].result);
                break;
            }
            case read:{
                //x=loc(intercode[i].result);
                x=intercode[i].ti3;
                if(tables[x].type==chartyp){
                    sprintf(text[++index],"li $v0,12");
                }
                else if(tables[x].type==inttyp){
                    sprintf(text[++index],"li $v0,5");
                }
                else {
                    //error 不支持的类型
                }
                sprintf(text[++index],"syscall");
                if(x<=glvi){
                    sprintf(text[++index],"sw $v0,-%d($gp)",tables[x].adr);
                }
                else
                    sprintf(text[++index],"sw $v0,-%d($k0)",tables[x].adr);
                break;
            }
            case write:{
                if(intercode[i].ti3==-4){
                    //write string
                    sprintf(text[++index],"la $a0,%s",intercode[i].result);
                    sprintf(text[++index],"li $v0,4");
                    sprintf(text[++index],"syscall");
                }
                else if(intercode[i].ti3==-1){
                    //write var
                    if(ischar[atoi(intercode[i].result+1)]){
                        sprintf(text[++index],"move $a0,$t%d",findreg(atoi(intercode[i].result+1)));
                        sprintf(text[++index],"li $v0,11");
                        sprintf(text[++index],"syscall");
                    }
                    else {
                        sprintf(text[++index],"move $a0,$t%d",findreg(atoi(intercode[i].result+1)));
                        sprintf(text[++index],"li $v0,1");
                        sprintf(text[++index],"syscall");
                    }
                }
                else if(intercode[i].ti3==-5){
                    //
                    sprintf(text[++index],"move $a0,$t%d",findreg(atoi(intercode[i].result+1)));
                    sprintf(text[++index],"li $v0,11");
                    sprintf(text[++index],"syscall");
                }
                break;
            }
            default:{
                //undefined
            }
        }
    }

    for(i=0;i<=si;i++){
        fputs(data[i],fp);
        fputc('\n',fp);
    }
    for(i=0;i<index;i++){
        fputs(text[i],fp);
        fputc('\n',fp);
    }
/*
    for(i=0;i<=tempvar;i++){
        printf("%d:\t%d\n",i,ischar[i]);
    }
*/
    printintercode();
    fclose(fp);
}
