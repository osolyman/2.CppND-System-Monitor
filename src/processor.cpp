#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
   
  long activeJiffies = LinuxParser::ActiveJiffies();
  long idleJiffies = LinuxParser::IdleJiffies();
  
  long total = activeJiffies + idleJiffies;
  return static_cast<float>(activeJiffies) / total;
}