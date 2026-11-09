#include "processor.h"
#include "linux_parser.h"
#include <string>
#include <fstream>
#include <sstream>


// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { 
    std::string line, strCpu, strUser, strNice, strSystem, strIdle, strIOwait, strIRQ, strSoftIRQ, strSteal;
    std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
    if (stream.is_open()) {
        std::getline(stream, line);
        std::istringstream linestream(line);
        linestream >>  strCpu >> strUser >> strNice >> strSystem >> strIdle >> strIOwait >> strIRQ >> strSoftIRQ >> strSteal ;
        
        long user = std::stol(strUser);
        long nice = std::stol(strNice);
        long system = std::stol(strSystem);
        long idle = std::stol(strIdle);
        long iowait = std::stol(strIOwait);
        long irq = std::stol(strIRQ);
        long softirq = std::stol(strSoftIRQ);
        long steal = std::stol(strSteal);

        long current_idle = idle + iowait;
        long current_non_idle = user + nice + system + irq + softirq + steal;
        
        long current_total = current_idle + current_non_idle;

        long total_delta = current_total - previous_total_;
        long idle_delta = current_idle - previous_idle_;

        previous_total_ = current_total;
        previous_idle_ = current_idle;
        
        if(total_delta == 0)
            return 0.0;

        return static_cast<float>(total_delta - idle_delta) / static_cast<float>(total_delta);
    }
    return 0.0; 
}