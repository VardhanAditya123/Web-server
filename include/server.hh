#ifndef INCLUDE_SERVER_HH_
#define INCLUDE_SERVER_HH_

#include <algorithm>
#include <chrono>
#include <ctime>

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
  void loopthread() const;
  void set_portno();
  int port_number;
  int ip_address;
  int req_count;
  std::chrono::time_point<std::chrono::system_clock> a;
};

#endif  // INCLUDE_SERVER_HH_
