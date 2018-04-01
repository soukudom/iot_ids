/**
 * \file Analyzer.h
 * \brief Analyze time series data based on configuration
 * \author Dominik Soukup <soukudom@fit.cvut.cz>
 * \date 2018
**/
/* NOTE: Detailed explanation of this code is in documentation. */

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

/*
 Analyze time series data
*/
class Analyzer{
public:
    // Default constructor
    Analyzer();

    /**
    * Param constructor. Initialize configuration data
    * \param[in] meta_data Parse configuration data by ConfigParser class
    */
    Analyzer(map<string, map<string, vector<string> > > meta_data, int verbose);

    // Default destructor
    virtual ~Analyzer();

    /**
    * Main method that proces every incoming unirec field
    * \param[in] ur_field Name of unirec field
    * \param[in] ur_id ID value in unirec record
    * \param[in] ur_time TIME value in unirec record
    * \param[in] ur_data Value of ur_field
    */
    void processSeries(string ur_field, uint64_t *ur_id, double *ur_time, double *ur_data);

    /**
    * Setter for alert output interface
    * \param[in] alert_ifc Alert trap interface
    * \param[in] alert_template Unirec template for alert interface
    * \param[in] data_alert Unirec allocated records for alert interface
    */
    void setAlertInterface(trap_ctx_t *alert_ifc, ur_template_t *alert_template, void *data_alert);

    /**
    * Setter for export output interface
    * \param[in] export_ifc Export trap interface
    * \param[in] export_template Unirec templates for export interfaces
    * \param[in] data_export Unirec allocated records for export interfaces
    */
    void setExportInterface(trap_ctx_t *export_ifc, ur_template_t **export_template, void **data_export, map<int, vector<string> > ur_export_fields);


private:
/* 
* Member variables 
*/
    trap_ctx_t *alert_ifc;          // Alert trap interface
    ur_template_t *alert_template;  // Unirec template for alert interface
    void *data_alert;               // Unirec allocated records for alert interface
    int verbose;                    // Verbose level

    trap_ctx_t *export_ifc;                                  // Export trap interface
    ur_template_t **export_template;                         // Unirec templates for export interfaces
    void **data_export;                                      // Unirec allocated records for export interfaces
    map<int,vector<string> >ur_export_fields; // Map with unirec values for each interface. The first key is number of interface and the second is name of record according to the configuration file (ur_field). In the last vector are profile items for export.

    map<string, map<string, vector<string> > > series_meta_data; // Parsed data from configuration file by ConfigParser. Data sequence: unirec field, subsection category (profile, profile items, export, general, metaData, metaProfile, profile), config params
    map<string, map<int, vector<double> > > control;             // Structure for time series data. Data sequence: unirec field, sensor ID, data series values

    /*
    * Buffers for moving variance
    */
    map<string, map<int, vector<double> > > x;  // Buffer for normal values
    map<string, map<int, vector<double> > > x2; // Buffer for square values

/*
* Internal methods. Used during processing time series.  
*/
    /**
    * Initialize data series structures
    * \param[in] ur_field Name of unirec field
    * \param[in] ur_id ID value in unirec record
    * \param[in] ur_data Value of ur_field
    * \param[in] ur_time TIME value in unirec record
    * \returns State of init process. 0 - init done, other values - init still in process
    */
    int initSeries(string &ur_field, uint64_t *ur_id, double *ur_data, double *ur_time);

    /**
    * Analyze data series data. Wrapper for all available methods.
    * \param[in] ur_field Name of unirec field
    * \param[in] ur_id ID value in unirec record
    * \param[in] ur_data Value of ur_field
    * \param[in] ur_time TIME value in unirec record
    * \returns Structure with alert messages
    */
    map<string,vector<string> > analyzeData(string ur_field, uint64_t *ur_id, double *ur_data, double *ur_time);

    /** 
    * Set reference profile values. Wrapper for all available profile methods.
    * \param[in] ur_field Name of unirec field 
    * \param[in] ur_id ID value in unirec record
    * \param[in] meta_it Iterator pointing to specific data in series_meta_data structure
    * \param[in] sensor_it Iterator pointing to specific data in control structure
    * \param[in] meta_id Flag switching between based (established during init) and right now profile
    */
    void modifyMetaData(string &ur_field, uint64_t *ur_id, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator sensor_it, string meta_id);

    /**
    * Push data to the control structure
    * \param[in] ur_time TIME value in unirec record
    * \param[in] ur_data Value of ur_field
    * \param[in] meta_it Iterator pointing to specific data in series_meta_data structure
    * \param[in] sensor_it Iterator pointing to specific data in control structure
    * \param[in] meta_id Flag switching between based (established during init) and right now profile
    * \returns The new pushed value
    */
    double pushData(double *ur_time, double *ur_data, map<string, map<string, vector<string> > >::iterator &meta_it, map<int, vector<double> >::iterator &sensor_it, string meta_id);

    /**
    * Get unirec field index for profile name
    * \param[in] name Name of profile 
    * \returs Unirec field index
    */
    int getIndex(string name);

    /**
    * Add alert to the alert structure
    * \param[in] profile_name Name of profile
    * \param[in] alert_message Alert message
    * \param[in] alert_str Structure for storing all alerts
    */
    void addAlert(string &profile_name, string alert_message, map<string, vector<string> > &alert_str);

    /**
    * Send alert
    * \param[in] alert_str Structure containing all alerts
    * \param[in] ur_field Name of unirec field
    * \param[in] ur_id ID value in unirec record
    * \param[in] ur_time TIME value in unirec record
    */
    void sendAlert(map<string, vector<string> > &alert_str, string &ur_field, uint64_t *ur_id, double *ur_time);


/*
* Profile determination methods
*/
    /**
    * Get median from data series
    * \param[in] sensor_it Iterator pointing to specific data in control structure
    * \returns Median value
    */
    double getMedian(map<int,vector<double> >::iterator &sensor_it);

    /**
    * Get moving average and variance from data series
    * \param[in] ur_field Name of unirec field
    * \param[in] ur_id ID value in unirec record
    * \param[in] meta_it Iterator pointing to specific data in series_meta_data structure
    * \param[in] sensor_it Iterator pointing to specific data in control structure
    * \param[in] meta_id Flag switching between based (established during init) and right now profile
    * \returns Pair<average, variance>
    */
    pair<double, double> getAverageAndVariance(string &ur_field, uint64_t *ur_id, map<string,map<string, vector<string> > >::iterator &meta_it, map<int,vector<double> >::iterator &sensor_it, string meta_id);

    /**
    * Get cumulatie moving average from data series
    * \param[in] sensor_it Iterator pointing to specific data in control structure
    * \param[in] meta_it Iterator pointing to specific data in series_meta_data structure
    * \param[in] meta_id Flag switching between based (established during init) and right now profile
    * \returns Cumulative average value
    */
    double getCumulativeAverage(map<int,vector<double> >::iterator &sensor_it, map<string, map<string, vector<string> > >::iterator &meta_it, string meta_id);

    /**
    * Print series data and meta information
    * \param[in] ur_field Name of unirec field
    */
    void printSeries(string &ur_field);

/*
* Alert detection methods
*/
    /**
    * Check soft and hard limits
    * \param[in] meta_it Iterator pointing to specific data in series_meta_data structure
    * \param[in] ur_field Name of unirec field
    * \param[in] ur_id ID value in unirec record
    * \param[in] ur_time TIME value in unirec record
    * \param[in] ur_data Value of ur_field
    * \param[in] alert_str Structure for storing all alerts
    */
    void dataLimitCheck(map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > & alert_str);

    /**
    * Check data change ration
    * \param[in] sensor_it Iterator pointing to specific data in control structure
    * \param[in] meta_it Iterator pointing to specific data in series_meta_data structur
    * \param[in] ur_field Name of unirec field
    * \param[in] ur_id ID value in unirec record
    * \param[in] ur_time TIME value in unirec record
    * \param[in] ur_data Value of ur_field
    * \param[in] alert_str Structure for storing all alerts
    */
    void dataChangeCheck(map<int,vector<double> >::iterator &sensor_it, map<string, map<string, vector<string> > >::iterator &meta_it, string ur_field, uint64_t *ur_id, double *ur_time ,double *ur_data, map<string,vector<string> > & alert_str);

    /**
    * Thread member functions
    */
    /*
    * Periodically check whether data are up to date. Run as a separate thread.
    * \param[in] period Specified period of time
    * \param[in] ur_field Name of unirec field
    */
    void periodicCheck(int period, string ur_field);

    /**
    * Periodically export defined fields. Run as a separate thread.
    * \param[in] perioad Specified period of time
    * \param[in] ur_field Name of unirec field
    */
    void periodicExport(int period, string ur_field);

    /**
    * Start monitoring thread after init phase
    */
    void runThreads(string &ur_field);
};
