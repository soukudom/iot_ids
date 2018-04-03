/**
 * \file Analyzer.cpp
 * \brief Analyze time series data based on configuration
 * \author Dominik Soukup <soukudom@fit.cvut.cz>
 * \date 2018
**/

#include "Analyzer.h"
#include "ConfigParser.h"

// Default constructor
Analyzer::Analyzer() = default;

// Param constructor
Analyzer::Analyzer(map<string, map<string, vector<string> > > meta_data,int verbose): series_meta_data(meta_data), verbose(verbose) {}

// Default destructor
Analyzer::~Analyzer() = default;

// Setters for trap interfaces
void Analyzer::setAlertInterface(trap_ctx_t *alert_ifc, ur_template_t *alert_template, void *data_alert){
    this->alert_ifc = alert_ifc;
    this->alert_template = alert_template;
    this->data_alert = data_alert;
}

void Analyzer::setExportInterface(trap_ctx_t *export_ifc, ur_template_t **export_template, void **data_export, map<int, vector<string> > ur_export_fields){
    
    this->export_ifc = export_ifc;
    this->export_template = export_template;
    this->data_export = data_export;
    this->ur_export_fields = ur_export_fields;
}

/* 
 * BEGIN CALCALULATION METHODS
 */
double Analyzer::getMedian(map<int,vector<double> >::iterator &sensor_it, map<string,map<string, vector<string> > >::iterator &meta_it, string &ur_field){
    // Basic non-moving median calculation

    // First value exception
    if (sensor_it->second.size() == 1){
        return sensor_it->second.back();
    }
    int series_length = stoi (meta_it->second["general"][SERIES_LENGTH],nullptr);
    double new_value = stod (meta_it->second["metaData"][NEW_VALUE],nullptr);
    double old_value = stod (meta_it->second["metaData"][OLDEST_VALUE],nullptr);
    double median = 0;
    int index = 0;

    // Insert the new value
    if (series_length > median_window[ur_field][sensor_it->first].size()){
        // Push back
        median_window[ur_field][sensor_it->first].push_back(new_value);
    } else {
        // Vector is full -> replace the oldest value
        index = 0;
        for (auto elem: median_window[ur_field][sensor_it->first]){
            if (elem == old_value){
                median_window[ur_field][sensor_it->first][index] = new_value;
                break;
            }
            index++;
        }        
    }

    // Calculate median value
    size_t size = median_window[ur_field][sensor_it->first].size();

    // Sort modified vector by the new value
    sort(median_window[ur_field][sensor_it->first].begin(), median_window[ur_field][sensor_it->first].end());

    if (size  % 2 == 0) {
        median = (median_window[ur_field][sensor_it->first][size / 2 - 1] + median_window[ur_field][sensor_it->first][size / 2]) / 2;
    }
    else {
        median = median_window[ur_field][sensor_it->first][size / 2]; 
    }
    return median;
}

double Analyzer::getCumulativeAverage(map<int,vector<double> >::iterator &sensor_it, map<string,map<string, vector<string> > >::iterator &meta_it, string meta_id){
    // Alg source: https://en.wikipedia.org/wiki/Moving_average
    double new_value = sensor_it->second.back();
    // First value exception
    if (sensor_it->second.size() == 1){
        return new_value;
    }
    double delta_average = stod (meta_it->second[meta_id][CUM_AVERAGE],nullptr); 
    delta_average = ( new_value + ( sensor_it->second.size() ) * delta_average ) / ( sensor_it->second.size() + 1 );
    return delta_average;
}

pair<double, double> Analyzer::getAverageAndVariance(string &ur_field, uint64_t *ur_id, map<string,map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id){
    // Alg source: https://www.dsprelated.com/showthread/comp.dsp/97276-1.php
    map<int, vector<double> >::iterator x_it;
    map<int, vector<double> >::iterator x2_it;
    
    int series_length = stoi (meta_it->second["general"][SERIES_LENGTH],nullptr);
    double new_value = sensor_it->second.back();
    double variance = 0;
    double average = 0;
    double sx = 0;
    double sx2 = 0;

    // Initialize time window
    if (x[ur_field][*ur_id].size() < series_length){
        //add new data to time window
        x[ur_field][*ur_id].push_back(new_value);
        x2[ur_field][*ur_id].push_back(new_value*new_value);

        // Count sum of values in window
        meta_it->second[meta_id][SX] = to_string(new_value + stod(meta_it->second[meta_id][SX],nullptr));
        meta_it->second[meta_id][SX2] = to_string(new_value*new_value+ stod(meta_it->second[meta_id][SX2],nullptr));
        
        return pair<double, double> (-1,-1);
    // Count moving average and variance
    } else {
        // Change values in time window
        rotate(x[ur_field][*ur_id].begin(), x[ur_field][*ur_id].begin()+1, x[ur_field][*ur_id].end());
        rotate(x2[ur_field][*ur_id].begin(), x2[ur_field][*ur_id].begin()+1, x2[ur_field][*ur_id].end());

        // Do a calculation
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

// Store data in data series based on configured values
pair<double, double> Analyzer::pushData(double *ur_time, double *ur_data, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id){
    double new_value = 0;
    double old_value = 0;
    string store_mode = meta_it->second["general"][STORE_MODE];
    int series_length = stoi (meta_it->second["general"][SERIES_LENGTH],nullptr);

    // Save data based on store mode
    if (store_mode == "simple"){
        new_value = *ur_data;
    } else if (store_mode == "average"){
        double average = stod (meta_it->second[meta_id][AVERAGE],nullptr);
        new_value = *ur_data - average;
    } else if (store_mode == "delta"){
        double prev_value = stod (meta_it->second[meta_id][PREV_VALUE],nullptr);
        new_value = *ur_data - prev_value;
        meta_it->second[meta_id][PREV_VALUE] = to_string(*ur_data);
    } else {
        cerr << "ERROR: Unknown mode" << endl;
    }

    // Insert new value
    if (series_length > sensor_it->second.size()){
        // Push back
        sensor_it->second.push_back(new_value);        
        old_value = sensor_it->second[0];
    } else {
        //back() -> vector is full
        old_value = sensor_it->second.back();
        sensor_it->second.back() = new_value;
    }
    // New value field is useful just for alert detection methods -> not relevant in metaProfile
    meta_it->second["metaData"][NEW_VALUE] = to_string(new_value);
    meta_it->second["metaData"][LAST_TIME] = to_string(*ur_time);
    meta_it->second["metaData"][OLDEST_VALUE] = to_string(old_value);
    if (verbose >= 0){
        cout << "VERBOSE: New data pushed: " << new_value << endl;
    }
    return pair<double, double>(new_value, old_value);
}

void Analyzer::modifyMetaData(string &ur_field, uint64_t *ur_id ,map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator sensor_it, string meta_id){
    int flag = 0; // Decision flag

    // Determine profile values
    for (auto profile_values:  meta_it->second["profile"]){
        if ( profile_values == "median" ){
            // Median method
            meta_it->second[meta_id][MEDIAN] = to_string(getMedian(sensor_it,meta_it,ur_field));
        } else if (profile_values == "average" || profile_values == "variance") {
            // Moving varinace and average method
            // Skip unnecessary calls
            if (flag == 1 ){
                continue;
            }
            pair<double, double> determine_values = getAverageAndVariance(ur_field, ur_id, meta_it, sensor_it,meta_id);
            meta_it->second[meta_id][AVERAGE] = to_string(determine_values.first);
            meta_it->second[meta_id][VARIANCE] = to_string(determine_values.second);
            flag = 1;
        } else if (profile_values == "cum_average"){
            // Cum moving average method
            meta_it->second[meta_id][CUM_AVERAGE] = to_string(getCumulativeAverage(sensor_it, meta_it,meta_id));
        }
    }
}

int Analyzer::initSeries(string &ur_field, uint64_t *ur_id, double *ur_data, double *ur_time){
    map<string, map<string, vector<string> > >::iterator meta_it;
    map<int, vector<double> >::iterator sensor_it;

    // Test if meta information exists
    meta_it = series_meta_data.find(ur_field);
    if (meta_it != series_meta_data.end()){

        // Cast necessary meta info values
        int learning_length = stoi (meta_it->second["general"][LEARNING_LENGTH],nullptr);
        int series_length = stoi (meta_it->second["general"][SERIES_LENGTH],nullptr);
        int rotate_cnt = stoi (meta_it->second["metaProfile"][ROTATE],nullptr);
        int ignore_cnt = stoi (meta_it->second["general"][IGNORE_LENGTH],nullptr);
        
        if (ignore_cnt > 0){
            meta_it->second["general"][IGNORE_LENGTH] = to_string(--ignore_cnt);
            if (verbose >= 0){
                cout << "VERBOSE: Ignoring phase" << endl;
            }
            return 5;
        }
        // Test if sensor id exists
        sensor_it = control[ur_field].find(*ur_id);
        if ( sensor_it != control[ur_field].end() ){

            // Learning profile phase
            if ( learning_length > sensor_it->second.size() + rotate_cnt){
                if (verbose >= 0){
                    cout << "VERBOSE: Series created -> learning phase" << endl;
                }
                if (series_length > sensor_it->second.size()){ 
                    if (verbose >= 0){
                        cout << "VERBOSE: Learning phase" << endl; 
                    }
                    // Push data 
                    pushData(ur_time, ur_data, meta_it, sensor_it, "metaProfile");
                    // Modify profile values
                    modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaProfile");
                    return 4;
                // Rotate values in learning phase
                } else {
                    if (verbose >= 0){
                        cout << "VERBOSE: Rotate values" <<endl;
                    }
                    rotate( sensor_it->second.begin(), sensor_it->second.begin()+1, sensor_it->second.end());
                    // Push data 
                    pushData(ur_time, ur_data, meta_it, sensor_it, "metaProfile");
                    rotate_cnt++;
                    meta_it->second["metaProfile"][ROTATE] = to_string(rotate_cnt);
                    // Modify profile values
                    modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaProfile");
                    return 3;
                }
            // Learning phase has been finished
            } else {
                // Check if init phase is finished for the first time and copy init values
                if ( meta_it->second["metaData"][AVERAGE] == "x"){
                    if (verbose >= 0){
                        cout << "VERBOSE: learning phase finished -> start analyzing" << endl;
                    }

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
        // Sensor id not found - create a new record 
        } else {
            vector<double> tmp;
            string store_mode = meta_it->second["general"][STORE_MODE];
            // Init variables by zeros in case of delta store mode
            if (store_mode == "delta"){
                tmp.push_back(0);
                meta_it->second["metaProfile"][AVERAGE] = to_string(0);
            // Init variables by first received value
            } else{
                tmp.push_back(*ur_data);
                meta_it->second["metaProfile"][AVERAGE] = to_string(*ur_data);
            }
            control[ur_field].insert(pair<int, vector<double> >(*ur_id,tmp));
            meta_it->second["metaProfile"][PREV_VALUE] = to_string(*ur_data);
            sensor_it = control[ur_field].find(*ur_id);
            // Modify profile values
            modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaProfile");
            meta_it->second["metaProfile"][ID] = to_string(*ur_id);
            return 2;
        }
    } else {
        if (verbose >= 1){
            cout << "VERBOSE: Field " << ur_field <<  " meta specification missig. Skipping... " << endl;
        }
        return 1;
    }
}

/*
 * END TIME SERIES SERVICE AND INIT METHODS
 */

/* 
 * BEGIN TIME SERIES PROCESS
 */

// Print series data
void Analyzer::printSeries(string &ur_field){
    cout << "field: " << ur_field << endl;
    for (auto element : control[ur_field]){
        cout << " ID value: " << element.first << endl; 
        cout << "  time series values: " << endl;
        cout << "   ";
        for (auto elem: element.second){
            cout << elem << ", ";
        }
        cout << endl;
    
        cout << "   AVERAGE, VARIANCE, MEDIAN, CUM_AVERAGE, SX, SX2, PREV_VALUE, NEW_VALUE, LAST_TIME, ROTATE, CHECKED_FLAG, ID" << endl;
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
    } else if (name == "average"){
        return AVERAGE;
    } else if (name == "variance"){
        return VARIANCE;
    } else if (name == "cum_average"){
        return CUM_AVERAGE;
    } else if (name == "new_value"){
        return NEW_VALUE;
    }
}

void Analyzer::addAlert(string & profile_name, string alert_message, map<string, vector<string> > & alert_str){
    // Find key
    auto alert_it = alert_str.find(profile_name);
    if (alert_it != alert_str.end()){
        alert_it->second.push_back(alert_message);
    } else {
        vector<string> tmp;
        tmp.push_back(alert_message);
        alert_str.insert(pair<string, vector<string> >(profile_name,tmp));
    }
}

void Analyzer::dataLimitCheck(map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > &alert_str){

    for (auto profile_values: meta_it->second["profile"]){
        // Test if soft limits are set
        // Soft min, max limits are dependent -> test for soft min is ok
        if (meta_it->second[profile_values][SOFT_MIN] != "-"){
        
            // Soft limit test 
            // Soft limit min
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr)  < stod(meta_it->second[profile_values][SOFT_MIN],nullptr) ){
                // Add counter or create alert message if the counter is above soft period 
                if ( stoi (meta_it->second[profile_values][S_MIN_LIMIT],nullptr) > stod(meta_it->second[profile_values][SOFT_PERIOD]) ){
                    if (verbose >= 0){
                        cout << "VERBOSE: ALERT: Lower soft limit" << endl;
                    }
                    addAlert(profile_values, "Lower soft limit", alert_str);
                } else{
                    meta_it->second[profile_values][S_MIN_LIMIT] = to_string(stoi (meta_it->second[profile_values][S_MIN_LIMIT],nullptr) + 1);
                }
            } else {
                // Reset counter
                meta_it->second[profile_values][S_MIN_LIMIT] = "0";
            }

            // Soft limit max
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr)  > stod(meta_it->second[profile_values][SOFT_MAX],nullptr) ){
                // Add counter
                if ( stoi (meta_it->second[profile_values][S_MAX_LIMIT],nullptr) > stod(meta_it->second[profile_values][SOFT_PERIOD]) ){
                    if (verbose >= 0){
                        cout <<  "VERBOSE: ALERT: Higher soft limit" << endl;
                    }
                    addAlert(profile_values, "Higher soft limit", alert_str);
                } else{
                    meta_it->second[profile_values][S_MAX_LIMIT] = to_string(stoi (meta_it->second[profile_values][S_MAX_LIMIT],nullptr) + 1);
                }
            } else {
                // Reset counter
                meta_it->second[profile_values][S_MAX_LIMIT] = "0";
            }
        // Test if hard limits are set
        // Hard min, max limits are dependent -> test for hard min is ok
        } else if (meta_it->second[profile_values][HARD_MIN] != "-"){
            // Hard limit test
            // Hard limit min
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr)  < stod(meta_it->second[profile_values][HARD_MIN],nullptr) ){
                if (verbose >= 0){
                    cout << "VERBOSE: ALERT: Lower hard limit" << endl;
                }
                addAlert(profile_values, "Lower hard limit", alert_str);
            }
            //hard limit max
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr)  > stod(meta_it->second[profile_values][HARD_MAX],nullptr) ){
                if (verbose >= 0){
                    cout  << "VERBOSE: ALERT: Higher hard limit" << endl;
                }
                addAlert(profile_values, "Higher hard limit", alert_str);
            }
        }
    }
}

void Analyzer::dataChangeCheck(map<int,vector<double> >::iterator &sensor_it ,map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > &alert_str){
    double alert_coef = 0;
    double new_value = 0;
    double profile_value = 0;
    
    for (auto profile_values: meta_it->second["profile"]){
        //new_value = sensor_it->second.back();
        new_value = stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr);
        profile_value = stod(meta_it->second["metaProfile"][getIndex(profile_values)],nullptr);
        // Protection against zero profile values
        if (profile_value == 0){
            profile_value = 1;
        } 
    
        // Test if grow limits are set
        // Grow up,down limits are dependent -> test for grow up is ok
        if (meta_it->second[profile_values][GROW_UP] != "-"){
            // Divide by zero protection
            if (stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr) == 0 ){
                alert_coef = 1; //-> mean no data grow change 
            } else {
               // alert_coef = new_value/stod(meta_it->second["metaData"][getIndex(profile_values)],nullptr);
                alert_coef = new_value / profile_value;
            }
            // Test grow limits
            if (alert_coef > stod(meta_it->second[profile_values][GROW_UP],nullptr)){
                if (verbose >= 0){
                    cout << "VERBOSE: ALERT: GROW UP" << endl;
                }
                addAlert(profile_values, "Higher grow limit", alert_str);
                
            }
            if (alert_coef < stod(meta_it->second[profile_values][GROW_DOWN],nullptr)){
                if (verbose >= 0){
                    cout << "VERBOSE: ALERT: GROW DOWN" << endl;
                }
                addAlert(profile_values, "Lower grow limit", alert_str);
            }
        }
    }
}

map<string,vector<string> > Analyzer::analyzeData(string ur_field, uint64_t *ur_id, double *ur_data, double *ur_time) {
    map<string, map<string, vector<string> > >::iterator meta_it;
    map<int, vector<double> >::iterator sensor_it;
    map<string, vector<string> > alert_str;

    // Find proper iterators
    meta_it = series_meta_data.find(ur_field);
    sensor_it = control[ur_field].find(*ur_id);
    /*
    * Check conditions
    */
    // Soft, hard limit check
    dataLimitCheck(meta_it, ur_field, ur_id, ur_time ,ur_data, alert_str);
    // Grow check
    dataChangeCheck(sensor_it, meta_it, ur_field, ur_id, ur_time, ur_data, alert_str);
    sensor_it = control[ur_field].find(*ur_id);

    // Push new data and do calculation
    rotate( sensor_it->second.begin(), sensor_it->second.begin()+1, sensor_it->second.end());
     //   for (auto elem: sensor_it->second){
       ///     cout << elem << ", ";
        //}
    pushData(ur_time, ur_data, meta_it, sensor_it, "metaData");
    modifyMetaData(ur_field,ur_id,meta_it, sensor_it, "metaData");

    // Do action - return result 
    return alert_str;

}

void Analyzer::sendAlert(map<string, vector<string> > &alert_str, string &ur_field, uint64_t *ur_id, double *ur_time){
    if (alert_str.empty()){
        if (verbose >= 1){
            cout << "VERBOSE: No alert detected" << endl;
        }
        return;
    }
    
    map<string, map<string, vector<string> > >::iterator meta_it;
    meta_it = series_meta_data.find(ur_field);
    double err_value = 0;
    double profile_value = 0;
     
    for (auto profile: alert_str){
        for (auto elem: profile.second){
            if (verbose >= 1){
                cout << "VERBOSE: Send alert: " << profile.first << ", " << elem << endl;
            }

            err_value = stod(meta_it->second["metaData"][getIndex(profile.first)],nullptr);
            profile_value = stod(meta_it->second["metaProfile"][getIndex(profile.first)],nullptr);

            // Clear variable-length fields
            ur_clear_varlen(alert_template, data_alert);
            // Set UniRec message values
            ur_set(alert_template, data_alert, F_ID, *ur_id);
            ur_set(alert_template, data_alert, F_TIME, *ur_time);
            ur_set(alert_template, data_alert, F_err_value, err_value);
            ur_set(alert_template, data_alert, F_profile_value, profile_value);
            ur_set_string(alert_template, data_alert, F_profile_key, profile.first.c_str());
            ur_set_string(alert_template, data_alert, F_alert_desc, elem.c_str());
            ur_set_string(alert_template, data_alert, F_ur_key, ur_field.c_str());
            trap_ctx_send(alert_ifc, 0, data_alert, ur_rec_size(alert_template, data_alert) );
            //trap_ctx_send_flush(alert_ifc,0);
        }
    }
}

void Analyzer::periodicCheck(int period,  string ur_field){

    map<string, map<string, vector<string> > >::iterator meta_it;
    meta_it = series_meta_data.find(ur_field);

    while(true){
        sleep(period);
        int result = std::time(nullptr);
        if (result - stoi(meta_it->second["metaData"][LAST_TIME]) > period ){
            map<string,vector<string> > alert_str;
            // Alert data hasn't been received
            if (verbose >= 0){
                cout << "VERBOSE: ALERT: data period" << endl;
            }
            addAlert(ur_field, "Periodic check", alert_str);
            uint64_t ur_id = stoi(meta_it->second["metaProfile"][ID],nullptr);
            double ur_time = stod(meta_it->second["metaData"][LAST_TIME],nullptr);
            sendAlert(alert_str, ur_field, &ur_id, &ur_time);
        }
    }
}

void Analyzer::periodicExport(int period, string ur_field){

    map<string, map<string, vector<string> > >::iterator meta_it;
    meta_it = series_meta_data.find(ur_field);
    if (verbose >= 0){
        cout << "VERBOSE: Periodic export " << ur_field << endl;
    }
    while (true){
        sleep (period);
        for (auto elem: ur_export_fields){
            for (auto field: elem.second){
                // Set unirec record 
                if (field == "average"){
                    //!!!REMOVE
                    cout << "ADD AVERAGE " << stod(meta_it->second["metaData"][AVERAGE],nullptr) << endl;
                    ur_set(export_template[elem.first], data_export[elem.first], F_average, stod(meta_it->second["metaData"][AVERAGE],nullptr));
                } else if (field == "variance") {
                    //!!!REMOVE
                    cout << "ADD VARIANCE: " << stod(meta_it->second["metaData"][VARIANCE],nullptr) <<  endl;
                    ur_set(export_template[elem.first], data_export[elem.first], F_variance, stod(meta_it->second["metaData"][VARIANCE],nullptr));

                } else if (field == "median"){
                    //!!!REMOVE
                    cout << "ADD MEDIAN" << endl;
                    ur_set(export_template[elem.first], data_export[elem.first], F_median, stod(meta_it->second["metaData"][MEDIAN],nullptr));

                } else if (field == "cum_average"){
                    //!!!REMOVE
                    cout << "ADD CUM_AVERAGE " << stod(meta_it->second["metaData"][CUM_AVERAGE],nullptr) << endl;
                    ur_set(export_template[elem.first], data_export[elem.first], F_cum_average, stod(meta_it->second["metaData"][CUM_AVERAGE],nullptr));

                }
            }
            
            // Send data for periodic export
            trap_ctx_send(export_ifc, elem.first, data_export[elem.first], ur_rec_size(export_template[elem.first], data_export[elem.first]));
        }
    }
}

void Analyzer::runThreads(string &ur_field){
    map<string, map<string, vector<string> > >::iterator meta_it;
    meta_it = series_meta_data.find(ur_field);
    // Run periodic check if it is set
    if (meta_it->second["general"][PERIODIC_CHECK] != "-" && meta_it->second["metaData"][CHECKED_FLAG] == "x"){
        thread t1(&Analyzer::periodicCheck,this,stoi(meta_it->second["general"][PERIODIC_CHECK],nullptr),ur_field);
        t1.detach();
        meta_it->second["metaData"][CHECKED_FLAG] = "p";
    }

    if (meta_it->second["general"][EXPORT_INTERVAL] != "-" && ( meta_it->second["metaData"][CHECKED_FLAG] == "x" || meta_it->second["metaData"][CHECKED_FLAG] == "p" )){
        thread t2(&Analyzer::periodicExport,this,stoi(meta_it->second["general"][PERIODIC_CHECK],nullptr),ur_field);
        t2.detach();
        meta_it->second["metaData"][CHECKED_FLAG] = "e";

    }
}

// Data series processing
void Analyzer::processSeries(string ur_field, uint64_t *ur_id, double *ur_time, double *ur_data) {
    int init_state = 0;
    // Initialize data series
    /*
    * return values: 
    *  0 - init done
    *  1 - ur_field is not specified in the template
    *  2 - new record has been created
    *  3 - values were rotated
    *  4 - new item was added - learning phase
    *  5 - ignore phase
    */
    init_state = initSeries(ur_field, ur_id, ur_data, ur_time);
    
    if (init_state == 0){ 
        // Analyze data series
        if (verbose >= 0){
            cout << "VERBOSE: Analyzing phase" << endl;
        }
        auto alert_str = analyzeData(ur_field, ur_id, ur_data, ur_time);
        sendAlert(alert_str, ur_field, ur_id, ur_time);
        runThreads(ur_field);
    }

    if (init_state != 1 && verbose >= 0){
        printSeries(ur_field);
    }
}
/* 
 * END TIME SERIES PROCESS
 */



