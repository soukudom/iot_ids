#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

#define DYNAMIC 11

enum localValues {SOFT_MIN, SOFT_MAX, HARD_MIN, HARD_MAX, SOFT_PERIOD, GROW_UP, GROW_DOWN, S_MIN_LIMIT, S_MAX_LIMIT};

enum general {SERIES_LENGTH, LEARNING_LENGTH, STORE_MODE, PERIODIC_CHECK, EXPORT_INTERVAL};

/*enum metaProfile {P_ROTATE, P_AVERAGE, P_PREV_VALUE, P_SX, P_SX2, P_VARIANCE, P_MEDIAN, P_CUM_AVERAGE};

enum metaData {D_AVERAGE,D_VARIANCE,D_MEDIAN,D_CUM_AVERAGE,D_SX,D_DX2, D_PREV_VALUE};
*/
enum meta {AVERAGE, VARIANCE, MEDIAN, CUM_AVERAGE, SX, SX2, PREV_VALUE, NEW_VALUE, LAST_TIME, ROTATE, CHECKED_FLAG};


class ConfigParser{
    public:
        ConfigParser(string configFile);
        virtual ~ConfigParser();
        map<string, map<string, vector<string> > > getSeries();
    private:
        map<string, map<string, vector<string> > > series; // parsed data from configuration file
        ifstream config; // configuration file
};


//PERIODIC_CHECK jsem presunul do general, protoze kdyz neprijdou data, tak se nezmeni zadna hodnota
