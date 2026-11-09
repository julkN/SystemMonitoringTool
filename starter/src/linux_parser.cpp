#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

constexpr int kJiffiesSize = 8;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  for (const auto& entry : std::filesystem::directory_iterator(kProcDirectory)) {
    if (entry.is_directory()) {
      string filename = entry.path().filename().string();
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

float LinuxParser::MemoryUtilization() { 
  std::string line, key;
  float value= 0.0;
  float memTotal = 0.0;
  float memFree = 0.0;
  float buffers = 0.0;
  float cached = 0.0;
  float shmen = 0.0;
  float s_reclaimable = 0.0;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          memTotal = value;
        } else if (key == "MemFree:") {
          memFree = value;
        } else if (key == "Buffers:") {
          buffers = value;
        } else if (key == "Cached:") {
          cached = value;
        } else if (key == "Shmem:") {
          shmen = value;
        } else if (key == "SReclaimable:") {
          s_reclaimable = value;
        }
      }
    }
  }
  if (memTotal == 0)
    return 0.0;
  float totalMemUsed = memTotal - memFree;
  float totalMemCached = cached + s_reclaimable - shmen;
  float totalnonCacheBuffMemUsed = totalMemUsed - buffers - totalMemCached;
  float memUtilization = totalnonCacheBuffMemUsed / memTotal;
  return memUtilization;
}

long LinuxParser::UpTime() { 
  std::string line, value1, value2;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> value1 >> value2;
    return std::stol(value1);
  }
  return 0; 
}

long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}


long LinuxParser::ActiveJiffies(int pid) { 
  std::string line, value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    int i = 0;
    long utime, stime, cutime, cstime;
    while (linestream >> value) {
      if (i == 13) utime = std::stol(value);
      else if (i == 14) stime = std::stol(value);
      else if (i == 15) cutime = std::stol(value);
      else if (i == 16) cstime = std::stol(value);
      i++;
    }
    return utime + stime + cutime + cstime;
  }
  return 0; 
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  std::vector<std::string> cpu_values = LinuxParser::CpuUtilization();
  if(cpu_values.size() < 8) return 0;
  return std::stol(cpu_values[LinuxParser::kUser_]) + 
         std::stol(cpu_values[LinuxParser::kNice_]) + 
         std::stol(cpu_values[LinuxParser::kSystem_]) + 
         std::stol(cpu_values[LinuxParser::kIRQ_]) + 
         std::stol(cpu_values[LinuxParser::kSoftIRQ_]) + 
         std::stol(cpu_values[LinuxParser::kSteal_]);
} 

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  std::vector<std::string> cpu_values = LinuxParser::CpuUtilization();
  if(cpu_values.size() < 8) return 0;
  return std::stol(cpu_values[LinuxParser::kIdle_]) + 
         std::stol(cpu_values[LinuxParser::kIOwait_]);
} 

// TODO: Read and return CPU utilization
std::vector<std::string> LinuxParser::CpuUtilization() {
  std::string line, cpu_value, value;
  int i = 0;
  std::vector<std::string> cpuValues;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu_value;
    while (linestream >> value) {
      if(i>7) break;
      cpuValues.push_back(value);
      i++;
    }
  }
  return cpuValues;
}

int LinuxParser::TotalProcesses() { 
  std::string line, key;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "processes") {
          int value;
          linestream >> value;
          return value;
        }
      }
    }
  }
  return 0; 
}


int LinuxParser::RunningProcesses() { 
  std::string line, key;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "procs_running") {
          int value;
          linestream >> value;
          return value;
        }
      }
    }
  }
  return 0; 
}

string LinuxParser::Command(int pid) { 
  std::string line;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    return line;
  }
  return string();
}

string LinuxParser::Ram(int pid) { 
  std::string line, key;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "VmRSS:") {
          std::string value;
          linestream >> value;
          long tmp_value = std::stol(value);
          long value_in_MB = tmp_value / 1024;
          return std::to_string(value_in_MB);
        }
      }
    }
  }
  return string(); 
}

string LinuxParser::Uid(int pid) { 
  std::string line, key;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "Uid:") {
          std::string value;
          linestream >> value;
          return value;
        }
      }
    }
  }
  return string(); 
}

string LinuxParser::User(int pid) { 
  std::string uid = Uid(pid);
  std::string line, name, x, id;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> name >> x >> id) {
        if (id == uid) {
          return name;
        }
      }
    }
  } 
  return string(); 
}

long LinuxParser::UpTime(int pid) { 
  std::string line, value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    std::string value;
    int i = 0;
    long starttime;
    while (linestream >> value) {
      if (i == 21) {
        starttime = std::stol(value);
        break;
      }
      i++;
    }
    long uptime = LinuxParser::UpTime();
    long hertz = sysconf(_SC_CLK_TCK);
    long seconds = uptime - (starttime / hertz);
    return seconds;
  }
  return 0; 
}
