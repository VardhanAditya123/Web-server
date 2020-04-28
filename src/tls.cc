/**
 * This file contains your implementation of a TLS socket and socket acceptor. The TLS socket uses
 * the OpenSSL library to handle all socket communication, so you need to configure OpenSSL and use the
 * OpenSSL functions to read/write to the socket. src/tcp.cc is provided for your reference on 
 * Sockets and SocketAdaptors and examples/simple_tls_server.c is provided for your reference on OpenSSL.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <iostream>
#include <sstream>
#include <cstring>
#include <memory>

#include "tls.hh"
#include "errors.hh"
SSL_CTX *create_context();
void init_openssl();
void configure_context(SSL_CTX *ctx);
void cleanup_openssl();

TLSSocket::TLSSocket(int port_no, struct sockaddr_in addr, SSL* ssl) :
  _socket(port_no), _addr(addr), _ssl(ssl) {
    // TODO: Task 2.1
}
TLSSocket::~TLSSocket() noexcept {
    // TODO: Task 2.1
}

char TLSSocket::getc() {
    // TODO: Task 2.1
    char c = '\0';
    return c;
}

ssize_t TLSSocket::read(char *buf, size_t buf_len) {
    // TODO: Task 2.1
    ssize_t r = 0;
    return r;
}

std::string TLSSocket::readline() {
    std::string str;
    char c;
    while ((c = getc()) != '\n' && c != EOF) {
        str.append(1, c);
    }
    if (c == '\n') {
        str.append(1, '\n');
    }
    return str;
}

void TLSSocket::write(std::string const &str) {
    write(str.c_str(), str.length());
}

void TLSSocket::write(char const * const buf, const size_t buf_len) {
    if (buf == NULL) return;
  int ret_code = send(_socket, buf, buf_len, 0);
  if (ret_code == -1) {
      throw ConnectionError("Unable to write: " + std::string(strerror(errno)));
  } else if ((size_t)ret_code != buf_len) {
      size_t i;
      std::stringstream buf_hex_stream;
      for (i = 0; i < buf_len; i++)
        buf_hex_stream << std::hex << buf[i];

      throw ConnectionError("Could not write all bytes of: \'" + buf_hex_stream.str() +
          "\'. Expected " + std::to_string(buf_len) + " but actually sent " +
          std::to_string(ret_code));
  }
}

int TLSSocket::ret_sock(){
    return _socket;
}

TLSSocketAcceptor::TLSSocketAcceptor(const int portno) {
    // TODO: Task 2.1
    //call all methods
    //init , create , configure
   
    init_openssl();
    _ssl_ctx = create_context();
    configure_context(_ssl_ctx);
    int s;


    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(portno);
    _addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	perror("Unable to create socket");
	exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
	perror("Unable to bind");
	exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
	perror("Unable to listen");
	exit(EXIT_FAILURE);
    }



}

Socket_t TLSSocketAcceptor::accept_connection() const {
    // TODO: Task 2.1
    //call
    //while in examples
    //remodel to tcp
}

TLSSocketAcceptor::~TLSSocketAcceptor() noexcept {
    // TODO: Task 2.1
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
	perror("Unable to create SSL context");
	ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    return ctx;
}

void init_openssl()
{ 
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl()
{
    EVP_cleanup();
}

void configure_context(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }
}