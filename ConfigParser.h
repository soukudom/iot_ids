#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

#define DYNAMIC 10

enum localValues {SOFT_MIN, SOFT_MAX, HARD_MIN, HARD_MAX, SOFT_PERIOD, GROW_UP, GROW_DOWN, COMPARE_MODE, PERIOD_CHECK};

enum general {SERIES_LENGTH, LEARNING_LENGTH, STORE_MODE, MODEL_REFRESH, EXPORT_INTERVAL};

enum metaValues {ROTATE, AVERAGE, PREV_VALUE, SX, SX2, VARIANCE};



class ConfigParser{
    public:
        ConfigParser(string configFile);
        virtual ~ConfigParser();
        map<string, map<string, vector<string> > > getSeries();
    private:
        map<string, map<string, vector<string> > > series; // parsed data from configuration file
        ifstream config; // configuration file
};
