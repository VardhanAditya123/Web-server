#include "http_messages.hh"
using namespace std;
// You could implement your logic for handling /cgi-bin requests here

HttpResponse handle_cgi_bin(const HttpRequest& request,vector<string> vec) {
  HttpResponse response;
  request.print();
  cout << "HERERE" << endl;
  response.http_version = request.http_version;
  string fn = "http-root-dir";
  // TODO: Task 2.2
  return response;
}
