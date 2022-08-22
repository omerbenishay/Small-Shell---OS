#include <unistd.h>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
#include <utime.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>



using namespace std;
#define ERROR -1
#define NO_FG -1
#define NO_ID -1
#define N 10
#define SUCCESS 1
#define LAST_LINE 2

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

const string WHITESPACE = " \n\r\t\f\v";



string _ltrim(const string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == string::npos) ? "" : s.substr(start);
}

string _rtrim(const string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const string &cmd_line, vector<string> &argv)
{
  FUNC_ENTRY()
  int i = 0;
  istringstream iss(_trim(cmd_line));
  for (string s; iss >> s; i++)
  {
    argv.push_back(s);
  }
  return i;

  FUNC_EXIT()
}


bool _isBackgroundComamnd(const string cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}
 
void _removeBackgroundSign(string&  cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

bool is_num (const string& str)
{
  return !str.empty() && str.find_first_not_of("0123456789") == string::npos;
}

// TODO: Add your implementation for classes in Commands.h 
void runInFg(pid_t pid, const string& line, int job_id, time_t to_die = -1)
{
  SmallShell& smash = SmallShell::getInstance(); 
  smash.setFgJob(pid, line, job_id);
  shared_ptr<JobEntry> timed_job;
  if(to_die - _getTime() > 0)
  {
    timed_job = shared_ptr<JobEntry>
                              (new JobEntry(job_id, pid, line, _getTime(), FG_ACTIVE));
    smash.addTimedJob(to_die - _getTime(), timed_job);
  }
  waitpid(pid, nullptr, WUNTRACED);
  smash.setFgJob();
  smash.getJobs()->removeFinishedJobs();
}

void Command::removeRedirectionPart()
{
  size_t found = this->line.find(">");
  if(found == string::npos)
  {
    found = this->line.find(">>"); 
  }
  this->line = this->line.substr(0,found);
  vector<string>::iterator it;
  for (it = argv.begin(); it != argv.end(); it++)
  {
    if(*it == ">" || *it == ">>")
    {
      break;
    }
  }
  vector<string> subargv = {argv.begin() , it};
  this->argv = subargv;
}


void Command::setDuration(int duration)
{
  this->duration = duration;
}

void Command::setFullLine(const string full_line)
{
  this->full_line = full_line;
}

Command::Command(const string cmd_line, int duration)
    : line(cmd_line), full_line() ,duration(duration)  {
    _parseCommandLine(cmd_line, argv);
}

BuiltInCommand::BuiltInCommand(const string cmd_line) : Command(cmd_line) {}

string Command::getCommandLine() 
{ 
  return this->line; 
}

ChangePromptCommand::ChangePromptCommand (const string cmd_line) : BuiltInCommand(cmd_line){}

void ChangePromptCommand::execute()
{
  if(argv.size() > 1)
    SmallShell::getInstance().setPrompt(argv[1]);
  else
  {
    SmallShell::getInstance().setPrompt();
  }
}

ShowPidCommand::ShowPidCommand(const string cmd_line) : BuiltInCommand(cmd_line){}

void ShowPidCommand::execute()
{
  cout << "smash pid is " << SmallShell::getInstance().getPid() << endl;
}

GetCurrDirCommand::GetCurrDirCommand(const string cmd_line) : BuiltInCommand(cmd_line){}

void GetCurrDirCommand::execute()
{
  string dir = SmallShell::getInstance().getCurrPwd();
  cout << dir << endl;
}

ChangeDirCommand::ChangeDirCommand(const string cmd_line) : BuiltInCommand(cmd_line)
{
  if(argv.size() > 2 || argv.size() == 1)
  {
    throw TooManyArgs(this->argv[0]);
  }
}

void ChangeDirCommand::execute()
{
  if(argv[1] == "-")
  {
    if(!(SmallShell::getInstance().isOldPwdSet()))
    {
      throw OldpwdNotSet(this->argv[0]);
    }
    string oldpwd = SmallShell::getInstance().getOldPwd();
    SmallShell::getInstance().changeCurrentDirectory(oldpwd);
    return;  
  }
  SmallShell::getInstance().changeCurrentDirectory(this->argv[1]);
}

ExternalCommand::ExternalCommand(const string cmd_line, int duration) : 
                                Command(cmd_line, duration)
{
  this->is_background = _isBackgroundComamnd(cmd_line);
  this->line_no_background = this->line;
  _removeBackgroundSign(this->line_no_background);
}

void ExternalCommand::execute() 
{
  pid_t pid = fork();
  if (pid == ERROR) //failed
  {
    throw SyscallException("fork");
  }
  else if (pid == 0) //child
  {
    try
    {
      if (setpgrp() == ERROR)
      {
        throw SyscallException("setgrp");
      }
      if (execlp("/bin/bash", "/bin/bash", "-c", line_no_background.c_str(), nullptr) == ERROR) {
          throw SyscallException("execv");
      }
    } catch (...) {
        ;
    }
    exit(0);
    
  }
  else //father
  {
    SmallShell& smash = SmallShell::getInstance();
    if(this->duration > 0)
    {
      if (this->is_background)
      {
        int job_id = smash.getJobs()->getFGJobID();
        smash.addJob(this->full_line, pid, false, job_id);
        shared_ptr<JobEntry> job = smash.getJobs()->getJobById(job_id);
        smash.addTimedJob(this->duration, job);
      }
      else
      {
        runInFg(pid, this->full_line, smash.getJobs()->getFGJobID(), this->duration + _getTime());
      }
    }
    else if (this->is_background)
    {
      smash.addJob(this->line, pid, false);
    }
    else
    {
      runInFg(pid, this->line, smash.getJobs()->getFGJobID());
    }
  }
}

PipeCommand::PipeCommand(const string cmd_line) : Command(cmd_line)
{
  this->pipe_err = (this->line.find("|&") != string::npos);
  size_t index_pipe = this->line.find("|");
  this->left_cmd = this->line.substr(0,index_pipe);
  this->right_cmd = this->line.substr(index_pipe + 1 + this->pipe_err);
}

void PipeCommand::execute()
{
  SmallShell& smash = SmallShell::getInstance();
  _removeBackgroundSign(this->left_cmd);
  _removeBackgroundSign(this->right_cmd);
  int fd[2];
  if(pipe(fd) == ERROR)
  {
    throw SyscallException("pipe");
  }
  pid_t left_pid, right_pid;
  left_pid = fork();
  if(left_pid < 0)
  {
    throw SyscallException("fork");
  }
  else if(left_pid == 0)//left cmd
  {
    try
    {
      if (setpgrp() == ERROR)
      {
        throw SyscallException("setgrp");
      }
      int fd_to_close = this->pipe_err ? 2 : 1;
      if(close(fd_to_close) == ERROR)
      {
        throw SyscallException("close");
      }
      if(dup2(fd[1],fd_to_close) == ERROR)
      {
        throw SyscallException("dup2");
      }
      if(close(fd[1]) == ERROR)
      {
        throw SyscallException("close");
      }
      shared_ptr<Command> left = smash.createCommand(this->left_cmd);
      left->execute();
    } catch (SyscallException &err) {
        perror(err.what()); // TODO: check prints to stderr
    } catch (SmashException &err) {
        cerr << err.what(); // TODO: check prints to stderr
    } catch(...) {
      ;
    }
    exit(0);
  }

  else
  {
    if (close(fd[1]) == ERROR)
    {
      throw SyscallException("close");
    }
    right_pid = fork();
    if(right_pid < 0)
    {
      throw SyscallException("fork");
    }
    if(right_pid == 0)//right cmd
    {
      try
      {
        if (setpgrp() == ERROR)
        {
          throw SyscallException("setgrp");
        }
        if(close(0) == ERROR)
        {
          throw SyscallException("close");
        }
        if(dup2(fd[0],0) == ERROR)
        {
          throw SyscallException("dup2");
        }
        if(close(fd[0]) == ERROR)
        {
          throw SyscallException("close");
        }
        shared_ptr<Command> right = smash.createCommand(this->right_cmd);
        right->execute();
        if(close(0) == ERROR)// TODO: check if close need to be outside
        {
          throw SyscallException("close");
        }
      } catch (SyscallException &err) {
          perror(err.what()); // TODO: check prints to stderr
      } catch (SmashException &err) {
          cerr << err.what(); // TODO: check prints to stderr
      } catch(...) {
        ;
      }
      
      exit(0);
    }
  }
  if (waitpid(left_pid, nullptr, WUNTRACED) == ERROR)
  {
    throw SyscallException("waitpid");
  }

  if (waitpid(right_pid, nullptr, WUNTRACED) == ERROR)
  {
    throw SyscallException("waitpid");
  }
  if(close(fd[0]) == ERROR)
  {
    throw SyscallException("close");
  }
  
}
RedirectionCommand::RedirectionCommand(const string cmd_line) : Command(cmd_line)
{
  this->cat = (this->line.find(">>") != string::npos);
  size_t index_end_cmd = this->line.find(">");
  this->cmd = this->line.substr(0,index_end_cmd);
  string str = this->cat ? ">>" : ">";
  this->filename = this->line.substr(index_end_cmd + 1 + this->cat);
  this->filename = _trim(this->filename);
}

void RedirectionCommand::execute()
{
  SmallShell& smash = SmallShell::getInstance();
  int fd;
  if (this->cat)
  {
    fd = open(this->filename.c_str(), O_APPEND | O_CREAT | O_WRONLY, 00655);
    if (fd == ERROR)
    {
      this->cat = false;
    }
  }
  if(!this->cat)
  {
    fd = open(this->filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 00655);
    if (fd == ERROR)
    {
      throw SyscallException("open");
    }
  }
  pid_t pid = fork();
  if(pid < 0)
  {
    throw SyscallException("fork");
  }
  
  if(pid == 0)
  {
    try {
      if (setpgrp() == ERROR)
      { 
        throw SyscallException("setgrp");
      }
      close(1);
      dup2(fd, 1);
      shared_ptr<Command> cmd = smash.createCommand(this->cmd);
      cmd->execute();
    } catch (SyscallException &err) {
        perror(err.what()); // TODO: check prints to stderr
    } catch (SmashException &err) {
        cerr << err.what(); // TODO: check prints to stderr
    } catch (...) {
        ;
    }
    if(close(fd) == ERROR)
    { 
      throw SyscallException("close");
    }
    exit(0);
  }
  if (waitpid(pid, nullptr, WUNTRACED) == ERROR)
  {
    throw SyscallException("waitpid");
  }
}

JobsCommand::JobsCommand(const string cmd_line, shared_ptr<JobsList> jobs) : BuiltInCommand(cmd_line), jobs_list(jobs)
{}

void JobsCommand::execute()
{
  this->jobs_list->printJobsList();
}
KillCommand::KillCommand(const string cmd_line, shared_ptr<JobsList> jobs) : 
                                    BuiltInCommand(cmd_line), jobs_list(jobs)
{
  this->jobs_list->removeFinishedJobs(); //// added
  if (argv.size() != 3 || (argv[1])[0] != '-')
  {
    throw InvalidlArguments(this->argv[0]);
  }
  if(this->argv[2].substr(0,1) == "-" && is_num(this->argv[2].substr(1)))
  {
    int job = stoi(this->argv[2].substr(1));
    throw JobIdDoesntExist(this->argv[0], -job);
  }
  if(!is_num(argv[1].substr(1)) || !is_num(argv[2]))
  {
    throw InvalidlArguments(this->argv[0]);
  }
  
}

void KillCommand::execute()
{
  SmallShell& smash = SmallShell::getInstance();
  int job_id = stoi(argv[2]);
  int signum = stoi(argv[1].substr(1));
  if(signum <= 0 || signum > 64)
  {
    throw InvalidlArguments(this->argv[0]);
  }
  shared_ptr<JobEntry> job = smash.getJobs()->getJobById(job_id);
  if(job == nullptr)
  {
    throw JobIdDoesntExist(this->argv[0], job_id);
  }

  if (kill(job->getPid(), signum) == ERROR)
  {
    throw SyscallException("kill");
  }
  
  if(signum == SIGKILL)
  {
    this->jobs_list->removeJobById(job->getJobId());
  }
  this->jobs_list->removeFinishedJobs();
  cout << "signal number " << signum << " was sent to pid " << job->getPid() << endl;

}

ForegroundCommand::ForegroundCommand(const string cmd_line, shared_ptr<JobsList> jobs) : 
                                    BuiltInCommand(cmd_line), jobs_list(jobs)
{
  if(this->argv.size() > 2)
  {
    throw InvalidlArguments(this->argv[0]);
  }
  else if(this->argv.size() > 1 && is_num(this->argv[1]))
  {
    this->job_id = stoi(this->argv[1]);
  }
  else if(this->argv.size() > 1 && this->argv[1].substr(0,1) == "-" && is_num(this->argv[1].substr(1)))
  {
    int job = stoi(this->argv[1].substr(1));
    throw JobIdDoesntExist(this->argv[0], -job);
  }

  else if(this->argv.size() == 1)
  {
    this->job_id = NO_ID;
  }
  else
  {
    throw InvalidlArguments(this->argv[0]);
  }
}

void ForegroundCommand::execute()
{
  shared_ptr<JobEntry> job;
  if(this->job_id != NO_ID)
  {
    job = this->jobs_list->getJobById(this->job_id);
    if(job == nullptr)
    {
      throw JobIdDoesntExist(this->argv[0], this->job_id);
    }
  }
  else
  {
    if(this->jobs_list->isEmpty())
    {
      throw JobsListEmpty(this->argv[0]);
    }
    job = this->jobs_list->getMaxJob();
  }
  cout << job->getLine() << " : " << job->getPid() << endl;
  job->activate();
  this->jobs_list->removeJobById(this->job_id);
  runInFg(job->getPid(), job->getLine(), job->getJobId());
}

BackgroundCommand::BackgroundCommand(const string cmd_line, shared_ptr<JobsList> jobs) : 
                                    BuiltInCommand(cmd_line), jobs_list(jobs)
{
  if(this->argv.size() > 2)
  {
    throw InvalidlArguments(this->argv[0]);
  }
  else if(this->argv.size() == 1)
  {
    this->job_id = NO_ID;
  }
  else if(this->argv.size() > 1 && this->argv[1].substr(0,1) == "-" && is_num(this->argv[1].substr(1)))
  {
    int job = stoi(this->argv[1].substr(1));
    throw JobIdDoesntExist(this->argv[0], -job);
  }
  else if (!is_num(this->argv[1]))
  {
    throw InvalidlArguments(this->argv[0]);
  }
  else
  {
    this->job_id = stoi(this->argv[1]);
  }
}

void BackgroundCommand::execute() 
{
  shared_ptr<JobEntry> job;
  this->jobs_list->removeFinishedJobs(); // added
  if (this->job_id == NO_ID)
  {
    job = this->jobs_list->getLastStoppedJob();
    if (job == nullptr)
    {
      throw NoStoppedJobs(this->argv[0]);
    }
  }
  else
  {
    job = this->jobs_list->getJobById(this->job_id);
    if (job == nullptr)
    {
      throw JobIdDoesntExist(this->argv[0], this->job_id);
    }
    bool is_running = this->jobs_list->isJobRunning(this->job_id);
    if (is_running)
    {
      throw JobAlreadyRunBG(this->argv[0], this->job_id);
    }
  }
  cout<< job->getLine() << " : " << job->getPid() << endl;
  job->activate();
  //now continue job and change status in the list
}
QuitCommand::QuitCommand(const string cmd_line, shared_ptr<JobsList> jobs) : 
                                    BuiltInCommand(cmd_line), jobs_list(jobs)
{}

void QuitCommand::execute()
{
  if(this->argv.size() > 1 && this->argv[1] == "kill")
  {
    this->jobs_list->killAllJobs();
  }
  SmallShell::getInstance().stopRun();
}

TailCommand::TailCommand(const string cmd_line) : BuiltInCommand(cmd_line)
{
  if(this->argv.size() > 3 || this->argv.size() < 2)
  {
    throw InvalidlArguments(this->argv[0]);
  }
  if(this->argv.size() == 3)
  {
    if(this->argv[1].substr(0,1) != "-" || !is_num(this->argv[1].substr(1)))
    {
      throw InvalidlArguments(this->argv[0]);
    }
    this->num_lines = stoi(this->argv[1].substr(1));
    if(this->num_lines < 0)
    {
      throw InvalidlArguments(this->argv[0]);
    }
    this->file_name = this->argv[2];
  }
  else
  {
    this->num_lines = N;
    this->file_name = this->argv[1];
  }
}

void TailCommand::execute()
{
  int fd = open(this->file_name.c_str(), O_RDONLY);
  if(fd == ERROR)
  {
    throw SyscallException("open");
  }
  int fd_faster = open(this->file_name.c_str(), O_RDONLY);
  if(this->ReadNLines(this->num_lines, fd_faster) != SUCCESS) //file shorter than N lines
  {
    this->ReadNLines(this->num_lines, fd, 1);
  }
  else 
  {
    int status;
    while((status = this->ReadNLines(1, fd_faster)) == SUCCESS)//we need - success, fail, last line
    {
      this->ReadNLines(1, fd);
    }
    if(status == LAST_LINE)
    {
      this->ReadNLines(1, fd);
    }
    this->ReadNLines(this->num_lines, fd, 1);
  }
  if(close(fd) == ERROR)
  {
    close(fd_faster);
    throw SyscallException("close");
  }
  if(close(fd_faster) == ERROR)
  {
    throw SyscallException("close");
  }
}
int TailCommand::ReadNLines(int lines, int fd_read, int fd_write)
{
  int i = 0;
  int chars_in_line = 0;
  char ch;
  ssize_t rv = read(fd_read, &ch, 1);
  if(rv == ERROR)
  {
    throw SyscallException("read");
  }
  while(rv != ERROR && i < lines)
  {
    
    if(rv == 0) //EOF
    {
        i++;
        if(i == lines && chars_in_line > 0)
        {
          return LAST_LINE;
        }
        return -1;
    }
    else if(ch == '\n')
    {
        i++;
        chars_in_line = 0;
    }
    else 
    {
      chars_in_line++;
    }
    if(fd_write != -1 && write(fd_write, &ch, 1) == ERROR)
    {
      throw SyscallException("write");
    }
    if(i == lines)
    {
      return rv;
    }
    rv = read(fd_read, &ch, 1);
    if(rv == ERROR)
    {
      throw SyscallException("read");
    }
  }
  return rv;
}

TouchCommand::TouchCommand(const string cmd_line) :  BuiltInCommand(cmd_line), timestamp()
{
  if(this->argv.size() != 3)
  {
    throw InvalidlArguments(this->argv[0]);
  }
  this->file_name = this->argv[1];
  this->timestamp.tm_isdst = -1;
  strptime(this->argv[2].c_str(), "%S:%M:%H:%d:%m:%Y", &this->timestamp);
}

void TouchCommand::execute()
{
  time_t set_time = mktime(&this->timestamp);
  if(set_time == ERROR)
  {
    throw SyscallException("mktime");
  }
  utimbuf times;
  times.actime = set_time;
  times.modtime = set_time;
  if(utime(this->file_name.c_str(), &times) == ERROR)
  {
    throw SyscallException("utime");
  }
}
TimeoutCommand::TimeoutCommand(const string cmd_line) : BuiltInCommand(cmd_line)
{
  if(this->argv.size() <= 2 || !is_num(this->argv[1]))
  {
    throw InvalidlArguments(this->argv[0]);
  }
  this->duration = stoi(this->argv[1]);
  size_t found = this->line.find(this->argv[1]);
  this->cmd = this->line.substr(found + this->argv[1].size());
  this->cmd = _trim(this->cmd);
}

void TimeoutCommand::execute()
{
  if(this->duration == 0)
  {
    cout << "smash: got an alarm" << endl << "smash:" << this->line << " timed out!" << endl;
    return;
  }
  SmallShell& smash = SmallShell::getInstance();
  shared_ptr<Command> command = smash.createCommand(this->cmd);
  command->setFullLine(this->line);
  command->setDuration(this->duration);
  command->execute();
}
