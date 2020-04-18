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
using namespace std;


void  parse_request(const Socket_t& sock, HttpRequest* const request);
void separate(HttpRequest* const request , string line);
Server::Server(SocketAcceptor const& acceptor) : _acceptor(acceptor) { }
mutex_unlock(&mutex);
string filename;
streampos size;
int hflag=0;
string nstr;
    char *buff;

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
while (1) {
// Accept request
mutex_lock(&mutex);
Socket_t sock = _acceptor.accept_connection();
mutex_unlock(&mutex);
// Put socket in new ThreadParams struct
ThreadParams * threadParams = new ThreadParams;
threadParams->server = this;
threadParams->sock = std::move(sock);
// Create thread
pthread_t thrID;
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
pthread_create(&thrID, &attr, (void* (*)(void*) )dispatchThread,
(void *) threadParams);
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
  // request.print();

  HttpResponse resp;
  // TODO: Make a response for the HTTP request
  resp.http_version = request.http_version;
  resp.message_body = request.message_body;
  resp.reason_phrase = "OK";
  string s =  request.headers["Authorization"];
 
  if(s.length()==0){
    request.headers["WWW-Authenticate"]="Basic realm=\"CS 252_web_server_p5 \"";
    resp.status_code = 401;

  }
  else{

    if(s.compare( "Basic YWRpdHlhOnZhcmRoYW4=") == 0){
      resp.status_code=200; 
    }
    else{
      resp.status_code = 401;
    }
  }

  if((resp.message_body).length()==0){
    resp.status_code = 404;
  }


  resp.headers["Connection"] = "close";
  resp.headers["Content-Length"] = (request.message_body).length();
  resp.headers["Content-Type"] = get_content_type(filename);
  if(filename.find("hello") != std::string::npos){
    resp.headers["Content-Type"] = "html";
  }

  if(hflag==1){
    // std::cout << resp.to_string() << std::endl;
    sock->write(resp.to_string());
  }
  else{
    // cout << resp.headers["Content-Type"] << endl;
    sock->write(resp.to_string());
  }
  
  hflag = 0;

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

  if(vec.at(1).compare("/")==0){
    vec.at(1) = "/index.html";
  }

  if (vec.at(1).find("html") != std::string::npos || vec.at(1).find("svg") != std::string::npos 
      ||vec.at(1).find("hello") != std::string::npos) {
    hflag = 1;
  }

  string fn = "http-root-dir/htdocs"+vec.at(1);
  filename=fn;
  fs.open (fn, std::fstream::in | std::fstream::out | std::fstream::app );
  
  if(hflag == 1){
  if (fs.is_open()){

    if (hflag==1 ) {
      while ( getline (fs,line) )
      {
        msg+=line;
      }
      fs.close();
      if(vec.at(1).compare("/hello")==0){
        msg="Hello CS252!";
      }
    }
  }
  }
    else{
      // streampos size;
      cout  <<fn << endl;
      ifstream file (fn, ios::in|ios::binary|ios::ate);
      if (file.is_open())
      {
        size = file.tellg();
        file.seekg (0, ios::beg);
      }
       nstr="";
      int c;
      FILE *f = fopen(fn.c_str(), "rb");
      cout << "SIIIZE: " << size <<endl;
      for(int i = 0 ; i < size ; i++){
        c = fgetc(f);
        nstr+=c;
      }
     msg=nstr;
    }
  

 

  request->method = vec.at(0);
  request->request_uri = vec.at(1);
  request-> http_version = vec.at(2);
  line = sock->readline();
  while(line.compare("\r\n")!=0){
    line.pop_back();
    line.pop_back();
    separate(request,line);
    line=sock->readline();
  }

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
    token = strtok(NULL, ":"); 
  }
  request->headers[vec.at(0)]=vec.at(1); 
}
