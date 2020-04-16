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
#include <fstream>

using namespace std;
string file_path;
void  parse_request(const Socket_t& sock, HttpRequest* const request);
void separate(HttpRequest* const request , string line);
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
  resp.http_version = request.http_version;
  resp.message_body = request.message_body;
  resp.status_code = 200;
  
    if((request.headers).find("Authorization")== (request.headers).end()){
       resp.headers["WWW-Authenticate"] ="Basic realm=\"CS 252_web_server_p5 \"";
       
    }
  resp.reason_phrase = "OK";
  resp.headers["Connection"] = "close";
  resp.headers["Content-Length"] = (request.message_body).length();
  resp.headers["Content-Type"] = "html";
 
 
   std::cout << resp.to_string() << std::endl; 
  sock->write(resp.to_string());
}

 void  parse_request(const Socket_t& sock, HttpRequest* const request){
   // Buffer used to store the name received from the client
  vector <string> vec;
  vector <string>head;
  char *name;
  char newChar;
  string msg;

  // Last character read
  char lastChar = 0;
  string line = sock->readline();
  line.pop_back();
  line.pop_back();
  cout << line << endl;
  char *token = strtok((char*)(line.c_str()), " "); 
    
    // Keep printing tokens while one of the 
    // delimiters present in str[]. 
    while (token != NULL) 
    { 
   
      vec.push_back(trim(token));
      token = strtok(NULL, " "); 
     
    } 
    std::fstream fs;
    // cout << fn << endl;
     if(vec.at(1).compare("/")==0){
      vec.at(1) = "/index.html";
    }
    string fn = "http-root-dir/htdocs"+vec.at(1);
    file_path = fn;
    
    fs.open (fn, std::fstream::in | std::fstream::out | std::fstream::app);
     if (fs.is_open())
  {
    while ( getline (fs,line) )
    {
       msg+=line;
    }
    fs.close();
   
  }
    request->method = vec.at(0);
    request->request_uri = vec.at(1);
    request-> http_version = vec.at(2);
      line = sock->readline();
    while(line.compare("\r\n")!=0){
      // cout << line ;
      line.pop_back();
      line.pop_back();
      separate(request,line);
      line=sock->readline();
    }
    // request->headers["Authorization"] = "Basic YWRpdHlhOnZhcmRoYW4K"
    // std::map<std::string, std::string> headers;
    request->message_body = msg ;
 }
//  GET /index.html HTTP/1.1
// GET /hello HTTP/1.1

void separate(HttpRequest* const request , string line){
   vector <string> vec;
   char *token = strtok((char*)(line.c_str()), ":"); 
    while (token != NULL) 
    { 
      vec.push_back(trim(token));
      token = strtok(NULL, " "); 
    }
    request->headers[vec.at(0)]=vec.at(1); 
}