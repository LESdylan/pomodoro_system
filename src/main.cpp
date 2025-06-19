#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ctime>
#include <cstring>

typedef struct s_var
{
    
}               t_var;

std::string&    getLogFile()
{
    static std::string logFile = "/var/log/pomodoro_timer.log";
    return (logFile);
}

bool    isDaemon(int argc, char **argv)
{
    bool    daemonMode = false;
    int i;
    int size;

    size = argc - 1;
    std::string daemon;
    while (i < size)
    {
        daemon = argv[i];
        if (daemon == "--daemon" || daemon == "-d")
        {
            daemonMode = true;
            break ;
        }
        argv++;
    }
    if (daemonMode)
        getLogFile() = std::string(getenv("HOME")) + "./local/share/pomodoro-timer/logs/pomodoro-timer.log";
    else
        getLogFile() = "pomodoro-timer.log";
}

void    signal_handler(int signal)
{
    bool
    if (signal == SIGTERM || signal == SIGINT)
    {
        running = false;
        std::cout << "Received signal" << signal << ", shutting down..." << std::endl;
    }
}
int main(int argc, char **argv)
{
    bool    daemonMode;
    bool    running;

    daemonMode = false;
    running = true;
    std::string daemon;

    daemon = argv[1];
    
}