#include <fstream>

//C style
class LOGFILE {
public:
    static std::ofstream log_file;
    static void open_log();
    static void log_value(float value);
    static void close_log();
};
