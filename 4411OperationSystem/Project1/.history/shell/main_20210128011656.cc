#include <cstdlib>
#include <iostream>
#include <string>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

void parse_and_run_command(const std::string &command) {
    /* TODO: Implement this. */
    /* Note that this is not the correct way to test for the exit command.
       For example the command "   exit  " should also exit your shell. */
    if (command == "exit") {
        exit(0);
    }
    //parse
    vector<vector<string> > cmds;
    vector<pair<int, string> > cmds_redirection;
    string args[512];
    int counter=0;
    bool command_first_letter=false;
    int argc=0;
    for (uint64_t i=0; i<command.length(); i++){
        if(command[i] ==  ' ' || command[i] == '\t' || command[i] == '\n' || command[i] == '\f' || command[i] == '\r' || command[i] == '\v' ){
            if(command_first_letter == false){
                continue;
            }else{
                counter++;
                command_first_letter = false;
            }
        }else{
            args[counter] += command[i];
            command_first_letter = true;
        }
    }
    
    if(command_first_letter == false){
        argc = counter;
    }else{
        argc = counter+1;
    }
    /*
    cout << "*******" <<endl;
    cout << "argc = " << argc << endl;
    for(int i=0; i < argc; i++){
        cout << "arg[" << i << "]=" << args[i]<< endl;
    }
    cout << "*******" <<endl;
    */
    vector<string> temp_args;
    bool does_cmd_redirect = false;
    //judging < << > >> |
    for(int i=0; i < argc; i++){
        if(args[i] == "|"){
            cmds.push_back(temp_args);
            if(does_cmd_redirect == false){
                cmds_redirection.push_back(make_pair(0, ""));
            }else{
                does_cmd_redirect = false;
            }
            temp_args.clear();
        }else if(args[i] == "<"){
            cmds_redirection.push_back(make_pair(-1, args[++i]));
            
            does_cmd_redirect = true;
        }
        // else if(args[i] == "<<"){
        //     cmds_redirection.push_back(make_pair(-2, args[++i]));
        //     does_cmd_redirect = true;
        // }
        else if(args[i] == ">"){
            cmds_redirection.push_back(make_pair(1, args[++i]));
            does_cmd_redirect = true;
        }
        // else if(args[i] == ">>"){
        //     cmds_redirection.push_back(make_pair(2, args[++i]));
        //     does_cmd_redirect = true;
        // }
        else{
            temp_args.push_back(args[i]);
        }   
        if((i==argc || args[i]=="<" || args[i]==">" || args[i]=="|") && does_cmd_redirect == true){
            cerr << "invalid command." << endl;
            return;
        }
    }
    cmds.push_back(temp_args);
    if(does_cmd_redirect == false)
        cmds_redirection.push_back(make_pair<int, string>(0, ""));
    temp_args.clear();
    vector<string>::iterator arg_iter;
    vector<vector<string> >::iterator cmd_iter;
    // cout << "$$$$$$$$ " << cmds.size() << " " << cmds_redirection.size() << endl;
    // for(cmd_iter = cmds.begin(); cmd_iter != cmds.end(); cmd_iter++){
    //     cout << "cmd[" << distance(cmds.begin(), cmd_iter);
    //     cout << "] red: " << cmds_redirection[distance(cmds.begin(), cmd_iter)].first ;
    //     cout << " " << cmds_redirection[distance(cmds.begin(), cmd_iter)].second;
    //     cout << " argc=" << cmd_iter->size() << endl;
    //     for(arg_iter = cmd_iter->begin(); arg_iter != cmd_iter->end(); arg_iter++)
    //         cout << "arg[" << distance(cmd_iter->begin(), arg_iter) << "]= " << *arg_iter << endl;
    //     cout << "$$$$$$$$ " << endl;
    // }
    //finish parsing
    vector<int> pid_list;
    vector<int>::iterator pid_iter;
    int init_input = dup(0), fd[2];
    for(cmd_iter = cmds.begin(); cmd_iter != cmds.end(); cmd_iter++){
        //for each command devided by pipe
        if(cmd_iter->size() == 0){
            cerr << "invalid command." << endl;
            return;
        }
        bool pipe_convert_next=false;

        uint64_t command_index = distance(cmds.begin(), cmd_iter); 
        
        int pipe_fd ;
        if(command_index < cmds.size() - 1){
            pipe_convert_next = true;
            pipe_fd = pipe(fd);
        }
        if(pipe_fd == -1 && pipe_convert_next == true){
            perror(nullptr);
            return;
        }

        int pid = fork();
        if (pid == 0) {//child
            if (pipe_convert_next == true) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                close(fd[0]);
            }

            if(cmds_redirection[command_index].first > 0){//output redirection
                int fd_out = open(cmds_redirection[command_index].second.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
                if(fd_out == -1){
                    perror(nullptr);
                    return;
                }
                close(STDOUT_FILENO);
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }else if(cmds_redirection[command_index].first < 0){//input redirection
                int fd_in = open(cmds_redirection[command_index].second.c_str(), O_RDONLY);
                if(fd_in == -1){
                    perror(nullptr);
                    return;
                }
                close(STDIN_FILENO);
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            char* cmd_args[512] = {nullptr};
            int k = 0;
            for(arg_iter = cmd_iter->begin(); arg_iter != cmd_iter->end(); arg_iter++, k++){
                string arg_string = *arg_iter;
                cmd_args[k] = new char[512];
                strcpy(cmd_args[k], arg_string.c_str());
            }

            if(execv(cmd_args[0], cmd_args) == -1){
                perror(nullptr);
                return;
            }
        } 
        else if (pid > 0){//parent
            if(pipe_convert_next == true){
                close(STDIN_FILENO);
                dup2(fd[0],STDIN_FILENO);
                close(fd[1]);
                close(fd[0]);
            }
            pid_list.push_back(pid);
        } else {
            perror(nullptr);
            return;
        }
    }
    cmd_iter = cmds.begin();
    for(pid_iter = pid_list.begin(); pid_iter != pid_list.end(); pid_iter++){
        int stat;
        if (waitpid(*pid_iter, &stat, 0)== -1){
            perror(nullptr);
            return;
        }
        int exit_status = 0;
        bool err = false;
        if (WIFEXITED(stat)) 
            exit_status = WEXITSTATUS(stat);
        else if (WIFSIGNALED(stat)) 
            err = true;
        for(arg_iter = cmd_iter->begin(); arg_iter != cmd_iter->end(); arg_iter++)
            cout << *arg_iter << " ";
        if (err == false)
            cout << "exit status: "<<exit_status<< endl;
        cmd_iter++;
    }
    dup2(init_input, STDIN_FILENO);
    close(init_input);
//test ls as an Inner command
    // if (args[0] == "my_ls"){
    //     if(argc == 1){
    //         DIR *dir;
    //         dir = opendir(".");
    //         struct dirent *rent;
    //         //int cnt = 0;
    //         while((rent=readdir(dir))){
    //             if(rent->d_name[0] != '.') //avoid . & ..
    //             cout << rent->d_name << endl;
    //         }
    //         closedir(dir);
    //     }else{
    //         for (int i=1; i<argc; i++){
    //             DIR *dir;
    //             dir = opendir(args[i].c_str());
    //             struct dirent *rent;
    //             //int cnt = 0;
    //             while((rent=readdir(dir))){
    //                 if(rent->d_name[0] != '.') //avoid . & ..
    //                 cout << rent->d_name << endl;
    //             }
    //             closedir(dir);
    //         }
    //     }
    //     return;
    // }
    //std::cerr << "Not implemented.\n";
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
