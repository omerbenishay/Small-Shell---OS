
#include "small_shell.h"

#define PWD_NOT_SET "-1"

using namespace std;


#define ERROR -1
#define MAX_PATH_SIZE 80

string _getCwd() {
    char current_path[MAX_PATH_SIZE];
    if (getcwd(current_path, MAX_PATH_SIZE) == nullptr)
    {
      throw SyscallException("getcwd");
    }
    return string(current_path);
}
bool _isPipeCmd(const string cmd_line) {
  const string str(cmd_line);
  size_t found = str.find("|");
  return (found != std::string::npos);
}
bool _isIOCmd(const string cmd_line) {
  const string str(cmd_line);
  size_t found = str.find(">");
  return (found != std::string::npos);
}


SmallShell::SmallShell() {
  this->pid = getpid();
  this->current_prompt = "smash";
  this->current_pwd = PWD_NOT_SET; 
  this->old_pwd = PWD_NOT_SET;
  this->fg_job_id = NO_FG;
  this->fg_pid = NO_FG;
  this->fg_cmd = "";
  this->jobs_list = shared_ptr<JobsList>(new JobsList());
  this->is_running = true;
  this->timed_jobs = shared_ptr<TimedJobsList>(new TimedJobsList());

}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

string SmallShell::getPrompt()
{
  return this->current_prompt;
}

void SmallShell::setPrompt(string new_prompt)
{
  this->current_prompt = new_prompt;
}
pid_t SmallShell::getPid()
{
  return this->pid;
}
void SmallShell::changeCurrentDirectory(string new_pwd)
{
  if (this->current_pwd == PWD_NOT_SET)
  {
    this->current_pwd = _getCwd();
  }
  if(chdir(new_pwd.c_str()) == -1)
  {
    throw SyscallException("chdir");
  }
  this->old_pwd = this->current_pwd;
  this->current_pwd = _getCwd();
}
bool SmallShell::isOldPwdSet()
{
  return this->old_pwd != PWD_NOT_SET;
}
string SmallShell::getOldPwd()
{
  return this->old_pwd;
}
string SmallShell::getCurrPwd()
{
  if (this->current_pwd == PWD_NOT_SET)
  {
    this->current_pwd = _getCwd();
  }
  return this->current_pwd;
}
void SmallShell::stopRun()
{
  this->is_running = false;
}
bool SmallShell::isRunning() const
{
  return this->is_running;
}

void SmallShell::gotAlarm()
{
  this->timed_jobs->gotAlarm();
}
void SmallShell::addTimedJob(int duration, shared_ptr<JobEntry> job)
{
  this->timed_jobs->addTimedJob(duration, job);
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
shared_ptr<Command> SmallShell::createCommand(const string cmd_line) {
	// For example:
/*
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  string cmd_s = _trim(cmd_line);
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  
  if(_isIOCmd(cmd_s))
  {
    return shared_ptr<Command>(new RedirectionCommand(cmd_line));
  }
  if(_isPipeCmd(cmd_s))
  {
    return shared_ptr<Command>(new PipeCommand(cmd_line));
  }
  else if(firstWord.compare("chprompt") == 0)
    return shared_ptr<Command>(new ChangePromptCommand(cmd_line));
  else if(firstWord.compare("showpid") == 0)
    return shared_ptr<Command>(new ShowPidCommand(cmd_line));
  else if(firstWord.compare("pwd") == 0)
    return shared_ptr<Command>(new GetCurrDirCommand(cmd_line));
  else if(firstWord.compare("cd") == 0)
    return shared_ptr<Command>(new ChangeDirCommand(cmd_line));
  else if(firstWord.compare("jobs") == 0)
    return shared_ptr<Command>(new JobsCommand(cmd_line, SmallShell::getInstance().getJobs()));
  else if(firstWord.compare("kill") == 0)
    return shared_ptr<Command>(new KillCommand(cmd_line, SmallShell::getInstance().getJobs()));
  else if(firstWord.compare("fg") == 0)
    return shared_ptr<Command>(new ForegroundCommand(cmd_line, SmallShell::getInstance().getJobs()));
  else if(firstWord.compare("bg") == 0)
    return shared_ptr<Command>(new BackgroundCommand(cmd_line, SmallShell::getInstance().getJobs()));
  else if(firstWord.compare("quit") == 0)
    return shared_ptr<Command>(new QuitCommand(cmd_line, SmallShell::getInstance().getJobs()));
  else if(firstWord.compare("tail") == 0)
    return shared_ptr<Command>(new TailCommand(cmd_line));
  else if(firstWord.compare("touch") == 0)
    return shared_ptr<Command>(new TouchCommand(cmd_line));
  else if(firstWord.compare("timeout") == 0)
    return shared_ptr<Command>(new TimeoutCommand(cmd_line));
  else 
    return shared_ptr<Command>(new ExternalCommand(cmd_line));
}

void SmallShell::executeCommand(const string cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)


  //   jobs.updateJobList();
  try {
      shared_ptr<Command> cmd = SmallShell::createCommand(cmd_line);
      if (cmd != nullptr) {
        cmd->execute();

      }
  } catch (SyscallException &err) {
      perror(err.what());
  } catch (SmashException &err) {
      cerr << err.what();
  }
}

void SmallShell::addJob (string cmd_line, pid_t pid, bool is_stopped, int job_id)
{
  this->jobs_list->addJob(cmd_line, pid, is_stopped, job_id);
}

void SmallShell::setFgJob(pid_t pid, string cmd_line, int job_id)
{
  this->fg_pid = pid;
  this->fg_job_id = job_id;
  this->fg_cmd = cmd_line;
}

shared_ptr<JobsList> SmallShell::getJobs()
{
  return this->jobs_list;
}
void SmallShell::addFgJobToJobsList()
{
  pid_t pid = this->fg_pid;
  if(pid == NO_FG)
  {
    return;
  }
  this->addJob(this->fg_cmd, this->fg_pid, true, this->fg_job_id);
  this->setFgJob();
  if (kill(pid, SIGSTOP) == ERROR)
  {
    throw SyscallException("kill");
  }
  
  cout << "smash: process " << pid << " was stopped" << endl;
}
pid_t SmallShell::getFgPid()
{
  return this->fg_pid;
}
int SmallShell::getFgJobId()
{
  return this->fg_job_id;
}
