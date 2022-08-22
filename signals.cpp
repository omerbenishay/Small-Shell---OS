#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#define ERROR -1
using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
  cout << "smash: got ctrl-Z" << endl;
  SmallShell& smash = SmallShell::getInstance();
  smash.addFgJobToJobsList();
  //smash.getJobs()->removeFinishedJobs(); // added
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
  cout << "smash: got ctrl-C" << endl;
  SmallShell& smash = SmallShell::getInstance();
  pid_t pid = smash.getFgPid();
  if(pid == NO_FG)
  {
    return;
  }
  if (kill(pid, SIGKILL) == ERROR)
  {
    throw SyscallException("kill");
  }
  smash.getJobs()->removeFinishedJobs(); // added
  cout << "smash: process " << pid << " was killed" << endl;
  smash.setFgJob();
}

void alarmHandler(int sec) {
  // TODO: Add your implementation
  cout << "smash: got an alarm" << endl;
  SmallShell& smash = SmallShell::getInstance();
  smash.gotAlarm();
}

