#include "http_messages.hh"

// You may find implementing this function and using it in server.cc helpful

HttpResponse handle_htdocs(const Socket_t& sock,HttpRequest* const request,vector <string> vec) {
  HttpResponse response;
  response.http_version = request.http_version;
  return response;
}
