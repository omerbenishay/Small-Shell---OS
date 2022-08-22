#ifndef JOBS_H_
#define JOBS_H_

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <sys/wait.h>

//#include "Commands.h"
using namespace std;

#define EMPTY_JOB_ID 0
#define NO_JOB 0
enum JobStatus { STOPPED, FINISHED, BG_ACTIVE, FG_ACTIVE };//might be distructive

class Command;
time_t _getTime();

class JobEntry {
    int job_id;
    pid_t pid;
    string cmd_line;
    time_t insertion_time;
    JobStatus job_status;
  public:
    JobEntry(int job_id, pid_t pid, string cmd_line, time_t insertion_time, JobStatus job_status);
    ~JobEntry() = default;
    bool isFinished();
    bool isStopped();
    void print(bool full_print = true);
    pid_t getPid () const;
    int getJobId() const;
    void activate();
    void updateStatus();
    string getLine();
    time_t getTime() const;
};

class JobsList {
    map<int, shared_ptr<JobEntry>> job_entries;
    int max_job_id;
 // TODO: Add your data members
 public:
  JobsList();
  ~JobsList() = default;
  void addJob(string cmd_line, pid_t pid,  bool is_stopped = false, int job_id = NO_JOB);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  shared_ptr<JobEntry> getJobById(int jobId);
  void removeJobById(int jobId);
  shared_ptr<JobEntry> getLastJob();
  shared_ptr<JobEntry> getLastStoppedJob();
  int getFGJobID();
  pid_t getPid(int job_id);
  bool isEmpty() const;
  shared_ptr<JobEntry> getMaxJob() ;
  bool isJobRunning(int job);
  
  // TODO: Add extra methods or modify exisitng ones as needed
};

class TimedJobsList {
  map<time_t, vector<shared_ptr<JobEntry>>> job_entries;
  public:
  TimedJobsList() = default;
  ~TimedJobsList() = default;
  void addTimedJob(int alarm, shared_ptr<JobEntry> job);
  void gotAlarm();
};
#endif //JOBS_H_