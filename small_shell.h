#ifndef SMALL_SHELL_H_
#define SMALL_SHELL_H_

#include <memory>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "Commands.h"
#include "exceptions.h"
#include "jobs.h"
using namespace std;

#define NO_FG -1

string _ltrim(const std::string& s);
string _rtrim(const std::string& s);
string _trim(const std::string& s);
class Command;
class JobsList;



class SmallShell {
 private:
  // TODO: Add your data members
  string current_prompt;
  string old_pwd;
  string current_pwd;
  pid_t pid;
  SmallShell();
  shared_ptr<JobsList> jobs_list;
  shared_ptr<TimedJobsList> timed_jobs;
  pid_t fg_pid;
  int fg_job_id;
  string fg_cmd;
  bool is_running;
 public:
  shared_ptr<Command> createCommand(const string cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const string cmd_line);
  // TODO: add extra methods as needed
  void setPrompt(const string new_prompt = "smash");
  string getPrompt();
  pid_t getPid();
  void changeCurrentDirectory(const string new_pwd);
  bool isOldPwdSet();
  string getOldPwd();
  string getCurrPwd();
  void addJob (string cmd_line, pid_t pid, bool is_stopped, int job_id = NO_JOB); 
  void updateJobsList();
  void sendSignal(const string signal);
  void bg ();
  void fg ();
  shared_ptr<JobsList> getJobs();
  void setFgJob(pid_t pid = NO_FG, string cmd_line = "", int job_id = NO_FG);
  pid_t getFgPid();
  int getFgJobId();
  void addFgJobToJobsList();
  void stopRun();
  bool isRunning() const;
  void addTimedJob(int duration, shared_ptr<JobEntry> job);
  void gotAlarm();
};

#endif //SMALL_SHELL_H_
