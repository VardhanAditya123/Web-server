/**
 * This file contains your implementation of a TLS socket and socket acceptor. The TLS socket uses
 * the OpenSSL library to handle all socket communication, so you need to configure OpenSSL and use
 * the OpenSSL functions to read/write to the socket. src/tcp.cc is provided for your reference on
 * Sockets and SocketAdaptors and examples/simple_tls_server.c is provided for your reference on
 * OpenSSL.
 */

#include "tls.hh"

#include <arpa/inet.h>
#include <errno.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>

#include "errors.hh"
SSL_CTX *create_context();
void init_openssl();
void configure_context(SSL_CTX *ctx);
void cleanup_openssl();
using namespace std;
TLSSocket::TLSSocket(int port_no, struct sockaddr_in addr, SSL *ssl)
    : _socket(port_no), _addr(addr), _ssl(ssl) {
  // TODO: Task 2.1
  char inet_pres[INET_ADDRSTRLEN];
  // sin_family will be AF_INET
  if (inet_ntop(addr.sin_family, &(addr.sin_addr), inet_pres, INET_ADDRSTRLEN))
    ip = inet_pres;
}

TLSSocket::~TLSSocket() noexcept { close(_socket); }

string TLSSocket::set_ip() { return ip; }

char TLSSocket::getc() {
  char c[1];
  //   SSL_read(ssl, request, 4096);
  // 		puts(request);
  //         SSL_write(ssl, reply, strlen(reply));
  ssize_t read = SSL_read(_ssl, c, 1);
  if (read < 0) {
    //
    c[0] = EOF;
  } else if (read > 1) {
    throw ConnectionError("Read more than one byte when expecting to only read one.");
  }

  return c[0];
}

ssize_t TLSSocket::read(char *buf, size_t buf_len) {
  ssize_t r = recv(_socket, buf, buf_len, 0);
  if (r == -1) {
    throw ConnectionError("Unable to read a character: " + std::string(strerror(errno)));
  }
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

void TLSSocket::write(std::string const &str) { write(str.c_str(), str.length()); }

void TLSSocket::write(char const *const buf, const size_t buf_len) {
  // cout << buf << endl;
  signal(SIGPIPE,SIG_IGN);
  if (buf == NULL || buf_len == 0)
    return;
  int ret_code = SSL_write(_ssl, buf, buf_len);
  if (ret_code == -1) {
    throw ConnectionError("Unable to write: " + std::string(strerror(errno)));
  } else if ((size_t) ret_code != buf_len) {
    size_t i;
    std::stringstream buf_hex_stream;
    for (i = 0; i < buf_len; i++)
      buf_hex_stream << std::hex << buf[i];
        signal(SIGPIPE,SIG_IGN);
    throw ConnectionError("Could not write all bytes of: \'" + buf_hex_stream.str() +
                          "\'. Expected " + std::to_string(buf_len) + " but actually sent " +
                          std::to_string(ret_code));
  }

}

TLSSocketAcceptor::TLSSocketAcceptor(const int portno) {
  init_openssl();
  _ssl_ctx = create_context();
  configure_context(_ssl_ctx);
  int s;

  _addr.sin_family = AF_INET;
  _addr.sin_port = htons(portno);
  _addr.sin_addr.s_addr = htonl(INADDR_ANY);

  _master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_master_socket < 0) {
    perror("Unable to create socket");
    exit(EXIT_FAILURE);
  }

  if (bind(_master_socket, (struct sockaddr *) &_addr, sizeof(_addr)) < 0) {
    perror("Unable to bind");
    exit(EXIT_FAILURE);
  }

  if (listen(_master_socket, 1) < 0) {
    perror("Unable to listen");
    exit(EXIT_FAILURE);
  }
}

Socket_t TLSSocketAcceptor::accept_connection() const {
  // TODO: Task 2.1
  // call
  // while in examples
  // remodel to tcp

  struct sockaddr_in addr;
  uint len = sizeof(addr);
  SSL *ssl;

  int client = accept(_master_socket, (struct sockaddr *) &addr, &len);
  if (client < 0) {
    perror("Unable to accept");
    exit(EXIT_FAILURE);
  }

  ssl = SSL_new(_ssl_ctx);
  SSL_set_fd(ssl, client);

  if (SSL_accept(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
  }

  return std::make_unique<TLSSocket>(client, addr, ssl);
}

TLSSocketAcceptor::~TLSSocketAcceptor() noexcept {
  // TODO: Task 2.1
  // close(_master_socket);
  // SSL_CTX_free(_ssl_ctx);
  // cleanup_openssl();
}

SSL_CTX *create_context() {
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

void init_openssl() {
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() { EVP_cleanup(); }

void configure_context(SSL_CTX *ctx) {
  SSL_CTX_set_ecdh_auto(ctx, 1);

  /* Set the key and cert */
  if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
}

// openssl req -newkey rsa:4096 -nodes -sha512 -x509 -days 21 -nodes -out cert.pem -keyout key.pem
// chmod 700 *.pem