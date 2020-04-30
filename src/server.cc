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
#include <chrono>
#include <ctime>
#include <time.h>
#include <signal.h>
#include <stdio.h> 
#include <sys/time.h>   
#include <sys/resource.h> 
 #include <sys/mman.h>
using namespace std;




double findMax(string &fn);
double findMin(string &fn);
void  parse_request(const Socket_t& sock, HttpRequest* const request);
void separate(HttpRequest* const request , string line);
Server::Server(SocketAcceptor const& acceptor) : _acceptor(acceptor) { }
void handle_stat(const Socket_t& sock,HttpRequest* const request,vector <string> vec);
void update_stats(HttpRequest* const request);
void update_logs(const Socket_t& sock,HttpRequest* const request);
void handle_logs(const Socket_t& sock,HttpRequest* const request,vector <string> vec);
pthread_mutex_t _mutex;


// struct server_stats{
// int req_count = 0;
// std::vector<double>timer;
// std::chrono::time_point<std::chrono::system_clock>start_server;
// double max;
// double min;
// string max_url;
// string min_url;
// double val;
// int p_no;
// }s1;


// struct server_stats* s1 = (server_stats*)mmap(NULL, 1000, PROT_READ | PROT_WRITE,   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
void Server::run_linear() const {
  s1.min = 10;
  s1.p_no = port_number;
  s1.start_server=a;
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
    // cout << _acceptor.get_portno();
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

  auto start = std::chrono::system_clock::now();
  HttpRequest request;
  
  // TODO: implement parsing HTTP requests
  // recommendation:
  parse_request( sock, &request);
  //  request.print();

   if( request.message_body.length()==0  && (request.request_uri).length()==0){
     return;
    }
  HttpResponse resp;
  // TODO: Make a response for the HTTP request
  resp.http_version = request.http_version;
  resp.message_body = request.message_body;
  resp.reason_phrase = "OK";
  string s =  request.headers["Authorization"];
  resp.status_code=200;

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


  if(request.filename.compare("/stats")==0){
     request.content_type="html";
  }

  if(request.filename.compare("/logs")==0){
     request.content_type="html";
  }

  if((request.content_type).length()!=0){
    resp.headers["Content-Type"] = request.content_type;
  }
  else{
    resp.headers["Content-Type"] = get_content_type(request.filename);
  }
    
  sock->write(resp.to_string());
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  // cout << "BEFORE  " << elapsed_seconds.count() << endl;
  s1.val = elapsed_seconds.count();
  update_stats(&request);
  update_logs( sock, &request);

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
 

  if (second.find("cgi-bin") != std::string::npos) {
    handle_cgi_bin(sock,request,vec);
  }
  else if(second.find("stats") != std::string::npos){
     handle_stat(sock,request,vec);
  }
   else if(second.find("logs") != std::string::npos){
     handle_logs(sock,request,vec);
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
  auto end_server = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end_server-s1.start_server;
  string s_url  ="data.cs.purdue.edu:" + std::to_string( s1.p_no);
  msg+= "Name: Aditya Vardhan\n" ;
  msg+= "Number of Requests: " + std::to_string( s1.req_count) + "\n";
  msg+= "Elapsed time: " + std::to_string(elapsed_seconds.count()) + "\n";
  msg+= "Longest request: "+ std::to_string(s1.max)+ "\n";
  msg+= "Shortest request: "+ std::to_string(s1.min)+ "\n";
  msg+= "Longest request URL: "+s_url +s1.max_url+ "\n";
  msg+= "Shortest request URL: "+s_url+ s1.min_url+ "\n";
  msg+= "IP :"+sock->set_ip();

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

double findMax(string &fn){
  
  if(s1.val > s1. max){
    s1.max = s1.val;
    s1.max_url = fn;
  }
   return s1.max;
}

double findMin(string &fn){
  // cout << "MIN "<<s1.min << endl;
  if(s1.val < s1. min){
    s1.min = s1.val;
    s1.min_url = fn;
  }
   return s1.min;
}

void update_stats(HttpRequest* const request){
  s1.req_count+=1;
  findMax(request->request_uri);
  findMin(request->request_uri);
}

void update_logs(const Socket_t& sock,HttpRequest* const request){
  string msg ="";
  msg+= "\nIP :"+sock->set_ip()+"\n";
  msg+= "Route: "+request->request_uri+ "\n";
  msg+= "Response code: "+std::to_string(200)+ "\n\n";
  
  std::ofstream ofs;
  ofs.open ("myhttpd.log", std::ofstream::out | std::ofstream::app);
  ofs << msg;
  ofs.close();
}

void handle_logs(const Socket_t& sock,HttpRequest* const request,vector <string> vec){

  string msg="";

 
  std::ifstream is("myhttpd.log");     // open file
  if(is.is_open()){
    char c;
    while (is.get(c))          // loop getting single characters
      msg+=c;

    is.close();

  }
  

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