#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include <fstream>
#include <vector>
#include <iterator>
#include <unistd.h>

using namespace std;

// Classic helper function
class Util {

public:

static std::string convertToTime ( long int input_seconds );
static std::string getProgressBar(std::string percent);
static void getStream(std::string path, std::ifstream& stream);
static std::vector<std::string> lineToVector(std::string line);
static float getClockFreq();
static std::string findMarkerInFile(std::string file_path, std::string marker);
};

std::string Util::convertToTime (long int input_seconds){
long minutes = input_seconds / 60;
long hours = minutes / 60;
long seconds = int(input_seconds%60);
minutes = int(minutes%60);
std::string result = std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds);
return result;
}
// constructing string for given percentage
// 50 bars is uniformly streched 0 - 100 %
// meaning: every 2% is one bar(|)
std::string Util::getProgressBar(std::string percent){

    std::string result = "0%% ";
    int _size= 50;
    int  boundaries;
    try {
        boundaries = (stof(percent)/100)*_size;
    } catch (...){
    boundaries = 0;
    }

    for(int i=0;i<_size;i++){
        if(i<=boundaries){
        result +="|";
        }
        else{
        result +=" ";
        }
    }

    result +=" " + percent.substr(0,5) + " /100%%";
    return result;
}

// wrapper for creating streams
void Util::getStream(std::string path, std::ifstream& stream){    
    stream.open (path, std::ifstream::in);
    if (!stream && !stream.is_open()){
        stream.close();        
        throw std::runtime_error("Non - existing PID");
    }
    //return stream;
}

// convert a line of words to a string vector 
std::vector<std::string> Util::lineToVector(std::string line)
{    
    istringstream line_stream(line);
    std::istream_iterator<std::string> beg(line_stream), end;
    vector<std::string> values(beg, end);

    return values;
}

// return the system frequency
float Util::getClockFreq()
{
    return sysconf(_SC_CLK_TCK);
}

// return the line containing a marker, in a file
std::string Util::findMarkerInFile(std::string file_path, std::string marker)
{
    std::ifstream file;
    std::string line;    

    // open the process file
    Util::getStream(file_path, file);    
    // search for the marker looping into every line
    while(std::getline(file, line)) {        
        if (line.compare(0, marker.size(), marker) == 0) {                 
            return line;
        }        
    }

    return "";
}

#endif