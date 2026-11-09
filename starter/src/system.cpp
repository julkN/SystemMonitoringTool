#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

Processor& System::Cpu() {
    return cpu_; 
}

vector<Process>& System::Processes() {
    processes_.clear();
    vector<int> pids = LinuxParser::Pids();
    Process max(0);
    for (int& pid : pids) {
        processes_.emplace_back(pid);
    
    std::sort(processes_.begin(), processes_.end());
    }

    return processes_; 
}

std::string System::Kernel() { 
    std::string kernel = LinuxParser::Kernel();
    return kernel; 
}

float System::MemoryUtilization() { 
    return LinuxParser::MemoryUtilization(); 
}

std::string System::OperatingSystem() { 
    std::string os = LinuxParser::OperatingSystem();
    return os; 
}

int System::RunningProcesses() { 
    return LinuxParser::RunningProcesses(); 
}

int System::TotalProcesses() { 
    return LinuxParser::TotalProcesses(); 
}

long int System::UpTime() { 
    return LinuxParser::UpTime(); 
}
