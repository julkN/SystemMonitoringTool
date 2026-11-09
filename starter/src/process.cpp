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

int Process::Pid() {
    return pid_; 
}

float Process::CpuUtilization() { 
    long active_jiffies = LinuxParser::ActiveJiffies(pid_);
    long uptime = LinuxParser::UpTime(pid_);
    long hertz = sysconf(_SC_CLK_TCK);
    if (uptime == 0) {
        return 0.0;
    }
    return static_cast<float>(active_jiffies) / static_cast<float>(hertz) / static_cast<float>(uptime);
}

string Process::Command() { 
    return LinuxParser::Command(pid_);
}

string Process::Ram() { 
    return LinuxParser::Ram(pid_); 
}

string Process::User() { 
    return LinuxParser::User(pid_);
}

long int Process::UpTime() { 
    return LinuxParser::UpTime(pid_); 
}

bool Process::operator<(Process const& a) const {
    Process p1 = *this;
    Process p2 = a;
    return p2.CpuUtilization() < p1.CpuUtilization();
}    