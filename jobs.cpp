//#include <iostream>
#include "jobs.h"
//#include "small_shell.h"
#include "Commands.h"
using namespace std;
#include <vector>

#define ERROR -1
#define DOSENT_EXIST 0

time_t _getTime() {
    time_t curr_time;
    if (time(&curr_time) == ERROR) {
        throw SyscallException("time");
    }
    return curr_time;
}

JobEntry::JobEntry(int job_id, pid_t pid, string cmd_line,
                   time_t insertion_time, JobStatus job_status)
    : job_id(job_id), pid(pid), cmd_line(cmd_line),
      insertion_time(insertion_time), job_status(job_status) {}

void JobEntry::updateStatus()   
{
    if(this->job_status == STOPPED || getpid() != SmallShell::getInstance().getPid())
    {
        return;
    }
    pid_t pid = waitpid(this->pid, nullptr, WNOHANG);
    if (pid != 0)
    {
        this->job_status = FINISHED;
    }
    else
    {
        this->job_status = BG_ACTIVE;
    }
}
bool JobEntry::isFinished() 
{
    this->updateStatus();
    return (this->job_status == FINISHED);
}

bool JobEntry::isStopped()
{
    this->updateStatus();
    return (this->job_status == STOPPED);
}

string JobEntry::getLine()
{
    return this->cmd_line;
}

void JobEntry::print(bool full_print)
{
    if(!full_print)
    {
        cout << this->pid << ": " << this->cmd_line << endl;
    }
    else
    {
        double diff = difftime(_getTime(), this->insertion_time);
        cout<<"["<<this->job_id<<"] "<< this->cmd_line << " : "<< this->pid << " " << diff << " secs";
        if (this->isStopped())
        {
            cout<<" (stopped)";
        }
        cout << endl;
    }
}

pid_t JobEntry::getPid () const
{
    return this->pid;
}

int JobEntry::getJobId() const
{
    return this->job_id;
}

void JobEntry::activate()
{
    if (kill(this->pid, SIGCONT) == ERROR) 
    {
        throw SyscallException("kill");
    }
    this->job_status = BG_ACTIVE;
}
time_t JobEntry::getTime() const
{
    return this->insertion_time;
}

JobsList::JobsList() : job_entries(), max_job_id(EMPTY_JOB_ID) {}

void JobsList::addJob(string cmd_line, pid_t pid, bool is_stopped, int job_id)
{
    this->removeFinishedJobs();
    int new_job_id = job_id;
    if(job_id == NO_JOB)
    {
        this->max_job_id ++;
        new_job_id = this->max_job_id;
    }
    time_t time = _getTime();
    JobStatus status = (is_stopped) ? STOPPED : BG_ACTIVE;
    shared_ptr<JobEntry> new_job (new JobEntry(new_job_id, pid, cmd_line, time, status));
    this->job_entries[new_job_id] = new_job;
}

void JobsList::printJobsList() 
{
    this->removeFinishedJobs();
    for (auto it = this->job_entries.begin(); it != this->job_entries.end(); it++)
    {
        (*it->second).print();
    }
}

void JobsList::removeFinishedJobs()
{
    if (getpid() != SmallShell::getInstance().getPid())
    {
        return;
    }
    int max = EMPTY_JOB_ID;
    vector<int> jobs_to_erase;
    for (auto it = this->job_entries.begin(); it != this->job_entries.end(); it++)
    {
        if ((*(it->second)).isFinished())
        {
            jobs_to_erase.push_back(it->first);
        }
        else
        {
            max = (max < (it->first)) ? (it->first) : max;
        }
    }
    this->max_job_id = max;
    for (auto it = jobs_to_erase.begin(); it != jobs_to_erase.end(); it++)
    {
        this->job_entries.erase(*it);
    }
}

int JobsList::getFGJobID()
{
    this->removeFinishedJobs();
    this->max_job_id++;
    return this->max_job_id;
}

pid_t JobsList::getPid(int job_id)
{
    if (this->job_entries.find(job_id) == this->job_entries.end())
    {
        return DOSENT_EXIST;
    }
    return this->job_entries[job_id]->getPid();//TODO: Eficciency
}
bool JobsList::isEmpty() const
{
    return this->job_entries.empty();
}
shared_ptr<JobEntry> JobsList::getMaxJob() 
{
    return this->job_entries[this->max_job_id];
}

bool JobsList::isJobRunning(int job_id)
{
    this->removeFinishedJobs();

    auto it = this->job_entries.find(job_id);
    if (it == this->job_entries.end())
    {
        return false;
    }
    return !(it->second->isStopped());
}
shared_ptr<JobEntry> JobsList::getJobById(int job_id)
{
    this->removeFinishedJobs();
    auto it = this->job_entries.find(job_id);
    if (it == this->job_entries.end())
    {
        return nullptr;
    }
    return it->second;
}
shared_ptr<JobEntry> JobsList::getLastStoppedJob()
{
    for (auto iter = this->job_entries.rbegin(); iter != this->job_entries.rend(); ++iter) 
    {
        if(iter->second->isStopped())
        {
            return iter->second;
        }
    }
    return nullptr;
}
void JobsList::killAllJobs()
{
    this->removeFinishedJobs();
    cout << "smash: sending SIGKILL signal to " << this->job_entries.size() << " jobs:" << endl;
    for (auto it = this->job_entries.begin(); it != this->job_entries.end(); it++)
    {
        it->second->updateStatus();
        if(!(it->second->isFinished()))
        {
            if (kill(it->second->getPid(), SIGKILL) == ERROR) 
            {
                throw SyscallException("kill");
            }
            it->second->print(false);
        }
    }
    //this->removeFinishedJobs();

}
void JobsList::removeJobById(int jobId)
{
    this->job_entries.erase(jobId);
}




void TimedJobsList::addTimedJob(int sec, shared_ptr<JobEntry> job)
{
    
    time_t to_die = sec + job->getTime();
    auto it = this->job_entries.find(to_die);
    if (it == this->job_entries.end())
    {
        this->job_entries[to_die] = vector<shared_ptr<JobEntry>>();
    }
    this->job_entries[to_die].push_back(job);
    it = this->job_entries.begin();
    if(it == this->job_entries.end())
    {
        //TODO:: 
        cout << "time problem!!!!!!" << endl;
    }
    alarm(it->first - _getTime());
}

void TimedJobsList::gotAlarm()
{
    time_t curr = _getTime();
    auto it = this->job_entries.find(curr);
    if (it != this->job_entries.end())
    {
        for (auto itr = it->second.begin(); itr != it->second.end(); itr++)
        {
            (*itr)->updateStatus();
            if(!(*itr)->isFinished())
            {
                if (kill((*itr)->getPid(), SIGKILL) == ERROR) 
                {
                throw SyscallException("kill");
                }
                cout << "smash: " << (*itr)->getLine() << " timed out!" << endl;
            }
        }
        it->second.clear();
        this->job_entries.erase(it);
    }

    it = this->job_entries.begin();
    if(it == this->job_entries.end())
    {
        return;
    }
    alarm(it->first - _getTime());
}