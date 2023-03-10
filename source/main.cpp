#include "wifi_manager.hpp"
#include "socket_manager.hpp"
#include "mbed-trace/mbed_trace.h"
#include "ISM43362Interface.h"
#include <iostream>

AnalogIn in(A5);
DigitalIn button(PA_0);
ISM43362Interface wifi(true);

void printNetworkInfo(){
    SocketAddress a;
    wifi.get_ip_address(&a);
    printf("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    wifi.get_netmask(&a);
    printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    wifi.get_gateway(&a);
    printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
}

int main()
{
    int result = wifi.connectToWifiIfSaved();
    if(result != 0){
        result = wifi.setCredentialsForAP(MBED_CONF_APP_AP_SSID, MBED_CONF_APP_AP_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
        do {
            wifi.startSoftAP();
            result = wifi.listenForConnection();

            if (result != 0) {
                printf("Wifi connection error: %d\n", result);
            }
        }while(result != 0);

        wifi.saveWifiSettings();
    }
    printf("Wifi connected\n\n");

    printNetworkInfo();
    
    SocketManager *socketManager = new SocketManager(&wifi);
    socketManager->init();

    socketManager->sync_date();
    while(true){
        if(button.read()){
            NVIC_SystemReset();
        }

        result = socketManager->post_data(in.read_u16());
        if(result != 0){
            socketManager->reset(&wifi);
        }
        ThisThread::sleep_for(5s);
    }
}
