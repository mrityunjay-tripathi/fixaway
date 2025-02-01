/**
* @file fixaway/connection.hpp
* @author Mrityunjay Tripathi
*
* Connection is a base class implementation for various network/file stream
* sources, through which FIX messages can exchanged.
*
* fixaway is free software; you may redistribute it and/or modify it under the
* terms of the BSD 2-Clause "Simplified" License. You should have received a copy of the
* BSD 2-Clause "Simplified" License along with fixaway. If not, see
* http://www.opensource.org/licenses/BSD-2-Clause for more information.
* Copyright (c) 2025, Mrityunjay Tripathi
*/

#ifndef FIXAWAY_CONNECTION_HPP_
#define FIXAWAY_CONNECTION_HPP_

#include <ctime>
#include <string>
#include <memory>
#include <cstdlib>
#include <functional>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <ringbuffer/ringbuffer.h>

namespace fixaway {

    class connection_exception : public std::exception
    {
    public:
        connection_exception(int ec, const std::string& msg) : ec(ec), msg(msg) {}
        connection_exception(int ec, const char* msg) : ec(ec), msg(msg)  {}
        int code() const { return ec; }
        const char* what() const noexcept { return msg.c_str(); }
    private:
        int ec;
        std::string msg;
    };

    using on_connect = std::function<void()>;
    using on_disconnect = std::function<void()>;
    using on_error = std::function<void(int, const std::string&)>;

    template <typename ConnectionType>
    class base_connection
    {
    public:
        static constexpr const int DEFAULT_ERROR_CODE = -1;
        static constexpr const int MAX_READ_SIZE = 8 * 1024;
        static int64_t system_timestamp();
    public:
        base_connection(const std::string& remote_address, int port,
                on_connect on_connect_cb, on_disconnect on_disconnect_cb, on_error on_error_cb);
        ~base_connection();
        int connect();
        int disconnect();
        const char* read_ptr();
        int move_head(int size);
        int size();
        int poll();
        int send_message(const char *buffer, int size);
        bool active() const;
        int64_t last_sent_at() const;
        int64_t last_read_at() const;
    protected:
        bool has_data();
        int close_file_descriptor(int fd);
    protected:
        int sockfd = -1;
        bool is_active = false;
        int port = 0;
        std::string remote_address;
        int64_t last_read_timestamp = 0;
        int64_t last_sent_timestamp = 0;
        vrb_ctx_t* vrb_context = nullptr;
        on_connect on_connect_cb;
        on_disconnect on_disconnect_cb;
        on_error on_error_cb;
    };

    class tcp_client : public base_connection<tcp_client> {
    public:
        typedef base_connection<tcp_client> base;
        using base::vrb_context;
        using base::last_read_timestamp;
        using base::last_sent_timestamp;
    public:
        tcp_client(const std::string& remote_address, int port,
                on_connect on_connect_cb, on_disconnect on_disconnect_cb, on_error on_error_cb);
        ~tcp_client();
        int connect();
        int disconnect();
        int poll();
        int send_message(const char* buffer, int size);
    private:
        void error_handler();
        int open_connection(const char *hostname, const char *port);
    };

    class tcp_ssl_client : public base_connection<tcp_ssl_client> {
    public:
        typedef base_connection<tcp_ssl_client> base;
        using base::vrb_context;
        using base::last_read_timestamp;
        using base::last_sent_timestamp;
    public:
        tcp_ssl_client(const std::string& remote_address, int port,
                on_connect on_connect_cb, on_disconnect on_disconnect_cb, on_error on_error_cb);
        ~tcp_ssl_client();
        int connect();
        int disconnect();
        int poll();
        int send_message(const char* buffer, int size);
    private:
        void error_handler(int ret_val);
        int open_connection(const char *hostname, const char *port);
    private:
        SSL* ssl = nullptr;
        SSL_CTX* ssl_context = nullptr;
    };

    class udp_client : public base_connection<udp_client> {
    public:
        typedef base_connection<udp_client> base;
        using base::vrb_context;
        using base::last_read_timestamp;
        using base::last_sent_timestamp;
    public:
        udp_client(const std::string& remote_address, int port,
                on_connect on_connect_cb, on_disconnect on_disconnect_cb, on_error on_error_cb);
        ~udp_client();
        int connect();
        int disconnect();
        int poll();
        int send_message(const char* buffer, int size);

    private:
        void error_handler();
        int open_connection(const char *hostname, const char *port);
    private:
        struct sockaddr_in server_addr;
    };

    class file_client : public base_connection<file_client> {
    public:
        typedef base_connection<file_client> base;
        using base::vrb_context;
        using base::last_read_timestamp;
        using base::last_sent_timestamp;
        enum class io_error : int
        { fsize = 1, fopen = 2, fclose = 3, fread = 4, fwrite = 5, load_file = 6, save_file = 7, malloc = 8, large_file = 9};
    public:
        file_client(const std::string& filename,
                on_connect on_connect_cb, on_disconnect on_disconnect_cb, on_error on_error_cb);
        ~file_client();
        int connect();
        int disconnect();
        int poll();
        int send_message(const char* buffer, int size);
    private:
        void error_handler(io_error ec, const std::string& msg);
        FILE* fopen_or_die(const char *filename, const char *instruction);
        void fclose_or_die(FILE *file);
        size_t fsize_or_die(const char *filename);
    private:
        std::string filename;
        FILE* rfile = nullptr;
        FILE* wfile = nullptr;
    };
}

#include "connection_impl.hpp"
#endif

