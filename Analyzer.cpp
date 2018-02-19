#include "Analyzer.h"
#include "ConfigParser.h"

//constructor
//default
Analyzer::Analyzer() = default;
//param
Analyzer::Analyzer(map<string, map<string, vector<string> > > meta_data): series_meta_data(meta_data) {}

//destructor
Analyzer::~Analyzer() = default;

/* 
 * BEGIN CALCALULATION METHODS
 */
double Analyzer::getMedian(map<int,vector<double> >::iterator &sensor_it){
    //basic non-moving median calculation
    double median;
    size_t size = sensor_it->second.size();

    sort(sensor_it->second.begin(), sensor_it->second.end());
    if (size  % 2 == 0) {
        median = (sensor_it->second[size / 2 - 1] + sensor_it->second[size / 2]) / 2;
    }
    else {
        median = sensor_it->second[size / 2]; 
    }
    return median;
}

double Analyzer::cumulativeAverage(map<int,vector<double> >::iterator &sensor_it, map<string,map<string, vector<string> > >::iterator &meta_it){
    //alg source: https://en.wikipedia.org/wiki/Moving_average
    double new_value = sensor_it->second.back();
    double delta_average = stod (meta_it->second["meta"][AVERAGE],nullptr); 
    delta_average = ( new_value + ( sensor_it->second.size() ) * delta_average ) / ( sensor_it->second.size() + 1 );
    meta_it->second["meta"][AVERAGE] = to_string(delta_average); 
    return delta_average;
}

pair<double, double> Analyzer::getAverageAndVariance(string &ur_field, uint64_t *ur_id, map<string,map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it){
    map<int, vector<double> >::iterator x_it;
    map<int, vector<double> >::iterator x2_it;
    
    int series_length = stoi (meta_it->second["general"][SERIES_LENGTH],nullptr);
    double new_value = sensor_it->second.back();
    double variance = 0;
    double average = 0;
    double sx = 0;
    double sx2 = 0;

    //initialize time window
    if (x[ur_field][*ur_id].size() <= series_length){
        cout << "add value to the average/variance meta series" << endl;
        //add new data to time window
        x[ur_field][*ur_id].push_back(new_value);
        x2[ur_field][*ur_id].push_back(new_value*new_value);

        //count sum of values in window
        meta_it->second["meta"][SX] = to_string(new_value + stod(meta_it->second["meta"][SX],nullptr));
        meta_it->second["meta"][SX2] = to_string(new_value*new_value+ stod(meta_it->second["meta"][SX2],nullptr));
        
        return pair<double, double> (-1,-1);
    }
    //count moving average and variance
    else {
        cout << "cont average and average. stop adding" << endl;
        //change values in time window
        rotate(x[ur_field][*ur_id].begin(), x[ur_field][*ur_id].begin()+1, x[ur_field][*ur_id].end());
        rotate(x2[ur_field][*ur_id].begin(), x2[ur_field][*ur_id].begin()+1, x2[ur_field][*ur_id].end());

        double new_x = new_value; 
        double new_x2 = new_value*new_value;

        double y = x[ur_field][*ur_id].back();
        double y2 = x2[ur_field][*ur_id].back();

        //cout << "oldest_value x before " << x[ur_field][*ur_id].back() << endl;
        //cout << "oldest value x2 before" <<  x2[ur_field][*ur_id].back() << endl;
        x[ur_field][*ur_id].back() = new_x;
        x2[ur_field][*ur_id].back() = new_x2; 
        //cout << "oldest_value x after " << x[ur_field][*ur_id].back() << endl;
        //cout << "oldest value x2 after" <<  x2[ur_field][*ur_id].back() << endl;

        //cout << "sx_before" << meta_it->second[SX] << endl;
        //cout << "sx2_before " << meta_it->second[SX2] << endl;
        meta_it->second["meta"][SX] = to_string(stod(meta_it->second["meta"][SX],nullptr) + new_x - y);
        meta_it->second["meta"][SX2] = to_string(stod(meta_it->second["meta"][SX2],nullptr) + new_x2 - y2);
        //cout << "sx_after" << meta_it->second[SX] << endl;
        //cout << "sx2_after" << meta_it->second[SX2] << endl;

        sx = stod(meta_it->second["meta"][SX],nullptr);
        sx2 = stod(meta_it->second["meta"][SX2],nullptr);

        average = sx/series_length; 
        variance = (series_length*sx2 - (sx*sx)) / (series_length*(series_length-1));
        
        return pair<double,double> (average, variance);
    }
}
/* 
 * END CALCALULATION METHODS
 */

/*
 * BEGIN TIME SERIES SERVICE AND INIT METHODS
 */

//store date in data series 
double Analyzer::pushData(double *ur_data, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it){
    double new_value = 0;
    string store_mode = meta_it->second["general"][STORE_MODE];
    int series_length = stoi (meta_it->second["general"][SERIES_LENGTH],nullptr);

    //save data based on store mode
    if (store_mode == "simple"){
        //sensor_it->second.push_back(*ur_data);
        new_value = *ur_data;
        //return *ur_data;
    }
    else if (store_mode == "average"){
        double average = stod (meta_it->second["meta"][AVERAGE],nullptr);
        new_value = *ur_data - average;
        //sensor_it->second.push_back(new_value);        
        //return new_value;
    }
    else if (store_mode == "delta"){
        double prev_value = stod (meta_it->second["meta"][PREV_VALUE],nullptr);
        new_value = *ur_data - prev_value;
        meta_it->second["meta"][PREV_VALUE] = to_string(*ur_data);
        //sensor_it->second.push_back(new_value);        
        //return new_value;
    }
    else {
        cerr << "ERROR: Unknown mode" << endl;
    }

    //insert new value
    if (series_length >= sensor_it->second.size()){
        //push back
        sensor_it->second.push_back(new_value);        
    }
    else {
        //back()
        sensor_it->second.back() = new_value;
    }
    return new_value;

}

void Analyzer::modifyProfile(string &ur_field, uint64_t *ur_id ,map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it){
    int flag = 0; //decision flag

    //determine profile values
    for (auto profile_values:  meta_it->second["profile"]){
        if ( profile_values == "median" ){
            //median method
            cout << "median method" << endl;
        }
        else if (profile_values == "average" || profile_values == "variance") {
            //moving varinace and average method
            //skip unnecessary calls
            if (flag == 1 ){
                continue;
            }
            pair<double, double> determine_values = getAverageAndVariance(ur_field, ur_id, meta_it, sensor_it);
            meta_it->second["meta"][AVERAGE] = to_string(determine_values.first);
            meta_it->second["meta"][VARIANCE] = to_string(determine_values.second);
            flag = 1;
        }
        else if (profile_values == "cum_average"){
            //cum moving average method
            cout << "cum_average method" << endl;
        }
    }
}

int Analyzer::initSeries(string &ur_field, uint64_t *ur_id, double *ur_data){
    map<string, map<string, vector<string> > >::iterator meta_it;
    map<int, vector<double> >::iterator sensor_it;


    //test if meta information exists
    meta_it = series_meta_data.find(ur_field);
    if (meta_it != series_meta_data.end()){

        //cast necessary meta info values
        int learning_length = stoi (meta_it->second["general"][LEARNING_LENGTH],nullptr);
        int series_length = stoi (meta_it->second["general"][SERIES_LENGTH],nullptr);
        int rotate_cnt = stoi (meta_it->second["meta"][ROTATE],nullptr);

        //test if sensor id exists
        sensor_it = control[ur_field].find(*ur_id);
        if ( sensor_it != control[ur_field].end() ){

            //learning profile phase
            if ( learning_length > sensor_it->second.size() + rotate_cnt){
                //cout << "series created -> learning phase" << endl;
                if (series_length >= sensor_it->second.size()){ 
                    cout << "learning phase" << endl; 
                    //push data 
                    pushData(ur_data, meta_it, sensor_it);
                    //sensor_it->second.push_back(*ur_data);
                    //modify profile values
                    modifyProfile(ur_field,ur_id,meta_it, sensor_it);
                    return 4;
                }
                //rotate values in learning phase
                else {
                    cout << "rotate values" <<endl;
                    rotate( sensor_it->second.begin(), sensor_it->second.begin()+1, sensor_it->second.end());
                    //push data 
                    pushData(ur_data, meta_it, sensor_it);
                    //sensor_it->second.back() = *ur_data;
                    rotate_cnt++;
                    meta_it->second["meta"][ROTATE] = to_string(rotate_cnt);
                    //modify profile values
                    modifyProfile(ur_field,ur_id,meta_it, sensor_it);
                    return 3;
                }
            }
            else {
                //learning phase has been finished
                //modify profile values
                //modifyProfile(ur_field,ur_id,meta_it, sensor_it);
                cout << "learning phase finished -> start analyzing" << endl;
                return 0;
            }
        }
        //sensor id not found - create new record 
        else {
            vector<double> tmp;
            tmp.push_back(*ur_data);
            control[ur_field].insert(pair<int, vector<double> >(*ur_id,tmp));
            meta_it->second["meta"][AVERAGE] = to_string(*ur_data);
            meta_it->second["meta"][PREV_VALUE] = to_string(*ur_data);
            sensor_it = control[ur_field].find(*ur_id);
            cout << "new value was created" << endl;
            //modify profile values
            modifyProfile(ur_field,ur_id,meta_it, sensor_it);
            return 2;
        }
    }
    else {
     //   cerr << "INFO: Field " << ur_field <<  " meta specification missig. Skipping... " << endl;
        return 1;
    }
}

/*
 * END TIME SERIES SERVICE AND INIT METHODS
 */

/* 
 * BEGIN TIME SERIES PROCESS
 */

//print series data
void Analyzer::printSeries(string &ur_field){
    cout << "field: " << ur_field << endl;;
    for (auto element : control[ur_field]){
        cout << " key: " << element.first << endl; 
        cout << "  values: " << endl;
        cout << "   ";
        for (auto elem: element.second){
            cout << elem << ", ";
        }
        cout << endl;
        cout << "   ROTATE, AVERAGE, PREV_VALUE, SX, SX2, VARIANCE" << endl;
        cout << "   ";
        for (auto meta: series_meta_data[ur_field]["meta"] ){
            cout << meta << ", ";
        }
        cout << endl;
    }
}

//data series processing
void Analyzer::processSeries(string ur_field, uint64_t *ur_id, int *ur_time, double *ur_data) {
    //initialize data series
    /*return 
    * 0 - init done
    * 1 - ur_field is not specified in the template
    * 2 - new value was created
    * 3 - values were rotated
    * 4 - new item was added
    */
    initSeries(ur_field, ur_id, ur_data);
    printSeries(ur_field);
    
    //analyze data series
    //analyzeData(ur_field, ur_id, ur_data);
}

/* 
 * END TIME SERIES PROCESS
 */



