#include "http_messages.hh"
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
#include <dlfcn.h>
#include <link.h>
#include <errno.h>
#include <stdio.h>
#include<stdlib.h>
using namespace std;
typedef void (*httprunfunc)(int ssock, const char* querystring);
// You could implement your logic for handling /cgi-bin requests here
int isDir(const char *path);
HttpResponse handle_cgi_bin(const Socket_t& sock,HttpRequest* const request,vector <string> vec) {
  HttpResponse response;
  string msg;
  string first = vec.at(0);
  string second = vec.at(1);
  string third = vec.at(2);
  string second2="";
  string second3="";
  string second4=second;
  vec.clear();
  string tmp;
  char* p = strtok ((char*)second4.c_str(),"?");
  while (p != NULL)
  {
    tmp = p;
    vec.push_back(tmp);
    p = strtok (NULL, "?");
  }

  second2 = vec.at(0);
  if(vec.size() > 1){
    second3 = vec.at(1);
    request->query = second3;
  }

  string fn = "http-root-dir"+second2;
  int out[2];
  // cout << fn << endl;
  pipe(out);
  int tmpin=dup(0);
  int tmpout=dup(1);

  int ret = fork();

  if (ret == 0) {
    dup2(out[1],1);
    close(out[1]);
    setenv( "REQUEST_METHOD","GET",1);
    if((request->query).length()!=0)
      setenv("QUERY_STRING",second3.c_str(),1);

    if(fn.find(".so")!=std::string::npos){
      void * lib = dlopen( fn.c_str(), RTLD_LAZY );

      if ( lib == NULL ) {
        fprintf( stderr, "./jj-mod.so not found\n");
        perror( "dlopen");
        exit(1);
      }
      httprunfunc hello_httprun;

      hello_httprun = (httprunfunc) dlsym( lib, "httprun");
      if ( hello_httprun == NULL ) {
        perror( "dlsym: httprun not found:");
        exit(1);
      }
      // Call the function
      hello_httprun( 1, second3.c_str());
    }

    else
      execl(fn.c_str(),NULL);

  }

  close(out[1]);
  dup2(tmpin,0);
  dup2(tmpout,1);
  close(tmpin);
  close(tmpout);

  char c;
  string str2;
  while(read(out[0],&c,1)){
    str2+=c;
  }
  close(out[0]);

  // waitpid(-1, NULL, WNOHANG) ;

  msg = str2;

  // cout <<"TESTING " << msg << endl;
  request->method = first;
  request->request_uri = second;
  request-> http_version = third;
  request->message_body = msg ;
  request->filename=fn;
  return response;
}




