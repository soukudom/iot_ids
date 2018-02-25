#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <getopt.h>
#include <signal.h>


#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include "fields.h"
#include "ConfigParser.h"
#include "config.h"
#include "Analyzer.h"

using namespace std;

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
  BASIC("data-series-detector", "This module detect anomalies in data series", 1, -1)
#define MODULE_PARAMS(PARAM)

//test print function 
//auto specifier in function parameter is available from c++14
void printSeries( map<string,map<string, vector<string> > >& series_meta_data){
    cout << "printSeries method" << endl;
    for (auto main_key: series_meta_data){
        cout << "main key: " << main_key.first << endl;
         for (auto element : series_meta_data[main_key.first]) {
            cout << " key: " << element.first << endl;
            cout << "  values: " << endl;
            for (auto elem: element.second){
                cout << "   " << elem << endl;
            }   
        }   
    }
}

int main (int argc, char** argv){
    
    int exit_value = 0; //detector return value
    static int n_inputs = 1; //number of input interface
    static ur_template_t * in_template = NULL; //UniRec input template
    trap_ctx_t *ctx = NULL;
    int ret = 2;
    int verbose = 0;

    uint64_t *ur_id = 0;
    double *ur_time = 0;  
    double *ur_data = 0;
    uint8_t data_fmt = TRAP_FMT_UNIREC;

    //parse created configuration file
    ConfigParser cp("config.txt");
    auto series_meta_data = cp.getSeries();

    Analyzer series_a (series_meta_data);

//    printSeries(series_meta_data);
 
    //**interface initialization**
    //Allocate and initialize module_info structure and all its members
    INIT_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
    // trap parameters processing
    trap_ifc_spec_t ifc_spec;
    ret = trap_parse_params(&argc, argv, &ifc_spec);
    if (ret != TRAP_E_OK) {
        if (ret == TRAP_E_HELP) { // "-h" was found
            trap_print_help(module_info);
            return 0;
        }   
        cerr << "ERROR in parsing of parameters for TRAP: " << trap_last_error_msg << endl;
        return 1;
    }   

    // Parse remaining parameters and get configuration -> I don't need any now
    signed char opt;
    while ((opt = TRAP_GETOPT(argc, argv, module_getopt_string, long_options)) != -1) {
        switch (opt) {
        default:
            cerr << "Error: Invalid arguments." << endl;
            FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
            return 1;
        }   
    }

    verbose = trap_get_verbose_level();
    if (verbose >= 0) {
        cout << "Verbosity level: " <<  trap_get_verbose_level() << endl;;
    }

    if (verbose >= 0) {
        cerr << "Number of inputs: " <<  n_inputs << endl;
    }

    //check input parameter
    if (n_inputs != 1) {
        cerr <<  "Error: Number of input interfaces must be 1" << endl;
        FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
        return 4;
    }

    // Set number of input interfaces
    module_info->num_ifc_in = n_inputs;

    if (verbose >= 0) {
        cout << "Initializing TRAP library ..." << endl;
    }

    ctx = trap_ctx_init(module_info, ifc_spec);

    if (ctx == NULL) {
        cerr << "ERROR in TRAP initialization: " << trap_last_error_msg << endl;
        exit_value=1;
        goto cleanup;
    }

    //input interface control settings
    if (trap_ctx_ifcctl(ctx, TRAPIFC_INPUT, 0, TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK) {
        cerr << "ERROR in input interface initialization" << endl;
        exit_value=2;
        goto cleanup;
    }
    //create empty input template
    in_template = ur_ctx_create_input_template(ctx, 0, NULL, NULL);
    if (in_template == NULL) {
        cerr <<  "ERROR: unirec template create fail" << endl;
        goto cleanup;
    }

    //set required incoming format
    //trap_ctx_set_required_fmt(ctx, 0, TRAP_FMT_UNIREC, NULL);

    if (verbose >= 0) {
        cout << "Initialization done" << endl;
    }


    //main loop
    while (true){
        
        uint16_t memory_received = 0;
        const void *data_nemea_input = NULL;

        //received data and interate over all fields
        TRAP_CTX_RECEIVE(ctx,0,data_nemea_input,memory_received,in_template);

        ur_id = ur_get_ptr(in_template, data_nemea_input, F_ID);
        ur_time = ur_get_ptr(in_template, data_nemea_input, F_TIME);


        //go through all unirec fields
        ur_field_id_t id = UR_ITER_BEGIN;
        while ((id = ur_iter_fields(in_template, id)) != UR_ITER_END) {
            //skip id and time values
            if ( strcmp("ID",(ur_get_name(id))) == 0 /*|| strcmp("TIME",(ur_get_name(id))) == 0*/ ){
                continue;
            }
            ur_data = (double*) ur_get_ptr_by_id(in_template, data_nemea_input,id);
//            cout << "ur_data: " << ur_get_name(id) << ": " << *ur_data << endl;
            series_a.processSeries(ur_get_name(id), ur_id, ur_time, ur_data);
        }
    }

cleanup:
   //cleaning
   trap_ctx_finalize(&ctx);
   return exit_value;

}
