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
#include <pthread.h>
#include <stdio.h>
#include <thread>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <algorithm>
#include <algorithm>
#include <chrono>
#include <ctime>
using namespace std;
auto start = std::chrono::system_clock::now();
void  parse_request(const Socket_t& sock, HttpRequest* const request);
void separate(HttpRequest* const request , string line);
Server::Server(SocketAcceptor const& acceptor) : _acceptor(acceptor) { }
void handle_stat(const Socket_t& sock,HttpRequest* const request,vector <string> vec);
pthread_mutex_t _mutex;
// string filename;
// int hflag=0;

void Server::run_linear() const {
  while (1) {
    Socket_t sock = _acceptor.accept_connection();
    handle(sock);
  }
}

void Server::run_fork() const {
  
  while (1) {
    Socket_t slaveSocket = _acceptor.accept_connection();
    int ret = fork();
    if (ret == 0) {
      handle(slaveSocket);
      exit(0);
    }
    waitpid(-1, NULL, WNOHANG) ;

  }

}


// PTHREAD VERSION
struct ThreadParams {
const Server * server;
Socket_t sock;
};

void dispatchThread( ThreadParams * params) {
printf("Dispatch Thread\n");
// Thread dispatching this request
params->server->handle(params->sock);
// Delete params struct
delete params;
}


void Server::run_thread() const {
pthread_mutex_t mutex;
while (1) {
// Accept request

Socket_t sock = _acceptor.accept_connection();
// Put socket in new ThreadParams struct
ThreadParams * threadParams = new ThreadParams;
threadParams->server = this;
threadParams->sock = std::move(sock);
// Create thread
pthread_t thrID;
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
pthread_create(&thrID, &attr, (void* (*)(void*) )dispatchThread,(void *) threadParams);

}
}


void* Server::run_linear2(const Server* serv) const {
  while (1) {
    Socket_t sock = _acceptor.accept_connection(); 
    handle(sock);
  }
}


void Server::run_thread_pool(const int num_threads) const {
  pthread_t thread[num_threads];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  for (int i=0; i< num_threads; i++) { 

    pthread_create(&thread[i],&attr,(void* (*)(void*))run_linear2(this),NULL);
  } 
  run_linear2(this);
}






// example route map. you could loop through these routes and find the first route which
// matches the prefix and call the corresponding handler. You are free to implement
// the different routes however you please

  //  std::vector<Route_t> route_map = {
  //  std::make_pair("/cgi-bin", handle_cgi_bin),
  //  std::make_pair("/", handle_htdocs),
  //  std::make_pair("", handle_default)
  //  };
 


void Server::handle(const Socket_t& sock) const {
  HttpRequest request;
  // TODO: implement parsing HTTP requests
  // recommendation:
  parse_request( sock, &request);
  //  request.print();

  HttpResponse resp;
  // TODO: Make a response for the HTTP request
  resp.http_version = request.http_version;
  resp.message_body = request.message_body;
  resp.reason_phrase = "OK";
  string s =  request.headers["Authorization"];
  resp.status_code=401;

  if(resp.message_body.length() == 0){
    resp.status_code=404;

  }
  
  if(s.length()==0){
    request.headers["WWW-Authenticate"]="Basic realm=\"CS 252_web_server_p5 \"";
    resp.status_code = 401;
    resp.message_body="";
  }
  else{

    if(s.compare( "Basic YWRpdHlhOnZhcmRoYW4=") == 0){
      if(resp.message_body.length()!=0)
      resp.status_code=200; 
    }
    else{
      resp.status_code = 401;
    }
  }



  resp.headers["Connection"] = "close";
  resp.headers["Content-Length"] = request.message_body.length();
  resp.headers["Content-Type"] = get_content_type(request.filename);
  // cout << resp.headers["Content-Type"] << endl;
  if(request.filename.compare("http-root-dir/cgi-bin/jj")==0 || request.filename.compare("http-root-dir/cgi-bin/jj-mod.so")==0){
     resp.headers["Content-Type"] ="text/html";
  }

  sock->write(resp.to_string());
  
  


}

void  parse_request(const Socket_t& sock, HttpRequest* const request){
  // Buffer used to store the name received from the client
  int hflag = 0;
  vector <string> vec;
  vector <string>head;
  char *name;
  char newChar;
  string msg;
  string nstr;
  string first = "";
  string second = "";
  string third = "";
  char* p;
  string line = sock->readline();
  string st = line;
  string tmp="";

  st.erase(std::remove(st.begin(), st.end(),'\r'),st.end());
  st.erase(std::remove(st.begin(), st.end(),'\n'),st.end());
  cout << st << endl;
  pthread_mutex_lock(&_mutex);
  p = strtok ((char*)st.c_str()," ");
  while (p != NULL)
  {
    tmp = p;
    vec.push_back(tmp);
    p = strtok (NULL, " ");
  }
  pthread_mutex_unlock(&_mutex);
  if(vec.size()== 3){
  first = vec.at(0);
  second = vec.at(1);
  third = vec.at(2);
  }
  else{
    return;
  }
 
  cout << second << endl;
  if (second.find("cgi-bin") != std::string::npos) {
    handle_cgi_bin(sock,request,vec);
  }
  else if(second.find("stats") != std::string::npos){
     handle_stat(sock,request,vec);
  }
  else{
    handle_htdocs(sock,request,vec);
  }
  

  line = sock->readline();
  while(line.compare("\r\n")!=0){
  line.erase(std::remove(line.begin(), line.end(),'\r'),line.end());
  line.erase(std::remove(line.begin(), line.end(),'\n'),line.end());
    separate(request,line);
    line=sock->readline();
  }

}
//  GET /index.html HTTP/1.1
// GET /hello HTTP/1.1

void separate(HttpRequest* const request , string line){
  
  string first="";
  string second="";
  int i = 0;
  for( i = 0 ; i < line.length();i++){
    char ch = line.at(i);
    if(ch ==':'){
      break;
    }
    first +=ch;
  }
  i+=1;
  for(  ; i <line.length();i++  ){
    char ch = line.at(i);
    second+=ch;
  }
  
  request->headers[trim(first)]=trim(second); 

}

void handle_stat(const Socket_t& sock,HttpRequest* const request,vector <string> vec){
  string msg;
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  msg = msg + "elapsed time: " + std::to_string(elapsed_seconds.count()) + "\n";
  string first = vec.at(0);
  string second = vec.at(1);
  string third = vec.at(2);
  string fn = second;
  request->method = first;
  request->request_uri = second;
  request-> http_version = third;
  request->message_body = msg ;
  request->filename=fn;
}

