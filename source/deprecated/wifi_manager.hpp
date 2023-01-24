#include "mbed.h"

class WifiManager {
    static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;

    NetworkInterface *_net;

public:
    WifiManager() : _net(NetworkInterface::get_default_instance()) {
    }

    ~WifiManager() {
        if(_net) {
            _net->disconnect();
        }
    }


    NetworkInterface *connect() {
        if (!_net) {
            printf("Error! No network interface found.\r\n");
            return NULL;
        }

        if (_net->wifiInterface()) {
            wifi_scan();
        }

        printf("Connecting to the network...\r\n");

        nsapi_size_or_error_t result = _net->connect();
        if (result != 0) {
            printf("Error! _net->connect() returned: %d\r\n", result);
            return NULL;
        }

        print_network_info();

        printf("Network connected successfully \r\n");

        return _net;
    }

private:
    void wifi_scan() {
        WiFiInterface *wifi = _net->wifiInterface();

        WiFiAccessPoint ap[MAX_NUMBER_OF_ACCESS_POINTS];

        int result = wifi->scan(ap, MAX_NUMBER_OF_ACCESS_POINTS);

        if (result <= 0) {
            printf("WiFiInterface::scan() failed with return value: %d\r\n", result);
            return;
        }

        printf("%d networks available:\r\n", result);
        for (int i = 0; i < result; i++) {
            printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\r\n",
                   ap[i].get_ssid(), get_security_string(ap[i].get_security()),
                   ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
                   ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5],
                   ap[i].get_rssi(), ap[i].get_channel());
        }
        printf("\r\n");
    }

    const char *get_security_string(nsapi_security_t sec) {
        switch (sec) {
            case NSAPI_SECURITY_NONE:
                return "None";
            case NSAPI_SECURITY_WEP:
                return "WEP";
            case NSAPI_SECURITY_WPA:
                return "WPA";
            case NSAPI_SECURITY_WPA2:
                return "WPA2";
            case NSAPI_SECURITY_WPA_WPA2:
                return "WPA/WPA2";
            case NSAPI_SECURITY_UNKNOWN:
            default:
                return "Unknown";
        }
    }

    void print_network_info() {
        SocketAddress a;
        _net->get_ip_address(&a);
        printf("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_netmask(&a);
        printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_gateway(&a);
        printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    }
};