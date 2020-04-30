#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "errors.hh"
#include "http_messages.hh"
#include "misc.hh"
#include "routes.hh"
#include "server.hh"
namespace fs = std::experimental::filesystem;
using namespace std;
int isDir(const char* path);
std::string generate_html(const char* pth);
void generate_html(string& msg, string& fn);
void generate_file(string& msg, string& fn, string& second);

// You may find implementing this function and using it in server.cc helpful

HttpResponse  handle_htdocs(const Socket_t& sock, HttpRequest* const request, vector<string> vec) {
  int dir_flag = 0;
  HttpResponse response;
  string msg = "";
  string first = vec.at(0);
  string second = vec.at(1);
  string third = vec.at(2);
  string fn = "http-root-dir/htdocs" + second;
  // cout << fn << endl;

  if (isDir(fn.c_str()) == 1 && second.at(second.length() - 1) == '/' && second.length() != 1) {
    dir_flag = 1;
  }

  if (isDir(fn.c_str()) == 1 && second.at(second.length() - 1) != '/') {
    second += "/index.html";
  }
  // cout << "FLAG  "<< dir_flag <<endl;
  if (dir_flag == 1) {
    request->content_type = "text/html;charset=us-ascii";
    generate_html(msg, fn);

  }

  else {
    generate_file(msg, fn, second);
  }

  request->method = first;
  request->request_uri = second;
  request->http_version = third;
  request->message_body = msg;
  request->filename = fn;
  return response;
}

int isDir(const char* path) {
  struct stat stats;

  stat(path, &stats);

  // Check for file existence
  if (S_ISDIR(stats.st_mode))
    return 1;

  return 0;
}

void generate_html(string& msg, string& fn) {
  string name;
  string link;
  DIR* d = opendir(fn.c_str());
  if (NULL == d) {
    perror("opendir: ");
    exit(1);
  }

  for (dirent* ent = readdir(d); NULL != ent; ent = readdir(d)) {
    name = ent->d_name;
    link = name;
    DIR* d1 = opendir((fn + name).c_str());
    if (d1 != NULL) {
      link = "\"" + link + "/" + "\"";
    } else {
      link = "\"" + link + "\"";
    }
    closedir(d1);
    msg += "<p><a href=" + link + ">" + name + "</a></p>";
    msg += "\n";
  }
  closedir(d);
}

void generate_file(string& msg, string& fn, string& second) {
  if (second.compare("/") == 0)
    second = "/index.html";

  fn = "http-root-dir/htdocs" + second;
  // cout << fn <<endl;
  std::ifstream is(fn);  // open file
  if (is.is_open()) {
    char c;
    while (is.get(c))  // loop getting single characters
      msg += c;

    is.close();
  }
}
