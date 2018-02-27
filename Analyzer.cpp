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
    double median = 0;
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

double Analyzer::getCumulativeAverage(map<int,vector<double> >::iterator &sensor_it, map<string,map<string, vector<string> > >::iterator &meta_it, string meta_id){
    //alg source: https://en.wikipedia.org/wiki/Moving_average
    double new_value = sensor_it->second.back();
    //first value exception
    if (sensor_it->second.size() == 1){
        return new_value;
    }
    double delta_average = stod (meta_it->second[meta_id][CUM_AVERAGE],nullptr); 
    delta_average = ( new_value + ( sensor_it->second.size() ) * delta_average ) / ( sensor_it->second.size() + 1 );
    return delta_average;
}

pair<double, double> Analyzer::getAverageAndVariance(string &ur_field, uint64_t *ur_id, map<string,map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id){
    //alg source: https://www.dsprelated.com/showthread/comp.dsp/97276-1.php
    map<int, vector<double> >::iterator x_it;
    map<int, vector<double> >::iterator x2_it;
    
    int series_length = stoi (meta_it->second["general"][SERIES_LENGTH],nullptr);
    double new_value = sensor_it->second.back();
    double variance = 0;
    double average = 0;
    double sx = 0;
    double sx2 = 0;

    //initialize time window
    if (x[ur_field][*ur_id].size() < series_length){
        //add new data to time window
        x[ur_field][*ur_id].push_back(new_value);
        x2[ur_field][*ur_id].push_back(new_value*new_value);

        //count sum of values in window
        meta_it->second[meta_id][SX] = to_string(new_value + stod(meta_it->second[meta_id][SX],nullptr));
        meta_it->second[meta_id][SX2] = to_string(new_value*new_value+ stod(meta_it->second[meta_id][SX2],nullptr));
        
        return pair<double, double> (-1,-1);
    }
    //count moving average and variance
    else {
        //change values in time window
        rotate(x[ur_field][*ur_id].begin(), x[ur_field][*ur_id].begin()+1, x[ur_field][*ur_id].end());
        rotate(x2[ur_field][*ur_id].begin(), x2[ur_field][*ur_id].begin()+1, x2[ur_field][*ur_id].end());

        //do a calculation
        double new_x = new_value; 
        double new_x2 = new_value*new_value;

        double y = x[ur_field][*ur_id].back();
        double y2 = x2[ur_field][*ur_id].back();

        x[ur_field][*ur_id].back() = new_x;
        x2[ur_field][*ur_id].back() = new_x2; 

        meta_it->second[meta_id][SX] = to_string(stod(meta_it->second[meta_id][SX],nullptr) + new_x - y);
        meta_it->second[meta_id][SX2] = to_string(stod(meta_it->second[meta_id][SX2],nullptr) + new_x2 - y2);

        sx = stod(meta_it->second[meta_id][SX],nullptr);
        sx2 = stod(meta_it->second[meta_id][SX2],nullptr);

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

//store data in data series based on configured values
double Analyzer::pushData(double *ur_data, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id){
    double new_value = 0;
    string store_mode = meta_it->second["general"][STORE_MODE];
    int series_length = stoi (meta_it->second["general"][SERIES_LENGTH],nullptr);

    //save data based on store mode
    if (store_mode == "simple"){
        new_value = *ur_data;
    }
    else if (store_mode == "average"){
        double average = stod (meta_it->second[meta_id][AVERAGE],nullptr);
        new_value = *ur_data - average;
    }
    else if (store_mode == "delta"){
        double prev_value = stod (meta_it->second[meta_id][PREV_VALUE],nullptr);
        new_value = *ur_data - prev_value;
        meta_it->second[meta_id][PREV_VALUE] = to_string(*ur_data);
    }
    else {
        cerr << "ERROR: Unknown mode" << endl;
    }

    //insert new value
    if (series_length > sensor_it->second.size()){
        //push back
        sensor_it->second.push_back(new_value);        
    }
    else {
        //back()
        sensor_it->second.back() = new_value;
    }
    //new value field is useful just for alert detection methods -> not relevant in metaProfile
    meta_it->second["metaData"][NEW_VALUE] = to_string(new_value);
    return new_value;
}

void Analyzer::modifyMetaData(string &ur_field, uint64_t *ur_id ,map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id){
    int flag = 0; //decision flag

    //determine profile values
    for (auto profile_values:  meta_it->second["profile"]){
        if ( profile_values == "median" ){
            //median method
            meta_it->second[meta_id][MEDIAN] = to_string(getMedian(sensor_it));
        }
        else if (profile_values == "average" || profile_values == "variance") {
            //moving varinace and average method
            //skip unnecessary calls
            if (flag == 1 ){
                continue;
            }
            pair<double, double> determine_values = getAverageAndVariance(ur_field, ur_id, meta_it, sensor_it,meta_id);
            meta_it->second[meta_id][AVERAGE] = to_string(determine_values.first);
            meta_it->second[meta_id][VARIANCE] = to_string(determine_values.second);
            flag = 1;
        }
        else if (profile_values == "cum_average"){
            //cum moving average method
            meta_it->second[meta_id][CUM_AVERAGE] = to_string(getCumulativeAverage(sensor_it, meta_it,meta_id));
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
        int rotate_cnt = stoi (meta_it->second["metaProfile"][ROTATE],nullptr);

        //test if sensor id exists
        sensor_it = control[ur_field].find(*ur_id);
        if ( sensor_it != control[ur_field].end() ){

            //learning profile phase
            if ( learning_length > sensor_it->second.size() + rotate_cnt){
                //cout << "series created -> learning phase" << endl;
                if (series_length > sensor_it->second.size()){ 
                    cout << "learning phase" << endl; 
                    //push data 
                    pushData(ur_data, meta_it, sensor_it, "metaProfile");
                    //sensor_it->second.push_back(*ur_data);
                    //modify profile values
                    modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaProfile");
                    return 4;
                }
                //rotate values in learning phase
                else {
                    cout << "rotate values" <<endl;
                    rotate( sensor_it->second.begin(), sensor_it->second.begin()+1, sensor_it->second.end());
                    //push data 
                    pushData(ur_data, meta_it, sensor_it, "metaProfile");
                    //sensor_it->second.back() = *ur_data;
                    rotate_cnt++;
                    meta_it->second["metaProfile"][ROTATE] = to_string(rotate_cnt);
                    //modify profile values
                    modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaProfile");
                    return 3;
                }
            }
            else {
                //learning phase has been finished
                //check if init phase is finished for the first time and copy init values
                if ( meta_it->second["metaData"][AVERAGE] == "x"){
                    cout << "learning phase finished -> start analyzing" << endl;

                    meta_it->second["metaData"][AVERAGE] = meta_it->second["metaProfile"][AVERAGE];
                    meta_it->second["metaData"][VARIANCE] = meta_it->second["metaProfile"][VARIANCE];
                    meta_it->second["metaData"][MEDIAN] = meta_it->second["metaProfile"][MEDIAN];
                    meta_it->second["metaData"][CUM_AVERAGE] = meta_it->second["metaProfile"][CUM_AVERAGE];
                    meta_it->second["metaData"][SX] = meta_it->second["metaProfile"][SX];
                    meta_it->second["metaData"][SX2] = meta_it->second["metaProfile"][SX2];
                    meta_it->second["metaData"][PREV_VALUE] = meta_it->second["metaProfile"][PREV_VALUE];
                }
                return 0;
            }
        }
        //sensor id not found - create new record 
        else {
            vector<double> tmp;
            string store_mode = meta_it->second["general"][STORE_MODE];
            //init variables by zeros in case of delta store mode
            if (store_mode == "delta"){
                tmp.push_back(0);
                meta_it->second["metaProfile"][AVERAGE] = to_string(0);
            }
            //init variables by first received value
            else{
                tmp.push_back(*ur_data);
                meta_it->second["metaProfile"][AVERAGE] = to_string(*ur_data);
            }
            control[ur_field].insert(pair<int, vector<double> >(*ur_id,tmp));
            meta_it->second["metaProfile"][PREV_VALUE] = to_string(*ur_data);
            sensor_it = control[ur_field].find(*ur_id);
            //modify profile values
            modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaProfile");
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
        cout << "   AVERAGE, VARIANCE, MEDIAN, CUM_AVERAGE, SX, SX2, PREV_VALUE, NEW_VALUE, ROTATE" << endl;
        cout << "   ";
        for (auto meta: series_meta_data[ur_field]["metaProfile"] ){
            cout.precision(1);
            cout << fixed << meta << ", ";
        }
        cout << endl;
        cout << "   ";
        for (auto meta: series_meta_data[ur_field]["metaData"] ){
            cout.precision(1);
            cout << fixed << meta << ", ";
        }
        cout << endl;
    }
}

int Analyzer::getIndex(string name){
    if (name == "median"){
        return MEDIAN;
    }
    else if (name == "average"){
        return AVERAGE;
    }
    else if (name == "variance"){
        return VARIANCE;
    }
    else if (name == "cum_average"){
        return CUM_AVERAGE;
    }
    else if (name == "new_value"){
        return NEW_VALUE;
    }
}

void Analyzer::addAlert(string & profile_name, string alert_message, map<string, vector<string> > & alert_str){
    //find key
    auto alert_it = alert_str.find(profile_name);
    if (alert_it != alert_str.end()){
        alert_it->second.push_back(alert_message);
    }
    else {
        vector<string> tmp;
        tmp.push_back(alert_message);
        alert_str.insert(pair<string, vector<string> >(profile_name,tmp));
    }
}

void Analyzer::dataLimitCheck(map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > &alert_str){

    for (auto profile_values: meta_it->second["profile"]){
        //test if soft limits are set
        //soft min, max limits are dependent -> test for soft min is ok
        if (meta_it->second[profile_values][SOFT_MIN] != "-"){
        
            //soft limit test 
            //soft limit min
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr)  < stod(meta_it->second[profile_values][SOFT_MIN],nullptr) ){
                //add counter or create alert message if the counter is above soft period 
                if ( stoi (meta_it->second[profile_values][S_MIN_LIMIT],nullptr) > stod(meta_it->second[profile_values][SOFT_PERIOD]) ){
                    cout << "ALERT: Lower soft limit" << endl;
                    addAlert(profile_values, "Lower soft limit", alert_str);
                }
                else{
                    meta_it->second[profile_values][S_MIN_LIMIT] = to_string(stoi (meta_it->second[profile_values][S_MIN_LIMIT],nullptr) + 1);
                }
            }
            else {
                //reset counter
                meta_it->second[profile_values][S_MIN_LIMIT] = "0";
            }

            //soft limit max
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr)  > stod(meta_it->second[profile_values][SOFT_MAX],nullptr) ){
                //add counter
                if ( stoi (meta_it->second[profile_values][S_MAX_LIMIT],nullptr) > stod(meta_it->second[profile_values][SOFT_PERIOD]) ){
                    cout <<  "ALERT: Higher soft limit" << endl;
                    addAlert(profile_values, "Higher soft limit", alert_str);
                }
                else{
                    meta_it->second[profile_values][S_MAX_LIMIT] = to_string(stoi (meta_it->second[profile_values][S_MAX_LIMIT],nullptr) + 1);
                }
            }
            else {
                //reset counter
                meta_it->second[profile_values][S_MAX_LIMIT] = "0";
            }
        }
        //test if hard limits are set
        //hard min, max limits are dependent -> test for hard min is ok
        else if (meta_it->second[profile_values][HARD_MIN] != "-"){
            //hard limit test
            //hard limit min
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr)  < stod(meta_it->second[profile_values][HARD_MIN],nullptr) ){
                cout << "ALERT: Lower hard limit" << endl;;
                addAlert(profile_values, "Lower hard limit", alert_str);
            }
            //hard limit max
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr)  > stod(meta_it->second[profile_values][HARD_MAX],nullptr) ){
                cout  << "ALERT: Higher hard limit" << endl;
                addAlert(profile_values, "Higher hard limit", alert_str);
            }
        }
    }
}

void Analyzer::dataChangeCheck(map<int,vector<double> >::iterator &sensor_it ,map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > &alert_str){
    double alert_coef = 0;
    double new_value = 0;
    
    for (auto profile_values: meta_it->second["profile"]){
        new_value = sensor_it->second.back();
        //test if grow limits are set
        //grow up,down limits are dependent -> test for grow up is ok
        if (meta_it->second[profile_values][GROW_UP] != "-"){
            //divide by zero protection
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr) == 0 ){
                alert_coef = 1; //-> mean no data grow change 
            }
            else {
                alert_coef = new_value/stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr);
            }
            //test grow limits
            if (alert_coef > stod(meta_it->second[profile_values][GROW_UP],nullptr)){
                cout << "ALERT: GROW UP" << endl;
                addAlert(profile_values, "Higher grow limit", alert_str);
                
            }
            
            if (alert_coef < stod(meta_it->second[profile_values][GROW_DOWN],nullptr)){
                cout << "ALERT: GROW DOWN" << endl;
                addAlert(profile_values, "Lower grow limit", alert_str);
            }
        }
        
    }
}

map<string,vector<string> > Analyzer::analyzeData(string ur_field, uint64_t *ur_id, double *ur_data, double *ur_time) {
    map<string, map<string, vector<string> > >::iterator meta_it;
    map<int, vector<double> >::iterator sensor_it;
    map<string, vector<string> > alert_str;
    
    //find proper iterators
    meta_it = series_meta_data.find(ur_field);
    sensor_it = control[ur_field].find(*ur_id);
    //*check conditions*//
    //soft,hard limit check
    dataLimitCheck(meta_it, ur_field, ur_id, ur_time ,ur_data, alert_str);
    //grow check
    dataChangeCheck(sensor_it, meta_it, ur_field, ur_id, ur_time, ur_data, alert_str);

    //push new data and do calculation
    pushData(ur_data, meta_it, sensor_it, "metaData");
    modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaData");

    //do action - return result 
    return alert_str;

}

void Analyzer::sentAlert(map<string, vector<string> > &alert_str, trap_ctx_t *ctx){

}

//data series processing
void Analyzer::processSeries(string ur_field, uint64_t *ur_id, double *ur_time, double *ur_data, trap_ctx_t *alert, trap_ctx_t *d_export) {
    int init_state = 0;
    //initialize data series
    /*return 
    * 0 - init done
    * 1 - ur_field is not specified in the template
    * 2 - new value was created
    * 3 - values were rotated
    * 4 - new item was added
    */
    init_state = initSeries(ur_field, ur_id, ur_data);
    if (init_state != 1){
        printSeries(ur_field);
    }
    
    if (init_state == 0){ 
        //analyze data series
        analyzeData(ur_field, ur_id, ur_data, ur_time);
        
    }

    /*
    periodicExport(ctx2);
    periodicCheck(ctx);
    sentAlert(ctx);
    */
}

/* 
 * END TIME SERIES PROCESS
 */



