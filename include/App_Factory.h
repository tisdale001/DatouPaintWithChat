#ifndef APP_APP_FACTORY_H
#define APP_APP_FACTORY_H

/**
 *  @file   App_Factory.hpp
 *  @brief  App_Factory class interface
 *  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
 *  @date   November 2021
 ***********************************************/

#include <App.hpp>

class App_Factory {
public:
    App_Factory();
    ~App_Factory();
    std::unique_ptr<App> Create_App();
    void communicate_with_user();

private:
    std::string role;
    std::string username;
    std::string ip_address;
    long port;

};


#endif //APP_APP_FACTORY_H