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

int isDir(const char *path);
void  parse_request(const Socket_t& sock, HttpRequest* const request);
void separate(HttpRequest* const request , string line);
Server::Server(SocketAcceptor const& acceptor) : _acceptor(acceptor) { }

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
  resp.status_code=401;
    if(resp.message_body.length()==0){
    resp.status_code = 404;
  }
  if(s.length()==0){

    request.headers["WWW-Authenticate"]="Basic realm=\"CS 252_web_server_p5 \"";
    resp.status_code = 401;
    resp.message_body="";
  }
  else{

    if(s.compare( "Basic YWRpdHlhOnZhcmRoYW4=") == 0){
      resp.status_code=200; 
    }
    else{
      resp.status_code = 401;
    }
  }




  resp.headers["Connection"] = "close";
  resp.headers["Content-Length"] = request.message_body.length();
  resp.headers["Content-Type"] = get_content_type(request.filename);
  cout << resp.status_code << endl;
  cout << resp.message_body.size() << endl;
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
  string first = "GET";
  string second = "";
  string third = "";
  // Last character read
  char lastChar = 0;
  string line = sock->readline();
  // cout << line << endl;
  char*arrr[3];
  int i1 = 0;
  for( i1 = 4; i1 < line.length(); i1++){
    char ch = line.at(i1);
    // if(ch == ' ' && second.compare("")!=0){
      if(ch == ' '){
      second=trim(second);
      break;
    }
    second+=ch;
  }
  i1=i1+1;
  for( ; i1 < line.length()-2; i1++){
    char ch = line.at(i1);
    third+=ch;
  }
  third=trim(third);
  cout << third << endl;
  std::fstream fs;

  // if(isDir(second.c_str())==1){
  //   if(second.at(second.length()-1 )== '/'){
  //     second+="index.html";
  //   }
  //   else{
  //      second+="/index.html";
  //   }
  // }

  if(second.compare("/")==0){
    second = "/index.html";
  }

  if (second.find("html") != std::string::npos || second.find("svg") != std::string::npos ) {
    hflag = 1;
  }

  string fn = "http-root-dir/htdocs"+second;
 
  msg="";
  nstr="";
  FILE *fptr = fopen(fn.c_str(), "r");

  if(fptr!=NULL){
  fs.open (fn, std::fstream::in | std::fstream::out  );
  
  if(hflag == 1){
  if (fs.is_open()){

    if (hflag==1 ) {
      while ( getline (fs,line) )
      {
        msg+=line;
      }
      fs.close();
    }
  }
  
  }
    else{
      streampos size;
      // cout  <<fn << endl;
      ifstream file (fn, ios::in|ios::binary|ios::ate);
      if (file.is_open())
      {
        size = file.tellg();
        file.seekg (0, ios::beg);
      
      
      int c;
      FILE *f = fopen(fn.c_str(), "rb");
      int i = 0;
      for( i = 0 ; i < size ; i++){
        c = fgetc(f);
        nstr+=c;
      }
     msg=nstr;
     fclose(f);
     }
    }
  }

 

  request->method = first;
  request->request_uri = second;
  request-> http_version = third;
  line = sock->readline();
  while(line.compare("\r\n")!=0){
    // cout << line << endl;
    line.pop_back();
    line.pop_back();
    separate(request,line);
    line=sock->readline();
  }

  request->message_body = msg ;
  request->filename=fn;
   
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

int isDir(const char *path)
{
    struct stat stats;

    stat(path, &stats);

    // Check for file existence
    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
}