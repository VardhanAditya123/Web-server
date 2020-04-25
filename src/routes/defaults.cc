#include "http_messages.hh"

// If you use the recommended route handling scheme, you will probably want a default handler

HttpResponse handle_default(const Socket_t& sock,HttpRequest* const request,vector <string> vec) {
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
}

