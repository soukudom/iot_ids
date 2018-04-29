/**
 * \file ConfigParser.cpp
 * \brief Parse file with configuration of unirec field.
 * \author Dominik Soukup <soukudom@fit.cvut.cz>
 * \date 2018
**/

#include "ConfigParser.h"

using namespace std;

// Constructor
ConfigParser::ConfigParser(string configFile) : config(configFile){}

// Parse configuration file
void ConfigParser::parseFile(){
    if (config.is_open()){
        // Local tmp store variables
        string line;         // One line from configuration file
        string key;          // Name of config record (unirec field+id)
        string main_key;     // Name of unirec field
        uint64_t main_id;    // Name of record ID
        string value;        // Config params for one key 
        string multi_key;    // Name of composite value  
        string multi_value;  // Config params for one composite key
        string simple_value; // One item of multi value record
        size_t multi_item;   // Tmp local value for storing position in config line

        while(getline(config,line)){
            // Erase whitespace
            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
            // Erase comments and empty lines
            if(line[0] == '#' || line.empty()) continue;
            // Parse config record key (ur_field + id)
            auto delimiter = line.find(":");
            key = line.substr(0, delimiter);
            multi_item = key.find(";");
            // Ur_field with id has been found
            if (multi_item != string::npos){
                main_key = key.substr(0, multi_item);
                istringstream iss(key.substr(multi_item+1,delimiter));
                iss >> main_id; 
            // Key with no ID -> use default ID value
            } else {
                main_key = key;
                main_id = 0;
            }
            line.erase(0,delimiter+1);

            // Insert ur_field to the map
            while (delimiter != string::npos){
                auto last_delimiter = delimiter;
                // Parse next config field
                delimiter = line.find(";");
                value = line.substr(0, delimiter);
                multi_item = value.find("(");
                // Composite key has been found -> name(param1, param2, ...,)
                if (multi_item != string::npos){
                    // Parse and save to the proper index
                    multi_key = value.substr(0, multi_item);
                    value.erase(0,multi_item+1);
                    multi_item = value.find(")");
                    value = value.substr(0, multi_item); 
                    // Parse values for composite key
                    size_t multi_value = value.find(",");
                    // Multi key (more than one item) -> value parsing needed
                    if (multi_value != string::npos){
                        while (multi_value != string::npos){
                            simple_value = value.substr(0,multi_value);
                            series[main_key][main_id][multi_key].push_back(simple_value);
                            value.erase(0,multi_value+1);
                            multi_value = value.find(",");
                        }
                    // Simple value -> save directly
                    } else {
                        series[main_key][main_id][multi_key].push_back(value);
                        value.erase(0,multi_item+1);
                    }   
                    line.erase(0,delimiter+1);
                // General key has been found -> just value without brackets
                } else {
                    series[main_key][main_id]["general"].push_back(value);
                    line.erase(0,delimiter+1);
                }
            }
            // Insert dynamic initialization values
            for (int i=0; i < DYNAMIC; i++){
                series[main_key][main_id]["metaProfile"].push_back(to_string(0));
                series[main_key][main_id]["metaData"].push_back("x");
            }
        }
    } else {
        cerr << "ERROR: Unable to open the configuration fle " << endl;
    }
} 

// Destructor
ConfigParser::~ConfigParser(){
    config.close();
}

// Getter function
map<string, map<uint64_t, map<string, vector<string> > > > ConfigParser::getSeries(){
    return series;
}
