#include "mbed.h"
#include "cert.hpp"
#include "https_request.h"
#include <string>
#include <sstream>

class SocketManager {
    static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;
    static constexpr size_t REMOTE_PORT = 443;

    TLSSocket *_socket;
    NetworkInterface *_net;

public:
    SocketManager(NetworkInterface *net) : _net(net), _socket(new TLSSocket()) {
    }

    ~SocketManager() {
        _socket->close();
        delete _socket;
        delete _net;
    }

    void reset(NetworkInterface *net){
        _net = net;
        _socket = new TLSSocket();
        init();
    }

    void init() {
        open_socket();
        set_tls_cert_and_hostname();
        connect_socket();
    }

    void sync_date() {
        string date = get(MBED_CONF_APP_SYNC_DATE_URL);
        set_time(stoi(date));
    }

    int post_data(unsigned short data) {
        string var = "{\"guid\": \"" + string(MBED_CONF_APP_DEVICE_GUID) + "\", \"epochDate\": " + to_string(time(NULL)) + ", \"dataInt\": " + to_string(data) + "}";
        printf("posting: %s\n", var.c_str());
        return post(MBED_CONF_APP_DATA_URL, &var);
    }
private:
    void open_socket(){
        nsapi_size_or_error_t result = _socket->open(_net);
        if (result != 0) {
            printf("Error! _socket.open() returned: %d\r\n", result);
            return;
        }
    }

    void set_tls_cert_and_hostname() {
        nsapi_size_or_error_t result = _socket->set_root_ca_cert(cert);
        if (result != NSAPI_ERROR_OK) {
            printf("Error: _socket.set_root_ca_cert() returned %d\n", result);
            return;
        }
        _socket->set_hostname(MBED_CONF_APP_HOSTNAME);
    }

    SocketAddress get_address() {
        SocketAddress address;
        if (!resolve_hostname(address)) {
            return NULL;
        }

        address.set_port(REMOTE_PORT);

        printf("Opening connection to remote port %d\r\n", REMOTE_PORT);
        return address;
    }

    void connect_socket() {
        nsapi_size_or_error_t result = _socket->connect(get_address());
        if (result != 0) {
            printf("Error! _socket.connect() returned: %d\r\n", result);
            return;
        }
    }

    bool resolve_hostname(SocketAddress &address) {
        const char hostname[] = MBED_CONF_APP_HOSTNAME;

        printf("\nResolve hostname %s\r\n", hostname);
        nsapi_size_or_error_t result = _net->gethostbyname(hostname, &address);
        if (result != 0) {
            printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
            return false;
        }

        printf("%s address is %s\r\n", hostname, (address.get_ip_address() ? address.get_ip_address() : "None") );

        return true;
    }

    int post(const char* url, string *body) {
        HttpsRequest *post_req = new HttpsRequest(_socket, HTTP_POST, url);
        post_req->set_header("Content-Type", "application/json");

        HttpResponse* post_res = post_req->send(body->c_str(), strlen(body->c_str()));
        if (!post_res) {
            printf("HttpRequest failed (error code %d)\n", post_req->get_error());
            return post_req->get_error();
        }

        printf("\n----- HTTPS POST response -----\n");
        dump_response(post_res);
    
        delete post_req;
        return 0;
    }

    string get(const char* url) {
        HttpsRequest* get_req = new HttpsRequest(_socket, HTTP_GET, url);

        HttpResponse* get_res = get_req->send();
        if (!get_res) {
            printf("HttpRequest failed (error code %d)\n", get_req->get_error());
            return "";
        }
        printf("\n----- HTTPS GET response -----\n");
        dump_response(get_res);

        string body = get_res->get_body_as_string();
        delete get_req;
        return body;
    }

    void dump_response(HttpResponse* res) {
        printf("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str());

        /*printf("Headers:\n");
        for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
            printf("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
        }*/
        printf("\nBody (%u bytes):\n%s\n\n", res->get_body_length(), res->get_body_as_string().c_str());
    }
};