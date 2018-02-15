#include "ConfigParser.h"

using namespace std;

//constructor
ConfigParser::ConfigParser(string configFile) : config(configFile){
    if (config.is_open()){
        string line;
        string key;
        string value;

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
            vector<string> tmp;
            auto key_it =  series.insert(pair<string,vector<string> >(key,tmp));
            //insert data series values
            while (delimiter != string::npos){
                auto last_delimiter = delimiter;
                delimiter = line.find(",");
                value = line.substr(0, delimiter);
                (key_it.first)->second.push_back(value);
                //cout << "value: " << value << endl;
                line.erase(0,delimiter+1);
            }
            //insert dynamic values
            for (int i=0; i < DYNAMIC; i++){
                (key_it.first)->second.push_back(to_string(0));
            }
        }

        /* //test output
        for (std::pair<string, vector<string>> element : series) {
                cout << "key: " << element.first << endl;
                cout << "values: " << endl;
                for (auto elem: element.second){
                    cout << " " << elem << endl;
                }
        }
        */
        
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
map<string, vector<string> > ConfigParser::getSeries(){
    return series;
}
