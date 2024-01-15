#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid_) : pid(pid_){}

// TODO: Return this process's ID
int Process::Pid() {
  return pid;
}

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() const {
  long active_jiffies = LinuxParser::ActiveJiffies(pid);
  long total_jiffies = LinuxParser::Jiffies();
  
  if(total_jiffies > 0) {
    return static_cast<float>(active_jiffies) / total_jiffies;
  }  
  return 0.0;
}

// TODO: Return the command that generated this process
string Process::Command() {
  return LinuxParser::Command(Pid());
}

// TODO: Return this process's memory utilization
string Process::Ram() {
  return LinuxParser::Ram(Pid());
}

// TODO: Return the user (name) that generated this process
string Process::User() {
  return LinuxParser::User(Pid());
}

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() {
  return LinuxParser::UpTime(pid);
}

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const {
  return (CpuUtilization() < a.CpuUtilization());
}