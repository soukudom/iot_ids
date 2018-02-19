#include <map>
#include <vector>
#include <algorithm>
#include <iostream>


using namespace std;


class Analyzer{
public:
    Analyzer();
    Analyzer(map<string, map<string, vector<string> > > meta_data);
    virtual ~Analyzer();
    void processSeries(string ur_field, uint64_t *ur_id, int *ur_time, double *ur_data);

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
    void analyzeData(string ur_field, uint64_t *ur_id, double *ur_data);
    //set reference profile values
    void modifyProfile(string &ur_field, uint64_t *ur_id, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it);
    //push data to the control structure
    double pushData(double *ur_data, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it);
    //get median
    double getMedian(map<int,vector<double> >::iterator &sensor_it);
    //get moving average and variance
    pair<double, double> getAverageAndVariance(string &ur_field, uint64_t *ur_id, map<string,map<string, vector<string> > >::iterator &meta_it, map<int,vector<double> >::iterator &sensor_it);
    //get cumulatie moving average
    double cumulativeAverage(map<int,vector<double> >::iterator &sensor_it, map<string, map<string, vector<string> > >::iterator &meta_it);
    //print series data and meta information
    void printSeries(string &ur_field);
};
    
