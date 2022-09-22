#ifndef APP_FACTORY_CPP
#define APP_FACTORY_CPP

/**
 *  @file   App_Factory.cpp
 *  @brief  Implementation of App_Factory.hpp
 *  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
 *  @date   November 2021
 ***********************************************/

#include <iostream>
#include <string>
#include <sstream>

#include "App_Factory.h"
#include "App.hpp"

/*! \brief default constructor for App_Factory
 *
 */
App_Factory::App_Factory() {
    // as soon as the factory is created, ask user role and collect info
    this->communicate_with_user();
}

/*! \brief Method to ask the user for the role (host or client) and then to collect the port and IP address
 *         for session.
 */
void App_Factory::communicate_with_user(){
    // ask for role
    this->role = "x";
    while ( this->role[0] != 'j' && this->role[0] != 'h' ) {
        this->role.clear();
        std::cout << "Do you want to host (h) or join (j) a meeting: " << std::endl;
        std::getline(std::cin, this->role);
    }

    // collect username
    std::cout << "Enter your name and hit enter: ";
    getline(std::cin, this->username);

    // if client, ask for IP address
    if (role[0] == 'j') {
        bool done = false;
        std::cout << "To join a meeting, provide the host IP address and the port you wish to use to connect."
                  << std::endl;

        while (!done) {
            std::cout << "Enter the host IP address for this session: ";
            getline(std::cin, this->ip_address);
            if (!this->ip_address.empty()) done = true;
        }
    }

    // for both, ask the port
    bool done = false;
    std::string text_port;

    while (!done) {
        std::cout << "Enter the port that you want to use for this session: ";
        getline(std::cin, text_port);
        std::stringstream conversion(text_port);
        conversion >> this->port;
        if ( this->port >= 0 ) done = true;
    }
}

/*! \brief Method to create App objects and return pointer
 *
 * @return A pointer to an app object.
 */
std::unique_ptr<App> App_Factory::Create_App() {
    if (role[0] == 'h') {
        std::unique_ptr<App> new_app = std::make_unique<App>(this->port, this->username);
        return new_app;

    } else {
        std::unique_ptr<App> new_app = std::make_unique<App>(this->ip_address, this->port, this->username);
        return new_app;
    }
}

/*! \brief Default destructor
 *
 */
App_Factory::~App_Factory() {
    // nothing to do
}
#endif