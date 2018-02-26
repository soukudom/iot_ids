#include <string>
#include <vector>

using namespace std;

class Alert{
    public:
        Alert();
        virtual ~Alert();
    private:
        int init_state;
        bool alert;
        bool export_data;
        bool periodic_check;
        vector<string> export_fields;
        string alert_message;
};
