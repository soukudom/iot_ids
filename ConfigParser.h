#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

#define DYNAMIC 7

enum configParams {SOFT_MIN, SOFT_MAX, HARD_MIN, HARD_MAX, SOFT_PERIOD, GROW_UP, GROW_DOWN, GROW_MODE, AVERAGE_TYPE, SERIES_LENGTH, PERIOD_CHECK, LEARNING_LENGTH, STORE_MODE, MODEL_REFRESH, EXPORT_INTERVAL, ROTATE, AVERAGE, PREV_VALUE, SX,SX2};

class ConfigParser{
    public:
        ConfigParser(string configFile);
        virtual ~ConfigParser();
        map<string, vector<string> > getSeries();
    private:
        map<string, vector<string> > series; // parsed data from configuration file
        ifstream config; // configuration file
        
};
