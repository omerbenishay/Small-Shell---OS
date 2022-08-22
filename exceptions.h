#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <exception>
#include <typeinfo>
#include <string>
#include <memory>
#include "Commands.h"

using namespace std;


class SmashException : public exception //std::exception
{};

class TooManyArgs : public SmashException {
    string error_message;
public:
    TooManyArgs(const string cmd_name) 
    {
        this->error_message = "smash error: " + cmd_name + ":" + " too many arguments" + "\n"; 
    }
    ~TooManyArgs() = default;
    const char* what() const noexcept override
    {
        return error_message.c_str();
    }
};

class OldpwdNotSet : public SmashException {
    string error_message;
public:
    OldpwdNotSet(const string cmd_name)
    {
        this->error_message = "smash error: " + cmd_name + ":" + " OLDPWD not set" + "\n"; 
    }    
    const char* what() const noexcept override { return error_message.c_str(); }
};

class SyscallException : public exception {
    string error_message;
  public:
    SyscallException(string syscall_name) 
    {
        this->error_message = "smash error: " + syscall_name + " failed";
    }
    const char* what() const noexcept override { return error_message.c_str(); }
};

class InvalidlArguments : public SmashException {
    string error_message;
public:
    InvalidlArguments(const string cmd_name)
    {
        this->error_message = "smash error: " + cmd_name + ":" + " invalid arguments" + "\n"; 
    }    
    const char* what() const noexcept override { return error_message.c_str(); }
};

class JobIdDoesntExist : public SmashException {
    string error_message;
public:
    JobIdDoesntExist(const string cmd_name, int job_id)
    {
        this->error_message = "smash error: " + cmd_name + ": job-id " + to_string(job_id) + " does not exist" + "\n"; 
    }    
    const char* what() const noexcept override { return error_message.c_str(); }
};

class JobAlreadyRunBG : public SmashException {
    string error_message;
public:
    JobAlreadyRunBG(const string cmd_name, int job_id)
    {
        this->error_message = "smash error: " + cmd_name + ": job-id " + to_string(job_id) + " is already running in the background" + "\n"; 
    }  
    const char* what() const noexcept override { return error_message.c_str(); }
};


class NoStoppedJobs : public SmashException {
    string error_message;
public:
    NoStoppedJobs(const string cmd_name)
    {
        this->error_message = "smash error: " + cmd_name + ": there is no stopped jobs to resume" + "\n"; 
    }  
    const char* what() const noexcept override { return error_message.c_str(); }
};

class JobsListEmpty : public SmashException {
    string error_message;
public:
    JobsListEmpty(const string cmd_name)
    {
        this->error_message = "smash error: " + cmd_name + ": jobs list is empty" + "\n"; 
    }  
    const char* what() const noexcept override { return error_message.c_str(); }
};


#endif /*EXCEPTIONS_H*/