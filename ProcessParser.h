#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"


using namespace std;

class ProcessParser{
private:
    std::ifstream stream;
    public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static int getNumberOfCores();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
};

/*
* return the virtual memory size
* 
* @param pid    (in) pid of process we want to analyze
*
* @return       the amount of the vm size for the process
*/
std::string ProcessParser::getVmSize(std::string pid) 
{
    string marker = "VmData";
    string line;    
    ifstream file;

    // full path of the file to open
    std::string path = Path::basePath() + pid + "/" + Path::statusPath();

    // search for the marker in the file
    line = Util::findMarkerInFile(path, marker);
    if (!line.empty()) {
        // split the line into a string vector            
        vector<string> values = Util::lineToVector(line);

        // the value in the file is like 
        // VmData:    42836 kB
        // so the value we need is at posision 1 of the vector
        // The value is converted from kB to GB                    
        return to_string(stof(values[1]) / float(1024*1024));
    }    

    return "0";
}


/*
* return the amount of cpu used
* 
* @param pid    (in) pid of process we want to analyze
*
* @return       the amount of cpu used
*/
string ProcessParser::getCpuPercent(string pid)
{
    ifstream file;
    string line;

    // open the process file
    Util::getStream(Path::basePath() + pid + "/" + Path::statPath(), file);

    // the file only has one line
    std::getline(file, line);
    vector<string> values = Util::lineToVector(line);

    // get the values
    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float start_time = stof(values[21]);
    float uptime = ProcessParser::getSysUpTime();
    float clock_freq = Util::getClockFreq();
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (start_time/clock_freq);

    return to_string(100.0 * ((total_time/clock_freq)/seconds));
}


/*
* return the uptime for the process
* 
* @param pid    (in) pid of process we want to analyze
*
* @return       the uptime in seconds
*/
string ProcessParser::getProcUpTime(string pid)
{
    ifstream file;
    string line;

    // open the process file
    Util::getStream(Path::basePath() + pid + "/" + Path::statPath(), file);

    std::getline(file, line);
    vector<string> values = Util::lineToVector(line);

    return to_string(stof(values[13])/Util::getClockFreq());
}


/*
* return the uptime for the system
* 
* @param pid    (in) pid of process we want to analyze
*
* @return       the uptime in seconds
*/
long ProcessParser::getSysUpTime()
{
    ifstream file;
    string line;

    // open the process file
    Util::getStream(Path::basePath() + Path::upTimePath(), file);

    std::getline(file, line);
    vector<string> values = Util::lineToVector(line);

    return (stoi(values[0]));
}

/*
* return the owner of a process
* 
* @param pid    (in) pid of process we want to analyze
*
* @return       the owner of a process
*/
std::string ProcessParser::getProcUser(string pid)
{
    string marker = "Uid:";
    string line;
    string user_id;    

    // full path of the file to open
    std::string path = Path::basePath() + pid + "/" + Path::statusPath();

    // search for the marker in the file
    line = Util::findMarkerInFile(path, marker);
    if (!line.empty()) {
        vector<string> values = Util::lineToVector(line);
        // user id value
        user_id = values[1];        
    }
    
    std::ifstream file;
    // open the process file
    Util::getStream("/etc/passwd", file);

    // search the uid in the passwd file
    marker = "x:" + user_id;    

    // search for the VmData info looping into every line
    while(std::getline(file, line)) {                
        if (line.find(marker) != std::string::npos) {
            // get the user name (username:user_id)            
            return line.substr(0, line.find(":"));
        }
    }

    return "";
}


/*
* return the list of pids in the system
* 
* @return       list of pids
*/
std::vector<string> ProcessParser::getPidList()
{
    DIR *dir;
            
    // open the /proc dir
    if ((dir = opendir("/proc")) == NULL) {
        throw std::runtime_error(std::strerror(errno));
    }

    auto lIsDigit = [](char c){ return std::isdigit(c); };
    std::vector<string> pids;

    // scan the directory to extract pids    
    while (dirent *dirp = readdir(dir)) {
        if (dirp->d_type != DT_DIR)
            continue;
        
        // check if all the chars of the dir name are digits
        if (std::all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), lIsDigit)) {            
            pids.push_back(dirp->d_name);
        }
    }

    // close the /proc dir
    if (closedir(dir)) {
        throw std::runtime_error(std::strerror(errno));
    }
    
    return pids;
}

/*
* return the command that executed a pid
* 
* @param pid    (in) pid of process we want to analyze
*
* @return       command that executed the pid
*/
std::string ProcessParser::getCmd(string pid)
{    
    ifstream file;
    string line;

    // full path of the file to open
    std::string path = Path::basePath() + pid + "/" + Path::cmdPath();

    Util::getStream(path, file);
    std::getline(file, line);

    return line;
}

/*
* return the number of cpu cores of the host
*
* @return       number of cpu cores
*/
int ProcessParser::getNumberOfCores()
{
    ifstream file;

    // full path of the file to open
    std::string path = Path::basePath() + "cpuinfo";

    // search for the marker
    string marker = "cpu cores";
    string line;
    
    line = Util::findMarkerInFile(path, marker);
    if (!line.empty()) {        
        vector<string> values = Util::lineToVector(line);
        // user id value
        return stoi(values[3]);
    }

    return 0;
}


/*
* get the percentage of cpu usage
*
* @param coreNumber     the number of the core to get info about
*
* @return               info about cpu percentage usage
*/
vector<string> ProcessParser::getSysCpuPercent(string coreNumber)
{
    // full path of the file to open
    std::string path = Path::basePath() + Path::statPath();

    // search for the marker
    string marker = "cpu" + coreNumber;
    string line;
    
    line = Util::findMarkerInFile(path, marker);
    if (!line.empty()) {        
        return Util::lineToVector(line);        
    }

    return (vector<string>());
}

/*
* get the amount of time for a cpu
*
* @param cpuValues      vector with cpu time values
*
* @return               amount of cpu time
*/
float getSysActiveCpuTime(vector<string> cpuValues)
{
    if (cpuValues.empty()) {        
        return 0.0;
    }    

    return (stof(cpuValues[S_USER])     +            
            stof(cpuValues[S_NICE])     +
            stof(cpuValues[S_SYSTEM])   +
            stof(cpuValues[S_IRQ])      +
            stof(cpuValues[S_SOFTIRQ])  +
            stof(cpuValues[S_STEAL])    +
            stof(cpuValues[S_GUEST])    +
            stof(cpuValues[S_GUEST_NICE]));
}

/*
* get the idle time of a cpu
*
* @param cpuValues      vector with cpu time values
*
* @return               amount of idle time
*/
float getSysIdleCpuTime(vector<string> cpuValues)
{
    if (cpuValues.empty()) {        
        return 0.0;
    }    

    return (stof(cpuValues[S_IDLE]) +            
            stof(cpuValues[S_IOWAIT]));
}


/*
* calculate cpu time stats
*
* @param values1        vector containing cpu data
* @param values2        vector containing cpu data
*
* @return               percentage of cpu active time
*/
string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2)
{
    float activeTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    float idleTime   = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    float totaltime  = activeTime + idleTime;
    float result     = 100.0*(activeTime/totaltime);

    return to_string(result);
}

/*
* calculate percentage of used cpu
*
* @param values1        vector containing cpu data
* @param values2        vector containing cpu data
*
* @return               percentage of cpu active time
*/
float ProcessParser::getSysRamPercent()
{
    // full path of the file to open
    std::string path = Path::basePath() + Path::memInfoPath();

    // search for the marker
    string marker1 = "MemAvailable:";
    string marker2 = "MemFree:";
    string marker3 = "Buffers:";
    string line;
    
    std::ifstream file;

    // open the process file
    Util::getStream(path, file);
    
    vector<string> values;
    float memAvailable = -1;
    float memFree = -1;
    float buffers = -1;

    // search for the marker looping into every line
    while(std::getline(file, line)) {           
        if (line.compare(0, marker1.size(), marker1) == 0) {
            values = Util::lineToVector(line);
            memAvailable = stof(values[1]);
        }
        else if (line.compare(0, marker2.size(), marker2) == 0) {
            values = Util::lineToVector(line);
            memFree = stof(values[1]);            
        }
        else if (line.compare(0, marker3.size(), marker3) == 0) {
            values = Util::lineToVector(line);
            buffers = stof(values[1]);            
        }

        // exit the loop if we collected all the data we need
        if ((memAvailable >= 0) &&
            (memFree >= 0) &&
            (buffers >= 0))
            break;
    }

    return (100.0*(1-(memFree / (memAvailable-buffers))));
}


/*
* return the kernel version
**
* @return               kernel version
*/
string ProcessParser::getSysKernelVersion()
{
    // full path of the file to open
    std::string path = Path::basePath() + Path::versionPath();

    // search for the marker
    string marker = "Linux version ";    

    string line = Util::findMarkerInFile(path, marker);
    if (!line.empty()) {        
        vector<string> values = Util::lineToVector(line);        
        return values[2];
    }

    return "";
}

/*
* get the os release name
**
* @return               os release name
*/
string ProcessParser::getOSName()
{
    // full path of the file to open
    std::string path = "/etc/os-release";

    // search for the marker
    string marker = "PRETTY_NAME=";

    string line = Util::findMarkerInFile(path, marker);    
    if (!line.empty()) {   
        // extract the substring
        std::size_t pos = line.find("=");
        string os_name = line.substr(++pos);        
        // remove the '"' char if found
        os_name.erase(std::remove(os_name.begin(), os_name.end(), '"'), os_name.end());
        
        return os_name;
    }

    return "";
}


/*
* get the number of all the threads in the system
**
* @return               the total number of threads
*/
int ProcessParser::getTotalThreads()
{
    // get the list of all the pids
    vector<string> pids = getPidList();
    if (pids.empty())
        return 0;

    string marker = "Threads:";
    string line;
    string path;
    vector<string> values;
    int totalThreads = 0;

    // loop into the pids list to get the number of threads for every process
    for (auto &pid: pids) {
        // full path of the file to open
        path = Path::basePath() + pid + "/" + Path::statusPath();
        line = Util::findMarkerInFile(path, marker);
        if (!line.empty()) {
            values = Util::lineToVector(line);
            totalThreads += stoi(values[1]);
        }
    }

    return totalThreads;
}


/*
* get the total number of processes
**
* @return               the total number of processes
*/
int ProcessParser::getTotalNumberOfProcesses()
{
    string marker = "processes";

    // full path of the file to open
    string path = Path::basePath() + Path::statPath();
    string line = Util::findMarkerInFile(path, marker);
    if (!line.empty()) {
        vector<string> values = Util::lineToVector(line);
        return stoi(values[1]);        
    }

    return 0;
}


/*
* get the number of the running processes
**
* @return               the total number of running processes
*/
int ProcessParser::getNumberOfRunningProcesses()
{
    string marker = "procs_running";

    // full path of the file to open
    string path = Path::basePath() + Path::statPath();
    string line = Util::findMarkerInFile(path, marker);
    if (!line.empty()) {
        vector<string> values = Util::lineToVector(line);
        return stoi(values[1]);
    }

    return 0;
}

/*
* check if a pid is still existing
**
* @return               true or false
*/
 bool ProcessParser::isPidExisting(string pid)
 {
    string path = Path::basePath() + pid;
    ifstream stream;

    stream.open (path, std::ifstream::in);
    if (!stream && !stream.is_open()){
        return FALSE;        
    }

    return TRUE;
 }