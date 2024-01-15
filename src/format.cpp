#include <string>
#include <iomanip>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long times) {
  //converting the int number of seconds I get into hours, minutes and seconds:
  int hours = times / 3600; // dividing all seconds I have over 3600
  int minutes = (times%3600) / 60; // dividing the remainder of the last calculation of hours to devide it on 60 to get the minutes
  int seconds = times%60; // dividing the remainder of the last calculation of minutes to devide it on 60 to get the seconds
  
  std::ostringstream formTime;
  formTime << std::setw(2) << std::setfill('0') << hours << ":"
    		<< std::setw(2) << std::setfill('0') << minutes << ":"
    		<< std::setw(2) << std::setfill('0') << seconds;
  return formTime.str();
}