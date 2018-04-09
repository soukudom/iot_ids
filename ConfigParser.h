/**
 * \file ConfigParser.h
 * \brief Parse file with configuration of unirec field.
 * \author Dominik Soukup <soukudom@fit.cvut.cz>
 * \date 2018
**/
/* NOTE: Detailed explanation of this code is in documentation. */

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;

// Number of values in meta structure
#define DYNAMIC 15

/* 
 The following enums are used for accessing the created structures after parsing the configuraiton file.  
*/

// Parameters and fields for each value in profile. 
enum localValues {SOFT_MIN, SOFT_MAX, HARD_MIN, HARD_MAX, SOFT_PERIOD, GROW_UP, GROW_DOWN, S_MIN_LIMIT, S_MAX_LIMIT};

// Parameters for every analyzed unirec field.
enum general {SERIES_LENGTH, LEARNING_LENGTH, IGNORE_LENGTH, STORE_MODE, PERIODIC_CHECK, PERIODIC_INTERVAL, EXPORT_INTERVAL};

// Parameters and fields used in analyse process
enum meta {AVERAGE, VARIANCE, MEDIAN, CUM_AVERAGE, SX, SX2, PREV_VALUE, NEW_VALUE, LAST_TIME, ROTATE, CHECKED_FLAG, OLDEST_VALUE, NEW_ORIG_VALUE, CHANGE_PERIOD, CNT};

/*
 Parse file with configuration of unirec field.
*/
class ConfigParser{
    public:
        /*
        * Constructor 
        * Parse data in configuration file into series structure
        * /param[in] configFile Name of configuration file
        */
        ConfigParser(string configFile);
        /*
        * Destructor
        */
        virtual ~ConfigParser();
        /*
        * Method that returns parsed structure from configuration file
        */
        map<string, map<uint64_t, map<string, vector<string> > > > getSeries();
    private:
        map<string, map<uint64_t, map<string, vector<string> > > > series; // parsed data from configuration file. Data sequence: unirec field, ur_id, subsection category (profile, profile items, export, general, metaData, metaProfile, profile), config params
        ifstream config; // configuration filename
};
