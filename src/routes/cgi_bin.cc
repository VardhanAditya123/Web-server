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
// You could implement your logic for handling /cgi-bin requests here

HttpResponse handle_cgi_bin(const Socket_t& sock,HttpRequest* const request,vector <string> vec) {
  HttpResponse response;
  string msg;
  string first = vec.at(0);
  string second = vec.at(1);
  string third = vec.at(2);
  string fn = "http-root-dir"+vec.at(1);
    int out[2];
    cout << fn << endl;
    pipe(out);
    int tmpin=dup(0);
    int tmpout=dup(1);
  
    int ret = fork();
  
    if (ret == 0) {
      dup2(out[1],1);
      close(out[1]);
      system(fn.c_str());
 
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

  cout <<"TESTING " << msg << endl;
  request->method = first;
  request->request_uri = second;
  request-> http_version = third;
  // response.http_version = request->http_version;
  request->print();

  return response;
}
