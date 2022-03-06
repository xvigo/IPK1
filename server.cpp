/**
 * @file server.cpp
 * @author Vilem Gottwald (xgottw07@vutbr.cz)
 * @brief IPK project 1
 * @version 0.1
 * @date 2022-02-28
 */

#include <unistd.h> // sleep

#include <cstring>
#include <string>
#include <sstream>

#include <fstream>
#include <cstdio>

#include <cstdlib> // strtol

#include <sys/socket.h>
#include <netinet/in.h>

using std::string;

/**
 * @brief Round double and convert it to integer
 * 
 * @param num double to be rounded
 * @return int rounded number
 */
int round_double_to_int(double num)
{
    return (int) (num + 0.5f);
}

/**
 * @brief Get cpu times from /proc/stat
 * 
 * @param idleSum sum of idle time  
 * @param nonIdleSum sum of nonidle time
 * @return true on succes
 * @return false on failure
 */
bool get_cpu_times(unsigned long long *idleSum, unsigned long long *nonIdleSum)
{
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

    std::ifstream stat_f ("/proc/stat");
    if (!stat_f.is_open())
    {
        return false;
    }
    stat_f.ignore(3) >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
    stat_f.close();

    *idleSum = idle + iowait;
    *nonIdleSum = user + nice + system + irq + softirq + steal;
    return true;
}

/**
 * @brief Get average cpu load of the last 1s period
 * 
 * @return int (0-100) cpu load in percent 
 * @return int (-1) on failure        
 */
int get_cpu_load()
{
    unsigned long long idleSum, nonIdleSum, prevIdleSum, prevNonIdleSum;
    if (!get_cpu_times(&prevIdleSum, &prevNonIdleSum))
    {
        return -1;
    }

    sleep(1);

    if (!get_cpu_times(&idleSum, &nonIdleSum))
    {
        return -1;
    }

    unsigned long long prevTotal = prevIdleSum + prevNonIdleSum;
    unsigned long long total = idleSum + nonIdleSum;
    
    unsigned long long totalDiff = total - prevTotal;
    unsigned long long idleDiff = idleSum - prevIdleSum;

    double cpuUsage = (1.0 - (double)idleDiff/(double)totalDiff)*100.0;

    return round_double_to_int(cpuUsage);
}

/**
 * @brief Get hostname string form /proc/sys/kernel/hostname
 * 
 * @param hostname pointer to the string where hostname will be saved
 * @return true on succes
 * @return false on failure
 */
bool get_hostname(string *hostname)
{
    std::ifstream stat_f ("/proc/sys/kernel/hostname");
    if (!stat_f.is_open())
    {
        return false;
    }
    getline(stat_f, *hostname);
    stat_f.close();
    return true;
}

#define CPU_NAME_MAX_LEN 256
/**
 * @brief Get the cpu name from /proc/cpuinfo
 * 
 * @param cpu_name pointer to the string where cpu name will be saved
 * @return true on succes
 * @return false on failure
 */
bool get_cpu_name(string *cpu_name) {
   
    FILE *fp = popen("cat /proc/cpuinfo | grep \"model name\" | head -n 1 | awk \'{for (i=4; i<NF; i++) printf $i \" \"; print $NF}\' ", "r");
    if (!fp)
    {
        return false;
    }

    char aux_cpu_name[CPU_NAME_MAX_LEN];
    fgets(aux_cpu_name, CPU_NAME_MAX_LEN - 1, fp);

    pclose(fp);
    
    aux_cpu_name[std::strlen(aux_cpu_name) - 1] = '\0';
    *cpu_name = string(aux_cpu_name);
    return true;
}

#define INVALID_PORT 0
#define MAX_PORT_VAL 65535
#define MIN_PORT_VAL 1
/**
 * @brief Get the port number from commmand line argument
 *        valid port number in in range 1 - 65535
 * 
 * @param argc command line arguments count
 * @param argv array of command line arguments
 * @return unsigned short (1 - 65535) representing port
 * @return unsigned short (0) on failure
 */
unsigned short get_port_from_cla(int argc, char **argv)
{
    if (argc != 2)
    {
        return INVALID_PORT;
    }
   
    char *rest = nullptr;
    long int tmp = strtol(argv[1], &rest, 10);
    if (*rest != '\0' || tmp < MIN_PORT_VAL || tmp > MAX_PORT_VAL)
    {
        fprintf(stderr, "ERROR - Port has to be a number between 1 - 65535\n");
        return INVALID_PORT;
    }

    return (unsigned short) tmp;
}

enum Command
{
    NONE,
    GET,
    HEAD,
    NOT_IMPLEMENTED
};
/**
 * @brief Get the response message based on client request message into stringstream
 * 
 * @param response stringstream referaence for building response
 * @param buffer buffer with client request message
 */
void get_response(std::stringstream& response, char *buffer)
{
    // response templates
    const char nl[] = "\r\n";
    const char OK_header[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: ";
    const char not_found[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: 13\r\n\r\n404 Not Found";
    const char not_implemented[] = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: 54\r\n\r\n501 Not Implemented: Server only supports HEAD and GET";
    const char bad_request[] = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: 15\r\n\r\n400 Bad Request";
    const char internal_error[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: 25\r\n\r\n500 Internal Server Error";

    Command command = NONE;
    char delim[] = " ";
    char *command_str = strtok(buffer, delim);
    if(command_str == nullptr)
    {
        command = NONE;
    }
    else if (!strcmp(command_str, "GET"))
    {
        command = GET;
    }
    else if (!strcmp(command_str, "HEAD"))
    {
        command = HEAD;
    }
    else if (!strcmp(command_str, "OPTIONS"))
    {
        command = NOT_IMPLEMENTED;
    }
    else if (!strcmp(command_str, "PUT"))
    {
        command = NOT_IMPLEMENTED;
    }
    else if (!strcmp(command_str, "POST"))
    {
        command = NOT_IMPLEMENTED;
    }
    else if (!strcmp(command_str, "DELETE"))
    {
        command = NOT_IMPLEMENTED;
    }
    else if (!strcmp(command_str, "PATCH"))
    {
        command = NOT_IMPLEMENTED;
    }

    if (command == NONE)
    {
        response << bad_request;
        return;
    }
    else if (command == NOT_IMPLEMENTED)
    {
        response << not_implemented;
        return;
    }
    


    char *param_str = strtok(nullptr, delim);
    if(param_str == nullptr)
    {
        response << bad_request;
        return;
    }
    else if (!strcmp(param_str, "/hostname"))
    {
        string hostname;
        if(!get_hostname(&hostname))
        { // error
            response << internal_error;
        }
        else
        {
            response << OK_header << hostname.length() << nl << nl;
            if (command == GET)
            {
                response << hostname;
            }
        }
        return;
    }
    else if (!strcmp(param_str, "/cpu-name"))
    {
        string cpu_name;
        if(!get_cpu_name(&cpu_name))
        { // error
            response << internal_error;
        }
        else
        {
            response << OK_header << cpu_name.length() << nl << nl;
            if (command == GET)
            {
                response << cpu_name;
            }
        }
        return;
    }
    else if (!strcmp(param_str, "/load"))
    {
        int cpu_load = get_cpu_load();
        if(cpu_load < 0)
        { // error
            response << internal_error;
        }
        else
        { 
            string str_cpu_load = std::to_string(cpu_load) + "%";
            response << OK_header << str_cpu_load.length() << nl << nl;

            if (command == GET)
            {
                response << str_cpu_load;
            }
        }
        return;
    }
    else
    {
        response << not_found;
        return;
    }
}

#define DEFAULT 0
#define NONE 0
#define BACKLOG 3 
int main(int argc, char **argv)
{
    unsigned short port = get_port_from_cla(argc, argv);
    if (port == INVALID_PORT)
    {
        return EXIT_FAILURE;
    }

    int welcome_socket;
    if ((welcome_socket = socket(AF_INET, SOCK_STREAM, DEFAULT)) <= 0)
    {
        perror("ERROR - socket");
        return EXIT_FAILURE;
    }

    int opt = 1;
    if (setsockopt(welcome_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("ERROR - setsockopt");
        return EXIT_FAILURE;    
    }

    struct sockaddr_in s_address;
    int address_len = sizeof(s_address);
    s_address.sin_family = AF_INET;
    s_address.sin_port = htons(port);
    s_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(welcome_socket, (struct sockaddr *)&s_address, address_len) < 0)
    {
        perror("ERROR - bind");
        return EXIT_FAILURE;    
    }

    printf("SERVER: listening on port %hu\n", port);
    if (listen(welcome_socket, BACKLOG) < 0)
    {
        perror("ERROR - listen");
        return EXIT_FAILURE;    
    }
    
    int comm_socket;
    char buffer[1024];
    std::stringstream response;
    while(true)
    {
        if ((comm_socket = accept(welcome_socket, (struct sockaddr *)&s_address, (socklen_t*)&address_len)) < 0)
        {
            perror("ERROR - accept");
            return EXIT_FAILURE;
        }

        if (recv(comm_socket , buffer, sizeof(buffer), NONE) <= 0)
        {
            perror("ERROR - recv");
            return EXIT_FAILURE;    
        }

        response.str("");
        get_response(response, buffer);
        string str_response = response.str();
        send(comm_socket, str_response.c_str(), str_response.length() , NONE );

        close(comm_socket);
    }
    return EXIT_SUCCESS;
}
