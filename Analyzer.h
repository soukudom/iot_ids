#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include "fields.h"
#include <thread>
#include <unistd.h>
#include <ctime>


using namespace std;


class Analyzer{
public:
    Analyzer();
    Analyzer(map<string, map<string, vector<string> > > meta_data);
    virtual ~Analyzer();
    void processSeries(string ur_field, uint64_t *ur_id, double *ur_time, double *ur_data, trap_ctx_t *alert_ifc, trap_ctx_t *data_export_ifc, ur_template_t *alert_template, void *data_alert);

private:
    /**atributes**/
    trap_ctx_t *alert_ifc;
    ur_template_t *alert_template;
    void *data_alert;

    trap_ctx_t *data_export_ifc;
    ur_template_t **export_template;
    void **data_export;
    map<int,string> ur_export_fields;

    map<string, map<string, vector<string> > > series_meta_data;
    map<string, map<int, vector<double> > > control;

    //buffers for moving variance
    map<string, map<int, vector<double> > > x;
    map<string, map<int, vector<double> > > x2;

    /**internal methods**/
    //initialize data series structures
    int initSeries(string &ur_field, uint64_t *ur_id, double *ur_data, double *ur_time);
    //analyze data series data
    map<string,vector<string> > analyzeData(string ur_field, uint64_t *ur_id, double *ur_data, double *ur_time);
    //set reference profile values
    void modifyMetaData(string &ur_field, uint64_t *ur_id, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id);
    //push data to the control structure
    double pushData(double *ur_time, double *ur_data, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id);
    //get field index for profile name
    int getIndex(string name);
    //add alert to the alert structure
    void addAlert(string &profile_name, string alert_message, map<string, vector<string> > &alert);
    void sentAlert(map<string, vector<string> > &alert_str, trap_ctx_t *ctx, string &ur_field, uint64_t *ur_id, double *ur_time, ur_template_t *alert_template, void *data_alert);


    /**profile determination methods**/
    //get median
    double getMedian(map<int,vector<double> >::iterator &sensor_it);
    //get moving average and variance
    pair<double, double> getAverageAndVariance(string &ur_field, uint64_t *ur_id, map<string,map<string, vector<string> > >::iterator &meta_it, map<int,vector<double> >::iterator &sensor_it, string meta_id);
    //get cumulatie moving average
    double getCumulativeAverage(map<int,vector<double> >::iterator &sensor_it, map<string, map<string, vector<string> > >::iterator &meta_it, string meta_id);
    //print series data and meta information
    void printSeries(string &ur_field);

    /**alert detection methods**/
    void dataLimitCheck(map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > & alert_str);
    void dataChangeCheck(map<int,vector<double> >::iterator &sensor_it, map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > & alert_str);


    /** thead method **/
    void periodicCheck(int period, string ur_field);
    void periodicExport(int period, string ur_field);
    void runThreads(string &ur_field);
    
};
