#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include "fields.h"


using namespace std;


class Analyzer{
public:
    Analyzer();
    Analyzer(map<string, map<string, vector<string> > > meta_data);
    virtual ~Analyzer();
    void processSeries(string ur_field, uint64_t *ur_id, double *ur_time, double *ur_data, trap_ctx_t *alert, trap_ctx_t *d_export);

private:
    /**atributes**/
    map<string, map<string, vector<string> > > series_meta_data;
    map<string, map<int, vector<double> > > control;

    //buffers for moving variance
    map<string, map<int, vector<double> > > x;
    map<string, map<int, vector<double> > > x2;

    /**internal methods**/
    //initialize data series structures
    int initSeries(string &ur_field, uint64_t *ur_id, double *ur_data);
    //analyze data series data
    void analyzeData(string ur_field, uint64_t *ur_id, double *ur_data, double *ur_time);
    //set reference profile values
    void modifyMetaData(string &ur_field, uint64_t *ur_id, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id);
    //push data to the control structure
    double pushData(double *ur_data, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id);
    //get median
    double getMedian(map<int,vector<double> >::iterator &sensor_it);
    //get moving average and variance
    pair<double, double> getAverageAndVariance(string &ur_field, uint64_t *ur_id, map<string,map<string, vector<string> > >::iterator &meta_it, map<int,vector<double> >::iterator &sensor_it, string meta_id);
    //get cumulatie moving average
    double getCumulativeAverage(map<int,vector<double> >::iterator &sensor_it, map<string, map<string, vector<string> > >::iterator &meta_it, string meta_id);
    //print series data and meta information
    void printSeries(string &ur_field);
    /**alert detection methods**/
    void dataLimitCheck(map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data);
    void dataChangeCheck(map<int,vector<double> >::iterator &sensor_it, map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data);
    int getIndex(string name);
    
};
