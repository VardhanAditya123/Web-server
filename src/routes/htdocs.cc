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
#include <dirent.h>
#include <experimental/filesystem>
#include <string>
namespace fs = std::experimental::filesystem;
using namespace std;
int isDir(const char *path);
std::string generate_html(const char* pth);

// You may find implementing this function and using it in server.cc helpful

HttpResponse handle_htdocs(const Socket_t& sock,HttpRequest* const request,vector <string> vec) {
  int dir_flag = 0;
  HttpResponse response;
  string msg = "";
  string first = vec.at(0);
  string second = vec.at(1);
  string third = vec.at(2);
  string fn = "http-root-dir/htdocs"+second;
  cout << fn << endl;
  
  // if(isDir(fn.c_str())==1 && fn.find("dir1") != std::string::npos){
  //   dir_flag =  1;
  // }

  if(second.compare("/dir1/")==0){
      
    generate_html(fn.c_str());
    msg = "LOLOLOL";
  }

  else{
     if(second.compare("/")==0)
     second="/index.html";
  
  
     fn = "http-root-dir/htdocs"+second;
     std::ifstream is(fn);     // open file
     if(is.is_open()){
     char c;
     while (is.get(c))          // loop getting single characters
     msg+=c;

     is.close();
  
    }
  }

  // response.http_version = request.http_version;
  
  request->method = first;
  request->request_uri = second;
  request-> http_version = third;
  request->message_body = msg ;
  request->filename=fn;
  // request->print();
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

std::string generate_html(const char* pth){

  string str = std::to_string(pth);
  cout << str << endl;
  str.pop_back();
  DIR * d = opendir(str.c_str());
  // if (NULL == d) {
  //   perror("opendir: ");
  //   exit(1);
  // }
  // for (dirent * ent = readdir(d); NULL != ent; ent = readdir(d)) {
  //   puts(ent->d_name);
  // }
  closedir(d);

}
