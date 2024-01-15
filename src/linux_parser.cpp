#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
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
    filestream.close();
  } else {
    std::cerr << "Error: Unable to open the file " << kOSPath << std::endl;
    return "Unknown";
  }  
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    if(std::getline(stream, line)) {
      std::istringstream linestream(line);
      if (linestream >> os >> version >> kernel) {
        stream.close();
        return kernel;
      }
    }
    stream.close();
  } else {
    std::cerr << "Error: Unable to open file " << kProcDirectory + kVersionFilename << std::endl;
    return "Unknown";
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  if (directory != nullptr) {
    struct dirent* file;
    while ((file = readdir(directory)) != nullptr) {
      if (file->d_type == DT_DIR) {
        std::string filename(file->d_name);
        if (std::all_of(filename.begin(), filename.end(), isdigit)) {
          try {
            int pid = std::stoi(filename);
            pids.push_back(pid);
          } catch (const std::invalid_argument& err) {
            std::cerr << "Error converting string to integer: " << err.what() << std::endl;
          } catch (const std::out_of_range& err) {
            std::cerr << "Error (out of range): " << err.what() << std::endl;
          }
        }
      }
    }
    closedir(directory);
  } else {
    std::cerr << "Error: Unable to open directory " << kProcDirectory << std::endl;
  }
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::string line;
  std::string key;
  std::string totalMemoryStr;
  std::string freeMemoryStr;
  float freeMemory = 0.0f;
  float totalMemory = 0.0f;
  try {
    std::ifstream filestream(kProcDirectory + kMeminfoFilename);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        linestream >> key;
        if (key == "MemTotal:") {
          linestream >> totalMemoryStr;
          if (!totalMemoryStr.empty()) {
            totalMemory = std::stof(totalMemoryStr);
          }
        } else if (key == "MemFree:") {
          linestream >> freeMemoryStr;
          if (!freeMemoryStr.empty()) {
            freeMemory = std::stof(freeMemoryStr);
          }
        }
      }
      filestream.close();
    } else {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + kMeminfoFilename);
    }
    if (totalMemory > 0.0f) {
      return (totalMemory - freeMemory) / totalMemory;
    } else {
      throw std::runtime_error("Error: MemTotal is zero or negative.");
    }
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return 0.0f;
  }
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  std::string line;
  std::string uptimeStr;
  long uptime = 0;
  try {
    std::ifstream filestream(kProcDirectory + kUptimeFilename);
    if (filestream.is_open()) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> uptimeStr;
      if (!uptimeStr.empty()) {
        uptime = std::stol(uptimeStr);
      }
      filestream.close();
    } else {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + kUptimeFilename);
    }
    return uptime;
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return 0;
  }
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return ActiveJiffies() + IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string value;
  std::istringstream linestream;
  try {
    std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
    if (filestream.is_open()) {
      std::getline(filestream, line);
      linestream.str(line);
      for (int i = 0; i < 14; i++) {
        linestream >> value;
      }
      long utime, stime, cutime, cstime;
      linestream >> utime >> stime >> cutime >> cstime;

      if (linestream.fail()) {
        throw std::invalid_argument("Invalid inputs: " + line);
      }
      filestream.close();
      return utime + stime + cutime + cstime;
    } else {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + to_string(pid) + kStatFilename);
    }
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return 0;
  }
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpu = CpuUtilization();
  long activeJiffies = 0;
  try {
    activeJiffies += stol(cpu[kUser_]);
    activeJiffies += stol(cpu[kNice_]);
    activeJiffies += stol(cpu[kSystem_]);
    activeJiffies += stol(cpu[kIRQ_]);
    activeJiffies += stol(cpu[kSoftIRQ_]);
    activeJiffies += stol(cpu[kSteal_]);
    activeJiffies += stol(cpu[kGuest_]);
    activeJiffies += stol(cpu[kGuestNice_]);
  } catch (const std::invalid_argument& err) {
    activeJiffies = 0;
  }
  return activeJiffies;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpu = CpuUtilization();
  long idleJiffies = 0;
  try {
    idleJiffies += stol(cpu[kIdle_]);
    idleJiffies += stol(cpu[kIOwait_]);
  } catch (const std::invalid_argument& err) {
    idleJiffies = 0;
  }
  return idleJiffies;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line;
  string key;
  string value;
  vector<string> cpu;
  try {
    std::ifstream filestream(kProcDirectory + kStatFilename);
    if (filestream.is_open()) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "cpu") {
        while (linestream >> value) {
          cpu.push_back(value);
        }
      }
      filestream.close();
    } else {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + kStatFilename);
    }
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return {};
  }
  return cpu;
}  

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;
  try {
    std::ifstream filestream(kProcDirectory + kStatFilename);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        linestream >> key;
        if (key == "processes") {
          linestream >> value;
          if (!value.empty()) {
            return stoi(value);
          } else {
            return 0;
          }
        }
      }
    } else {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + kStatFilename);
    }
  } catch (const std::invalid_argument& err) {
    std::cerr << "Invalid argument: " << err.what() << std::endl;
    return 0;
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return 0;
  }
  return 0;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key, value;
  try {
    std::ifstream filestream(kProcDirectory + kStatFilename);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        linestream >> key;
        if (key == "procs_running") {
          linestream >> value;
          if (!value.empty()) {
            return stoi(value);
          } else {
            return 0;
          }
        }
      }
    } else {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + kStatFilename);
    }
  } catch (const std::invalid_argument& err) {
    std::cerr << "Invalid argument: " << err.what() << std::endl;
    return 0;
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return 0;
  }
  return 0;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
std::string LinuxParser::Command(int pid) {
  std::string line;
  try {
    std::ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
    if (filestream.is_open()) {
      std::getline(filestream, line);
      return line;
    } else {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + std::to_string(pid) + kCmdlineFilename);
    }
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return std::string();
  }
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string line;
  string key, value;
  try {
    std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        linestream >> key >> value;
        if (key == "VmSize:") {
          if (!value.empty()) {
            try {
              long kbRam = std::stol(value);
              long mbRam = kbRam / 1024;
              return std::to_string(mbRam);
            } catch (const std::invalid_argument& err) {
              return std::string();
            }
          } else {
            return std::string();
          }
        }
      }
    } else {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + std::to_string(pid) + kStatusFilename);
    }
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return std::string();
  }
  return std::string();
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line;
  string key, value;
  try {
    std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        linestream >> key >> value;
        if (key == "Uid:") {
          return value;
        }
      }
    } else {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + std::to_string(pid) + kStatusFilename);
    }
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return std::string();
  }
  return std::string();
}  

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
std::string LinuxParser::User(int pid) {
  std::string line;
  std::string uid = Uid(pid);
  std::ifstream filestream;
  try {
    filestream.open(kPasswordPath);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        auto userFound = line.find(":" + uid + ":");
        if (userFound != std::string::npos) {
          auto pass = line.find(":");
          if (pass != std::string::npos) {
            filestream.close();
            return line.substr(0, pass);
          }
        }
      }
    } else {
      throw std::runtime_error("Error: Unable to open file " + kPasswordPath);
    }
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    filestream.close();
    return std::string();
  }
  filestream.close();
  return std::string();
}  
  
// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  string line;
  string value = "0";
  try {
    std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
    if (!filestream.is_open()) {
      throw std::runtime_error("Error: Unable to open file " + kProcDirectory + std::to_string(pid) + kStatFilename);
    }
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 22; i++) {
      linestream >> value;
    }
    if (!value.empty()) {
      return stol(value) / sysconf(_SC_CLK_TCK);
    } else {
      return 0;
    }
  } catch (const std::invalid_argument& err) {
    std::cerr << "Invalid argument: " << err.what() << std::endl;
    return 0;
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return 0;
  }
  return 0;
}
