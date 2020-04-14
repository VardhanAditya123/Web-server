/**
 * This file contains the primary logic for your server. It is responsible for
 * handling socket communication - parsing HTTP requests and sending HTTP responses
 * to the client. 
 */

#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <unistd.h>
#include <string.h>
#include "server.hh"
#include "http_messages.hh"
#include "errors.hh"
#include "misc.hh"
#include "routes.hh"

using namespace std;
void  parse_request(const Socket_t& sock, HttpRequest* const request);
Server::Server(SocketAcceptor const& acceptor) : _acceptor(acceptor) { }

void Server::run_linear() const {
  while (1) {
    Socket_t sock = _acceptor.accept_connection();
    handle(sock);
  }
}

void Server::run_fork() const {
  // TODO: Task 1.4
}

void Server::run_thread() const {
  // TODO: Task 1.4
}

void Server::run_thread_pool(const int num_threads) const {
  // TODO: Task 1.4
}

// example route map. you could loop through these routes and find the first route which
// matches the prefix and call the corresponding handler. You are free to implement
// the different routes however you please
/*
std::vector<Route_t> route_map = {
  std::make_pair("/cgi-bin", handle_cgi_bin),
  std::make_pair("/", handle_htdocs),
  std::make_pair("", handle_default)
};
*/

void Server::handle(const Socket_t& sock) const {
  HttpRequest request;
  // TODO: implement parsing HTTP requests
  // recommendation:
  parse_request( sock, &request);
  request.print();

  HttpResponse resp;
  // TODO: Make a response for the HTTP request
  resp.http_version = "HTTP/1.1";
  std::cout << resp.to_string() << std::endl;
  sock->write(resp.to_string());
}

 void  parse_request(const Socket_t& sock, HttpRequest* const request){
   // Buffer used to store the name received from the client
  const int MaxName = 1024;
  char name[ MaxName + 1 ];
  int nameLength = 0;
  int n;
  char newChar;

  // Last character read
  char lastChar = 0;

  //
  // The client should send <name><cr><lf>
  // Read the name of the client character by character until a
  // <CR><LF> is found.
  //
    
  while ( nameLength < MaxName &&(sock->read(  &newChar, sizeof(newChar) ) ) > 0 ) {

    if ( lastChar == '\015' && newChar == '\012' ) {
      // Discard previous <CR> from name
      nameLength--;
      break;
    }

    name[ nameLength ] = newChar;
    nameLength++;

    lastChar = newChar;
  }
  sock->write( name, strlen( name ));

  // Send last newline
//   const char * newline="\n";
//   write(fd, newline, strlen(newline));
 }