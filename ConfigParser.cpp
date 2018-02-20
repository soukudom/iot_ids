#include "ConfigParser.h"

using namespace std;

//constructor
ConfigParser::ConfigParser(string configFile) : config(configFile){
    if (config.is_open()){
        string line;
        string key;
        string value;
        string multi_key;
        string multi_value;
        string simple_value;

        while(getline(config,line)){
            //erase whitespace
            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
            //erase comments and empty lines
            if( line[0]=='#' || line.empty()) continue;
            //parse ur_field
            auto delimiter = line.find(":");
            key = line.substr(0, delimiter);
            line.erase(0,delimiter+1);

            //insert ur_field to the map
            while (delimiter != string::npos){
                auto last_delimiter = delimiter;
                //parse next config field
                delimiter = line.find(";");
                value = line.substr(0, delimiter);
                size_t multi_item = value.find("(");
                //composite key has been found
                if (multi_item != string::npos){
                    //parse and save to the proper index
                    multi_key = value.substr(0, multi_item);
                    value.erase(0,multi_item+1);
                    size_t multi_item = value.find(")");
                    value = value.substr(0, multi_item); 
                    //parse values
                    size_t multi_value = value.find(",");
                    //multi key (more than one item) -> value parsing needed
                    if (multi_value != string::npos){
                        while (multi_value != string::npos){
                            simple_value = value.substr(0,multi_value);
                            series[key][multi_key].push_back(simple_value);
                            value.erase(0,multi_value+1);
                            multi_value = value.find(",");
                        }
                    }
                    //simple value -> save directly
                    else {
                        series[key][multi_key].push_back(value);
                        value.erase(0,multi_item+1);
                    }   
                    line.erase(0,delimiter+1);
                }
                //general key has been found
                else {
                    //cout << "general value: " << value << endl; 
                    series[key]["general"].push_back(value);
                    line.erase(0,delimiter+1);
                }
            }
            //insert dynamic values
            for (int i=0; i < DYNAMIC; i++){
                series[key]["metaProfile"].push_back(to_string(0));
            }
        }
        config.close();
    }
    else{
        cerr << "ERROR: Unable to open the configuration fle " << configFile << endl;
    }
} 

//destructor
ConfigParser::~ConfigParser(){
    config.close();
}

//getter
map<string, map<string, vector<string> > > ConfigParser::getSeries(){
    return series;
}
