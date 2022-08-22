# Small-Shell---OS

An implementaion of a "small shell". Written and tested in Ubuntu 18.04 LTS.
To run it, you should run make in the terminal and then run the smash that created.

Include the implementaion of:
1. chprompt - allowing the user to change the prompt that being displayed
    Command format:
    chprompt <new-prompt>
2. showpid - prints the smash pid.
3. pwd - prints the full path of the current working directory. 
4. cd - Change directory (cd) command
    Command format:
    cd <new-path>
5. jobs - prints the jobs list which contains:
    1. unfinished jobs (which are running in the background).
    2. stopped jobs (which were stopped by pressing Ctrl+Z while they are running)
6. kill - sends a signal whose number is specified by <signum> to a job whose sequence ID in jobs list is <job-id>
    Command format:
    kill -<signum> <jobid>
7. fg - brings a stopped process or a process that runs in the background to the foreground.
    Command format:
    fg [job-id]
8. bg - resumes one of the stopped processes in the background.
    Command format:
    bg [job-id]
9. quit - exits the smash. If the kill argument was specified(optional) then smash should kill all of its
   unfinished and stopped jobs before exiting.
    Command format:
    quit [kill]
10. Pipes and IO redirection
11. tail - prints the last N lines of the file which it is given 
    Command format:
    tail [-N] <file-name>
12. touch - updates the file’s last access and modification timestamps to be the time specified in the <timestamp> argument
    Command format: 
    touch <file-name> <timestamp>
13. timeout - sets an alarm for ‘duration’ seconds, and runs the given ‘command’ as though it was given to the smash directly, and when the time is up it sends a SIGKILL to the given command’s process.
    Command format:
    timeout <duration> <command>
14. Signal Handlers: 
    1. SIGINT
    2. SIGTSTP
    3. SIG_ALRM
15. any other command(ls for example) should work(the program uses the code in /bin/bash).
