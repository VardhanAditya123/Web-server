#ifndef  INCLUDE_SERVER_HH_
#define INCLUDE_SERVER_HH_

#include "socket.hh"
#include <algorithm>
#include <chrono>
#include <ctime>

class Server {

 private:
    SocketAcceptor const& _acceptor;
   

 public:
    explicit Server(SocketAcceptor const& acceptor);
    void run_linear() const;
    void* run_linear2(const Server*) const;
    void run_fork() const; 
    void run_thread_pool(const int num_threads) const;
    void run_thread() const;
    void handle(const Socket_t& sock) const;
    void loopthread() const ;
    void set_portno();
    int port_number;
    int ip_address;
    int req_count;
    std::chrono::time_point<std::chrono::system_clock> a;
               
struct server_stats{
int req_count = 0;
std::vector<double>timer;
std::chrono::time_point<std::chrono::system_clock>start_server;
double max;
double min;
string max_url;
string min_url;
double val;
int p_no;
};
mutable struct server_stats s1;

  
};

#endif  // INCLUDE_SERVER_HH_
