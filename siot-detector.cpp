#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <getopt.h>
#include <signal.h>
#include <thread>
#include <unistd.h>

#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include "fields.h"
#include "ConfigParser.h"
#include "config.h"
#include "Analyzer.h"

/*
    TODO
    podpora vebose zprav
    vylepseni exportovaciho formatu (pridat string)
    uprava nazvu promennych
    doplneni komentaru
    testovani scenaru
*/

using namespace std;

trap_module_info_t *module_info = NULL;

#define MODULE_BASIC_INFO(BASIC) \
  BASIC("data-series-detector", "This module detect anomalies in data series", 1, 1)
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

//prepare output interfaces for periodic export
int initExportInterfaces(map<string, map<string, vector<string> > > &series_meta_data,  ur_template_t *** export_template, trap_ctx_t **ctx_export, void ***data_export, map<int,pair<string, vector<string> > > &ur_export_fields){
    string interface_spec; //name of output interface specification
    //map<int,string> ur_export_fields; //map with unirec values for each interface
    int flag = 0; //flag for definig output interface name
    vector<string> field_name; //tmp value for ur_values
    int number_of_keys = 0; //counter for number of export values
    string tmp_ur_export; //unirect export format

    //go through configuration data
    for (auto main_key: series_meta_data){
        for (auto element : series_meta_data[main_key.first]) {
            //find export key in configuration meta data
            if (element.first == "export"){
                //clear tmp variables
                flag = 0;
                field_name.clear();
                for (auto elem: element.second){
                    cout << "test elem: " << elem << endl;
                    //skip empty values
                    if(elem == "-"){
                        break;
                    }
                    //update tmp variables
                    field_name.push_back(elem);
                    if(flag == 0){
                        interface_spec += "u:export-"+main_key.first+",";
                        number_of_keys++;
                        flag = 1;
                    }
                }   
                if (flag == 1){
                   // cout << "insert: " << field_name << ", " << main_key.first << endl;
                    //insert tmp variables to the map structure
                    pair <string, vector<string> > tmp(main_key.first,field_name); 
                    ur_export_fields.insert(pair<int,pair<string, vector<string> > >(number_of_keys-1, tmp));
                }
            }
        }
    }


    //no export parameters were specified
    if (interface_spec.length() == 0){
        return 1;
    }
    //remove last comma
    interface_spec.pop_back();

    //allocate memory for output export interface
    *export_template = (ur_template_t **)calloc(number_of_keys,sizeof(*export_template));
    *data_export = (void **) calloc(number_of_keys,sizeof(void *));
    if (export_template == NULL){
        cerr << "ERROR: Export output template allocation error" << endl;
        return 2;
    }
    if (*data_export == NULL){
        cerr << "ERROR: Export output data record allocation error" << endl;
        return 2;
    }
    

    //interface initialization
    *ctx_export = trap_ctx_init3("data-periodic-export", "Export data profile periodicaly",0,number_of_keys,interface_spec.c_str(),NULL);
    if (*ctx_export == NULL){
        cerr << "ERROR: Data export interface initialization failed" << endl;
        return 3;
    }

    //interface control & create unirec template
    for (int i = 0; i < number_of_keys; i++ ){
        if ( trap_ctx_ifcctl(*ctx_export, TRAPIFC_OUTPUT,i,TRAPCTL_SETTIMEOUT,TRAP_WAIT) != TRAP_E_OK ) {
            cerr << "ERROR: export interface control setup failed" << endl;
            return 4;
        }

        tmp_ur_export.clear();
        //create unirec export format
        for (auto elem: ur_export_fields[i].second){
            tmp_ur_export += elem + ",";
        }
        //remove last comma
        tmp_ur_export.pop_back();

        *(export_template)[i] = ur_ctx_create_output_template(*ctx_export,i,tmp_ur_export.c_str(),NULL);
        if ( (*export_template)[i] == NULL ) {
            cerr << "ERROR: Unable to define unirec fields" << endl;
            return 5;
        }

        (*data_export)[i] = ur_create_record((*export_template)[i], 0);
        if ( (*data_export)[i] == NULL ) { 
            cerr << "Error: Data are not prepared for the export template" << endl;
            return 6;
        }
    }
    return 0;
}

int main (int argc, char** argv){
    
    int exit_value = 0; //detector return value
    ur_template_t * in_template = NULL; //UniRec input template
    ur_template_t * alert_template = NULL; //UniRec output template
    ur_template_t **export_template = NULL;
    trap_ctx_t *ctx = NULL;
    trap_ctx_t *ctx_export = NULL;
    int ret = 2;
    int verbose = 0;
    void *data_alert = NULL;
    void **data_export = NULL;
    map<int,pair<string, vector<string> > > ur_export_fields; //map with unirec values for each interface

    uint64_t *ur_id = 0;
    double *ur_time = 0;  
    double *ur_data = 0;
    uint8_t data_fmt = TRAP_FMT_UNIREC;


    //parse created configuration file
    ConfigParser cp("config.txt");
    auto series_meta_data = cp.getSeries();

    //create analyze object
    Analyzer series_a (series_meta_data);

    //printSeries(series_meta_data);
 
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

    //Parse remaining parameters and get configuration -> I don't need any param now
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

    //check number of interfaces parameter
    if (strlen(ifc_spec.types) != 2) {
        cerr <<  "Error: Module requires just one input and one output interface" << endl;
        FREE_MODULE_INFO_STRUCT(MODULE_BASIC_INFO, MODULE_PARAMS)
        return 4;
    }

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
    //output interface control settings
    if (trap_ctx_ifcctl(ctx, TRAPIFC_OUTPUT,0,TRAPCTL_SETTIMEOUT, TRAP_WAIT) != TRAP_E_OK){
        cerr << "ERROR in alert output interface initialization" << endl;
        exit_value=2;
        goto cleanup;
    }
    //create empty input template
    in_template = ur_ctx_create_input_template(ctx, 0, NULL, NULL);
    if (in_template == NULL) {
        cerr <<  "ERROR: unirec input template create fail" << endl;
        exit_value=2;
        goto cleanup;
    }
    //create alert template
    alert_template = ur_ctx_create_output_template(ctx, 0, "ID,TIME", NULL);
    if (alert_template == NULL) {
        cerr <<  "ERROR: unirec alert template create fail" << endl;
        exit_value=2;
        goto cleanup;
    }
    
    //set required incoming format
    //trap_ctx_set_required_fmt(ctx, 0, TRAP_FMT_UNIREC, NULL);

    //initialize export output interfaces
    ret = initExportInterfaces(series_meta_data, &export_template, &ctx_export, &data_export, ur_export_fields);
    if (ret > 1){
        exit_value=2;
        goto cleanup;
    }

    data_alert = ur_create_record(alert_template, UR_MAX_SIZE);
        if ( data_alert == NULL ) { 
            cout << "ERROR: Data are not prepared for alert template" << endl;
            exit_value=3;
            goto cleanup;
        }

    if (verbose >= 0) {
        cout << "Initialization done" << endl;
    }

    //set initialized values
    series_a.setAlertInterface(ctx,alert_template,data_alert);
    series_a.setExportInterface(ctx_export, export_template, data_export, ur_export_fields);




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
            series_a.processSeries(ur_get_name(id), ur_id, ur_time, ur_data);
        }
    }

cleanup:
    //cleaning
    trap_ctx_finalize(&ctx);
    trap_ctx_finalize(&ctx_export);
    return exit_value;

}
