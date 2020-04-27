#ifndef  INCLUDE_SERVER_HH_
#define INCLUDE_SERVER_HH_

#include "socket.hh"

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
    auto min_time, max_time, start_time, min_time_URL, max_time_URL;
};

#endif  // INCLUDE_SERVER_HH_
