#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include<vector>
#include<algorithm>
#include <unistd.h>
#include <stdio.h>
#include<sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define DB_H
#ifdef DB_H
#define DBUG(x) x
#else
#define DBUG(x)
#endif

#define MAX_CMD_LENGTH 100
#define MAX_ARG_NUM 50

std::vector<std::string> PipeComponents;
std::vector<std::string>::iterator PipeCItr;

typedef std::vector<std::string> SVec;
typedef std::vector<SVec> SSVec; 
SSVec InstructionS_PipeLine;
SSVec::iterator InsPip_Itr;

int redirCheck(std::string str)
{
    int tmp=0;
    int inRDir = std::count(str.begin(),str.end(),'<');
    int outRDir= std::count(str.begin(),str.end(),'>');
    if(inRDir>0) tmp+=1;
    if(outRDir>0) tmp+=2;
    return tmp;
}

void run_PipeElement(SSVec &InsPipes,int PipeElementNum,int PipeElementIndex)
{
    SSVec::iterator InsPip_Itr1=(InsPipes.begin()+PipeElementIndex);   
    SVec::iterator itr;
    for(itr=(*InsPip_Itr1).begin();itr!=(*InsPip_Itr1).end();itr++)
    {
        //(*itr) = app + args
        int ReDirectIndex = redirCheck((*itr));
        DBUG(std::cout<<"ReDirectIndex "<<ReDirectIndex<<std::endl;)

        int TokenNum=std::count((*itr).begin(),(*itr).end(),' ');
        DBUG(std::cout<<"TokenNum "<<TokenNum<<std::endl;)
        char *Commands[TokenNum+1];
        int c_index=0;
        char t1[(*itr).size()+1];
        strcpy(t1,(*itr).c_str());
        char *t2=std::strtok(t1," ");
        while(t2!=NULL)
        {
            Commands[c_index]= t2;
            c_index++;
            t2= std::strtok(NULL, " ");
        }
        DBUG(
            for(int i=0;i<TokenNum;i++)
            {
                std::cout<<"Command["<<i<<"]== "<<Commands[i]<<std::endl;
            }
            
        )
        //Now InputLine:"PipeE1 | PipeE2 | ... " -> PipeE: "Ins1 ; Ins2 ; ..." -> Ins: " Cmd[0] Cmd[1] ...." 
        std::string inFileName, outFileName;
        std::cout<<"AAA"<<TokenNum<<std::endl;
        for(int i=0;i<(TokenNum-1);i++)
        {
            std::string tmpStr=Commands[i];
            if(tmpStr==">")
            {
                outFileName = Commands[i+1];
            }
            if(tmpStr=="<")
            {
                inFileName = Commands[i+1];
            }
        }
        DBUG(
            std::cout<<"inFileName(string) "<<inFileName<<std::endl;
            std::cout<<"outFileName(string) "<<outFileName<<std::endl;
        )

        //ReDirection
        bool inRdir=false,outRdir=false;
        switch (ReDirectIndex)
        {
        case 1:
            inRdir=true;
            break;
        case 2:
            outRdir=true;
            break;
        case 3:
            inRdir=true;
            outRdir=true;
            break;
        default:
            break;
        }
        int fd,fd2;
        //In
        if(inRdir==true)
        {
            if((fd = open(inFileName.c_str(), O_RDONLY)) < 0){
                perror("open error");
                return;
            }

            dup2(fd, 0);
            close(fd);
        }
        //Out
        if(outRdir==true)
        {
            if((fd2 = open(outFileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC)) < 0){
                perror("open error");
                return;
            }
            dup2(fd2, 1);
            close(fd2);
        }
        char *cmd_args[512] = {NULL};
        for(int i=0;i<TokenNum;i++)
        {
            std::string tmpStr=Commands[i];
            cmd_args[i]=new char[512];
            std::strcpy(cmd_args[i],tmpStr.c_str());
        }
        if(execv(cmd_args[0], cmd_args) == -1){
            perror("execution error");
            _exit(EXIT_FAILURE);
        }
        

    }
    return;
}

void run_pipes(SSVec &InsPipes,int PipeElementNum,int PipeElementIndex)
{
    if(PipeElementIndex == PipeElementNum-1)
    {
        run_PipeElement(InsPipes,PipeElementNum,PipeElementIndex);
    }
    if(PipeElementIndex<PipeElementNum)
    {
        pid_t pid;
        int fd[2], status;

        if(pipe(fd)<0)
        {
            perror("pipe");
            _exit(EXIT_FAILURE);
        }

        if((pid=fork())<0)
        {
              perror("fork");
            _exit(EXIT_FAILURE);
        }

        if(pid!=0)
        {
            dup2(fd[1], 1);
            close(fd[0]);
            run_PipeElement(InsPipes,PipeElementNum,PipeElementIndex);
            waitpid(-1,&status,0);
        }
        else
        {
            if(PipeElementIndex!=PipeElementNum-1)
            {
                dup2(fd[0],0);
            }
            close(fd[1]);

            PipeElementIndex++;
            run_pipes(InsPipes,PipeElementNum,PipeElementIndex);
        }
    }
    return;
}

void parse_and_run_command(const std::string &command) {
    /* TODO: Implement this. */
    /* Note that this is not the correct way to test for the exit command.
       For example the command "   exit  " should also exit your shell.
     */
    if (command == "exit") {
        exit(0);
    }

    DBUG(std::cout<<"INFO: command is "<<command<<std::endl;
        std::cout<<std::endl<<std::endl;)
    
    //z Length Check
    if(command.size()>100)
    {
        std::cout<<"-DEBUG: ERROR - CommandLength is more than 100"<<std::endl;
        exit(0);
    }
    //z parse
    //Preprocessing command
    //Get Elements in PipeLine 
    PipeComponents.clear();
    char c_cmd[command.size()+1];
    strcpy(c_cmd,command.c_str());
    char* tmp = std::strtok(c_cmd,"|");
    while (tmp!=NULL)
    {
        std::string PipeElementTmp=tmp;
        std::string PipeElement;
        for(int i=0;i<PipeElementTmp.size();i++)
        {
            std::string t;
            if(PipeElementTmp[i]=='\\'&&(i+1)<PipeElementTmp.size())
            {
                if(PipeElementTmp[i+1]=='f' || \
                    PipeElementTmp[i+1]=='n' || \
                    PipeElementTmp[i+1]=='r' || \
                    PipeElementTmp[i+1]=='t' || \
                    PipeElementTmp[i+1]=='v')
                {
                    i=i+1;
                    t=" ";
                }
                else
                {
                    t=PipeElementTmp[i];
                }
            }
            else if(PipeElementTmp[i]=='>')
            {
                bool notReDirect=false;
                if(PipeElementTmp[i+1]=='>')
                {
                    notReDirect=true;
                }
                else
                {
                    if(i>0)
                    {
                        if(PipeElementTmp[i-1]=='>') notReDirect=true;
                    }
                }
                if(notReDirect==false)
                {
                    t=" > ";
                }
                else
                {
                    t=PipeElementTmp[i];
                }
            }
            else if(PipeElementTmp[i]=='<')
            {
                bool notReDirect=false;
                if(PipeElementTmp[i+1]=='<')
                {
                    notReDirect=true;
                }
                else
                {
                    if(i>0)
                    {
                        if(PipeElementTmp[i-1]=='<') notReDirect=true;
                    }
                }
                if(notReDirect==false)
                {
                    t=" < ";
                }
                else
                {
                    t=PipeElementTmp[i];
                }
            }
            else
            {
                t=PipeElementTmp[i];
            }
            PipeElement.append(t);
        }
        
        PipeComponents.push_back(PipeElement);
        tmp = std::strtok(NULL, "|");
    }
    //Now we get each part in the input which divides by '|'
    //Next to get each Instructions in the part we get, divided by ';'
    InstructionS_PipeLine.clear();
    for(PipeCItr=PipeComponents.begin();PipeCItr!=PipeComponents.end();PipeCItr++)
    {
        SVec InstructionS; InstructionS.clear();
        int comp_size = PipeCItr->size();
        char T1[comp_size+1];
        strcpy(T1,PipeCItr->c_str());
        char *t2=std::strtok(T1,";");
        while (t2!=NULL)
        {
            std::string t3 = t2;
            for(int i=0;i<t3.size();i++)
            {
                if(t3[i]!=' ')
                {
                    InstructionS.push_back(t3);
                    break;
                }
            }
            t2 = std::strtok(NULL, ";");
        }
        InstructionS_PipeLine.push_back(InstructionS);
    }
    //Now command={InstructionS_PipeLine1,InstructionS_PipeLine2,...} , InstructionS_PipeLine={InstructionS1,InstructionS2,....}
    //But InstructionS may have sequential ' ' among program and args. 
    //Next this will be handled.
    for(InsPip_Itr=InstructionS_PipeLine.begin();InsPip_Itr!=InstructionS_PipeLine.end();InsPip_Itr++)
    {
        SVec::iterator itr;
        for(itr=(*InsPip_Itr).begin();itr!=(*InsPip_Itr).end();itr++)
        {
            std::string tmpTokenS;
            char t1[(itr->size())+1];
            strcpy(t1,(*itr).c_str());
            char *t2=std::strtok(t1," ");
            while(t2!=NULL)
            {
                std::string t3=t2;
                tmpTokenS.append(t3);
                tmpTokenS.append(" ");
                t2=std::strtok(NULL," ");
            }
            (*itr)=tmpTokenS;
        }
    }
    DBUG(
        std::cout<<"-D: There are "<<std::dec<<InstructionS_PipeLine.size()<<" Components in PipeLine"<<std::endl;
        for(InsPip_Itr=InstructionS_PipeLine.begin();InsPip_Itr!=InstructionS_PipeLine.end();InsPip_Itr++)
        {
            std::cout<<"--D: There are "<<std::dec<<(*InsPip_Itr).size()<<" Instructions in this Component"<<std::endl;
            std::cout<<"   Instructions: "<<std::endl;
             std::vector<std::string>::iterator itr;
            for(itr=(*InsPip_Itr).begin();itr!=(*InsPip_Itr).end();itr++)
            {
                std::cout<<"   -"<<(*itr)<<"-"<<std::endl;
            }
        }
    )
    //run_PipeElement
    int PENum=InstructionS_PipeLine.size();
    run_PipeElement(InstructionS_PipeLine,PENum,0);

    std::cerr << "Not implemented.\n";
    PipeComponents.clear();
    InstructionS_PipeLine.clear();
}

int main(void) {
    std::string command;
    std::cout << "> ";
    while (std::getline(std::cin, command)) {
        parse_and_run_command(command);
        std::cout << "> ";
    }
    return 0;
}