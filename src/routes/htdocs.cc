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
using namespace std;
int isDir(const char *path);

// You may find implementing this function and using it in server.cc helpful

HttpResponse handle_htdocs(const Socket_t& sock,HttpRequest* const request,vector <string> vec) {
  HttpResponse response;
  string msg;
  string first = vec.at(0);
  string second = vec.at(1);
  string third = vec.at(2);
 if(isDir(second.c_str())==1){
    if(second.at(second.length()-1 )== '/'){
      second+="index.html";
    }
    else{
       second+="/index.html";
    }
  }

  if(second.compare("/")==0){
    second = "/index.html";
  }

  if (second.find("html") != std::string::npos || second.find("svg") != std::string::npos ) {
    hflag = 1;
  }

  string fn = "http-root-dir/htdocs"+second;
 
  msg="";
  nstr="";

 std::ifstream is(fn);     // open file
  char c;
  while (is.get(c))          // loop getting single characters
    msg+=c;

  is.close();

  response.http_version = request.http_version;
  
  request->method = first;
  request->request_uri = second;
  request-> http_version = third;
  request->message_body = msg ;
  return response;
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