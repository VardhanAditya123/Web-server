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
#include <cstring>

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


// void Server::handle(const Socket_t& sock) const {
//   // Maybe parse the request and print it too
 
//   HttpResponse resp;
//   resp.http_version = "HTTP/1.2";
//   resp.status_code = 200;
//   resp.reason_phrase = "OK";
//   resp.headers["Connection"] = "close";
//   resp.headers["Content-Length"] = 12;
//   resp.message_body = "Hello CS252!";
 
//   std::cout << "Start of HTTP response:" << std::endl;
//   std::cout << "{" << resp.to_string() << "}" << std::endl;
//   sock->write(resp.to_string());
// }

void Server::handle(const Socket_t& sock) const {
  HttpRequest request;
  // TODO: implement parsing HTTP requests
  // recommendation:
  parse_request( sock, &request);
  request.print();

  HttpResponse resp;
  // TODO: Make a response for the HTTP request
  resp.http_version = request.http_version;
  resp.status_code = 200;
  resp.reason_phrase = "OK";
  resp.headers["Connection"] = "close";
  resp.headers["Content-Length"] = 12;
  resp.headers["Content-Type"] = "text/text";
  resp.message_body = "Hello CS252 Gustavo!";
 
  std::cout << resp.to_string() << std::endl;
  sock->write(resp.to_string());
}

 void  parse_request(const Socket_t& sock, HttpRequest* const request){
   // Buffer used to store the name received from the client
  vector <string> vec;
  char *name;
  char newChar;

  // Last character read
  char lastChar = 0;
  string line = sock->readline();
  // name = (char*)(line.c_str());
  // sock->write( name, strlen( name ));
  // const char * newline="\n";
  // sock-> write(newline, strlen(newline));
  
    // Returns first token 
    
    char *token = strtok((char*)(line.c_str()), " "); 
    
    // Keep printing tokens while one of the 
    // delimiters present in str[]. 
    while (token != NULL) 
    { 
   
      vec.push_back(token);
      token = strtok(NULL, " "); 
     
    } 
  
    request->method = vec.at(0);
    request->request_uri = vec.at(1);
    request-> http_version = vec.at(2).push_back();
    // std::map<std::string, std::string> headers;
    // message_body;
 }