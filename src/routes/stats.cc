#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>
#include <tuple>
#include <vector>

#include "errors.hh"
#include "http_messages.hh"
#include "misc.hh"
#include "routes.hh"
#include "server.hh"
using namespace std;

void handle_stats(const Socket_t& sock, HttpRequest* const request, vector<string> vec) {
  string msg;
  string first = vec.at(0);
  string second = vec.at(1);
  string third = vec.at(2);
  auto end = std::chrono::system_clock::now();
  string fn = second;
  request->method = first;
  request->request_uri = second;
  request->http_version = third;
  request->message_body = msg;
  request->filename = fn;
}