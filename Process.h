#include <string>
//#include "ProcessParser.h"

using namespace std;
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
private:
    string pid;
    string user;
    string cmd;
    string cpu;
    string mem;
    string upTime;

public:
    Process(string pid){
        this->pid       = pid;
        this->user      = ProcessParser::getProcUser(pid);
        this->mem       = ProcessParser::getVmSize(pid);
        this->cmd       = ProcessParser::getCmd(pid);
        this->upTime    = ProcessParser::getProcUpTime(pid);
        this->cpu       = ProcessParser::getCpuPercent(pid);        
    }
    void setPid(int pid);
    string getPid()const;
    string getUser()const;
    string getCmd()const;
    int getCpu()const;
    int getMem()const;
    string getUpTime()const;
    string getProcess();
};
void Process::setPid(int pid){
    this->pid = pid;
}
string Process::getPid()const {
    return this->pid;
}
string Process::getProcess(){
    if(!ProcessParser::isPidExisting(this->pid))
        return "";
    this->mem = ProcessParser::getVmSize(this->pid);
    this->upTime = ProcessParser::getProcUpTime(this->pid);
    this->cpu = ProcessParser::getCpuPercent(this->pid);

    std::ostringstream line;
    line << std::setw(7)  << std::left  << this->pid.substr(0, 8) 
         << std::setw(10) << std::left  << this->user.substr(0, 8)         
         << std::setw(7)  << std::right << this->cpu.substr(0, 4)
         << std::setw(12) << std::right << this->mem.substr(0, 8)
         << std::setw(16) << std::right << this->upTime.substr(0, 8)
         << "     "
         << std::setw(55) << std::left  << this->cmd.substr(0, 40) + "...";

    return line.str();

}
