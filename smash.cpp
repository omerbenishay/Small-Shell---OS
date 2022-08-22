#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include "small_shell.h"
#define ERROR -1


int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    //TODO: setup sig alarm handler
    struct sigaction alarm;
    alarm.sa_handler = alarmHandler;
    alarm.sa_flags = SA_RESTART;

    sigemptyset(&alarm.sa_mask);
    sigaddset(&alarm.sa_mask, SIGALRM);
    if (sigaction(SIGALRM, &alarm, nullptr) == ERROR)
    {
        perror("smash error: failed to set ALARM handler");
    }

    SmallShell& smash = SmallShell::getInstance();
    while(smash.isRunning()) {
        std::cout << smash.getPrompt() + "> ";
        std::string cmd_line;
        if(!std::getline(std::cin, cmd_line))
        {
            break;
        }
        if(cmd_line == "")
        {
            continue;
        }//check if needed
        smash.executeCommand(cmd_line);
    }
    return 0;
}
