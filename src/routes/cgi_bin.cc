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

HttpResponse handle_cgi_bin(const HttpRequest& request,vector <string> vec) {
  HttpResponse response;
  request.print();
  string msg;
  cout << "HERERE" << endl;
  response.http_version = request.http_version;
  string fn = "http-root-dir"+vec.at(1);

  std::ifstream is(fn);     // open file
  char c;
  while (is.get(c))          // loop getting single characters
    msg+=c;

  is.close();
  cout << msg << endl;
  // TODO: Task 2.2
  return response;
}
