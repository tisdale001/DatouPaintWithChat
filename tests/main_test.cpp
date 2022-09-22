// main_test.cpp
#ifndef MAIN_TEST_CPP 
#define MAIN_TEST_CPP 
#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <SFML/Graphics/Image.hpp>

#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>
#include <tuple>
#include <map>


#include <App.hpp>
#include <Command.hpp>
#include <Draw.hpp>
#include <Clear.hpp>
#include <Paint.hpp>

// basic init function to pass to app
void initialization(App& app) {
    std::cout << "Starting the App" << std::endl;
}

// simple test case to start a server version of the app
TEST_CASE("init and destroy - client") {
    App* app = new App(2000, "Nate");
    app->Init(&initialization);
}

// simple test case to start a client version of the app
TEST_CASE("init and destroy - server") {
    App* app = new App(sf::IpAddress::getLocalAddress().toString(), 2000, "Nate");
    app->Init(&initialization);
}

// test case to change a pixel
TEST_CASE("check pixel change from Command") {
    App* app = new App(2000, "Nate");
    app->Init(&initialization);
    // check if pixel is white
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::White);
    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*app, 1, 1,
                                                           sf::Color::Black,
                                                           app->GetImage().getPixel(1, 1));
    app->AddCommand(draw);
    // execute command
    draw->execute();
    // check if pixel is now black
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::Black);
}

// test case to change pixel color, other than black
TEST_CASE("check pixel change from Command with different color") {
    App* app = new App(2000, "Nate");
    app->Init(&initialization);
    // check if pixel is white
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::White);
    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*app, 1, 1,
                                                           sf::Color::Blue,
                                                           app->GetImage().getPixel(1, 1));
    app->AddCommand(draw);
    // execute command
    draw->execute();
    // check if pixel is now black
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::Blue);
}

// check that only one pixel is modified
TEST_CASE("check Command and all other pixels are White") {
    App* app = new App(2000, "Nate");
    app->Init(&initialization);
    // check if pixel is white
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::White);
    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*app, 1, 1,
                                                           sf::Color::Black,
                                                           app->GetImage().getPixel(1, 1));

    app->AddCommand(draw);
    // execute command
    draw->execute();
    // check if all other pixels are White
    for (int i = 0; i < app->GetImage().getSize().x; i++) {
        for (int j = 0; j < app->GetImage().getSize().y; j++) {
            if (i == 1 && j == 1) {
                REQUIRE(app->GetImage().getPixel(i, j) == sf::Color::Black);
            }
            else {
                REQUIRE(app->GetImage().getPixel(i, j) == sf::Color::White);
            }
        }
    }
}

// test that undo functions
TEST_CASE("check Undo") {
    App* app = new App(2000, "Nate");
    app->Init(&initialization);
    app->initializeServerID();
    // check if pixel is white
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::White);
    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*app, 1, 1,
                                                           sf::Color::Black,
                                                           app->GetImage().getPixel(1, 1));
    app->addCommandToServer(draw);
    // execute command

    app->AddToDeque();
    draw->execute();
    // check if pixel is now black
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::Black);
    // Undo
    app->Undo();
    // check if pixel is now white
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::White);
}

// check that redo functions
TEST_CASE("check Redo") {
    App* app = new App(2000, "Nate");
    app->Init(&initialization);
    app->initializeServerID();
    // check if pixel is white
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::White);
    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*app, 1, 1,
                                                           sf::Color::Black,
                                                           app->GetImage().getPixel(1, 1));
    app->addCommandToServer(draw);
    // execute command
    draw->execute();
    // check if pixel is now black
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::Black);
    // Undo
    app->Undo();
    // check if pixel is now white
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::White);
    // Redo
    app->Redo();
    // check if pixel is black again
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::Black);
}

// test that clear screen works
TEST_CASE("clear screen") {

    App *app = new App(2000, "Nate");
    app->Init(&initialization);
    app->initializeServerID();
    // check if pixel is white
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::White);
    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*app, 1, 1,
                                                           sf::Color::Black,
                                                           app->GetImage().getPixel(1, 1));
    app->addCommandToServer(draw);
    // execute command

    app->AddToDeque();
    draw->execute();
    // check if pixel is now black
    REQUIRE(app->GetImage().getPixel(1, 1) == sf::Color::Black);

    app->setCurrentColor(sf::Color::Blue);

    std::shared_ptr<Command> blank = std::make_shared<Clear>(*app, app->getCurrentColor());
    app->addCommandToServer(blank);
    blank->execute();

    for (int i = 0; i < app->GetImage().getSize().x; i++) {
        for (int j = 0; j < app->GetImage().getSize().y; j++) {
            REQUIRE(app->GetImage().getPixel(i, j) == sf::Color::Blue);
        }
    }
}

// test where server draws and client should see change
TEST_CASE("Networking - Server Draw"){
    // Initialize Server
    App* server_app = new App(2001, "Nate");
    server_app->Init(&initialization);
    server_app->initializeServerID();
    server_app->setIPAddressCreateListener();
    server_app->connectServer();

    // Initialize Client
    App* client_app = new App(sf::IpAddress::getLocalAddress().toString(), 2001, "Nick");
    client_app->Init(&initialization);
    client_app->connect();
    client_app->communicate();

    // check if pixel is white
    REQUIRE(server_app->GetImage().getPixel(1, 1) == sf::Color::White);
    REQUIRE(client_app->GetImage().getPixel(1, 1) == sf::Color::White);

    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*server_app, 1, 1,
                                                           sf::Color::Black,
                                                           server_app->GetImage().getPixel(1, 1));
    server_app->addCommandToServer(draw);
    draw->execute();
    server_app->AddToDeque();

    REQUIRE(server_app->GetImage().getPixel(1, 1) == sf::Color::Black);
    // simple sleep take sure the receive thread has time to run
    sleep(1);
    REQUIRE(client_app->GetImage().getPixel(1, 1) == sf::Color::Black);
}

// test where client draws and server should see change
TEST_CASE("Networking - Client Draws"){
    // Initialize Server
    App* server_app = new App(2000, "Nate");
    server_app->Init(&initialization);
    server_app->initializeServerID();
    server_app->setIPAddressCreateListener();
    server_app->connectServer();

    sleep(1);

    // Initialize Client
    App* client_app = new App(sf::IpAddress::getLocalAddress().toString(), 2000, "Nick");
    client_app->Init(&initialization);
    client_app->connect();
    client_app->communicate();

    // check if pixel is white
    REQUIRE(server_app->GetImage().getPixel(1, 1) == sf::Color::White);
    REQUIRE(client_app->GetImage().getPixel(1, 1) == sf::Color::White);

    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*client_app, 1, 1,
                                                           sf::Color::Black,
                                                           client_app->GetImage().getPixel(1, 1));
    sleep(1);

    draw->execute();
    sf::Packet packet;
    sf::Color beforeColor = sf::Color(sf::Color::White);
    sf::Color toColor = sf::Color(sf::Color::Black);
    packet << 1 << (sf::Uint32)server_app->getID() << (sf::Uint32)1 <<
           1 << toColor.r <<
           toColor.g << toColor.b <<
           beforeColor.r << beforeColor.g << beforeColor.b;
    client_app->sendPacketFromClient(packet);

    REQUIRE(client_app->GetImage().getPixel(1, 1) == sf::Color::Black);
    // simple sleep take sure the receive thread has time to run
    sleep(1);
    REQUIRE(server_app->GetImage().getPixel(1, 1) == sf::Color::Black);
}

// test where server draws to multiple clients
// test where server draws and client should see change
TEST_CASE("Networking - Server Draw to Multiple Clients"){
    // Initialize Server
    App* server_app = new App(2002, "Nate");
    server_app->Init(&initialization);
    server_app->initializeServerID();
    server_app->setIPAddressCreateListener();
    server_app->connectServer();

    // Initialize Client
    App* client_app = new App(sf::IpAddress::getLocalAddress().toString(), 2002, "Nick");
    client_app->Init(&initialization);
    client_app->connect();
    client_app->communicate();

    sleep(1);

    // Initialize Second Client
    App* client_app2 = new App(sf::IpAddress::getLocalAddress().toString(), 2002, "Alison");
    client_app2->Init(&initialization);
    client_app2->connect();
    client_app2->communicate();

    sleep(1);

    // check if pixel is white
    REQUIRE(server_app->GetImage().getPixel(1, 1) == sf::Color::White);
    REQUIRE(client_app->GetImage().getPixel(1, 1) == sf::Color::White);
    REQUIRE(client_app2->GetImage().getPixel(1, 1) == sf::Color::White);


    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*server_app, 1, 1,
                                                           sf::Color::Black,
                                                           server_app->GetImage().getPixel(1, 1));
    server_app->addCommandToServer(draw);
    draw->execute();
    server_app->AddToDeque();

    sleep(1);
    REQUIRE(server_app->GetImage().getPixel(1, 1) == sf::Color::Black);
    // simple sleep take sure the receive thread has time to run
    REQUIRE(client_app->GetImage().getPixel(1, 1) == sf::Color::Black);
    REQUIRE(client_app2->GetImage().getPixel(1, 1) == sf::Color::Black);
}

// test where client draws but with a second client and a server
TEST_CASE("Networking - Client Draws with another Client Connected"){
    // Initialize Server
    App* server_app = new App(2003, "Nate");
    server_app->Init(&initialization);
    server_app->initializeServerID();
    server_app->setIPAddressCreateListener();
    server_app->connectServer();

    sleep(1);

    // Initialize Client
    App* client_app = new App(sf::IpAddress::getLocalAddress().toString(), 2003, "Nick");
    client_app->Init(&initialization);
    client_app->connect();
    client_app->communicate();

    // Initialize Client
    App* client_app2 = new App(sf::IpAddress::getLocalAddress().toString(), 2003, "Alison");
    client_app2->Init(&initialization);
    client_app2->connect();
    client_app2->communicate();

    sleep(1);

    // check if pixel is white
    REQUIRE(server_app->GetImage().getPixel(1, 1) == sf::Color::White);
    REQUIRE(client_app->GetImage().getPixel(1, 1) == sf::Color::White);
    REQUIRE(client_app2->GetImage().getPixel(1, 1) == sf::Color::White);

    // add command to change pixel to black
    std::shared_ptr<Command> draw = std::make_shared<Draw>(*client_app, 1, 1,
                                                           sf::Color::Black,
                                                           client_app->GetImage().getPixel(1, 1));
    sleep(1);

    draw->execute();
    sf::Packet packet;
    sf::Color beforeColor = sf::Color(sf::Color::White);
    sf::Color toColor = sf::Color(sf::Color::Black);
    packet << 1 << (sf::Uint32)server_app->getID() << (sf::Uint32)1 <<
           1 << toColor.r <<
           toColor.g << toColor.b <<
           beforeColor.r << beforeColor.g << beforeColor.b;
    client_app->sendPacketFromClient(packet);

    sleep(1);
    REQUIRE(client_app->GetImage().getPixel(1, 1) == sf::Color::Black);
    // simple sleep take sure the receive thread has time to run
    REQUIRE(server_app->GetImage().getPixel(1, 1) == sf::Color::Black);
    REQUIRE(client_app2->GetImage().getPixel(1, 1) == sf::Color::Black);
}

// Test of sending a message
TEST_CASE("Networking - Send Message"){
    // Initialize Server
    App* server_app = new App(2004, "Nate");
    server_app->Init(&initialization);
    server_app->initializeServerID();
    server_app->setIPAddressCreateListener();
    server_app->connectServer();

    sleep(1);

    // Initialize Client
    App* client_app = new App(sf::IpAddress::getLocalAddress().toString(), 2004, "Nick");
    client_app->Init(&initialization);
    client_app->connect();
    client_app->communicate();

    sleep(1);

    // create message and packet
    std::string new_message = "Adele: Hello, it's me.";

    char* char_msg = new char();
    strcpy(char_msg, new_message.c_str());

    sf::Packet packet;
    packet << (sf::Uint32)3 << new_message;
    // Server connected
    server_app->getMessages()->push_back(char_msg);
    server_app->sendPacket(packet);

    sleep(5);

    // check that both server and client have correct message
    for ( int i = 0; i < 22; i++){
        REQUIRE(server_app->getMessages()->at(0)[i] == new_message[i]);
        REQUIRE(client_app->getMessages()->at(0)[i] == new_message[i]);
    }
}

// test case for a larger paintbrush
TEST_CASE("check pixel change from Paint command") {
    App* app = new App(2000, "Nate");

    app->Init(&initialization);

    // set params, create paint command, execute paint command
    int radius = 4;
    int x = 50;
    int y = 50;
    app->setRadius(radius);
    std::shared_ptr<Command> draw = std::make_shared<Paint>(*app, x, y,radius,sf::Color::Black);
    draw->execute();

    // create map to store pixels that should be changed by paint command
    std::map<std::string, std::tuple<int,int>> painted_cords;

    // this is the algorithm that selects which pixels should be colored within the paint command
    for (int i = x - radius; i <= x + radius; i++) {
        for (int j = y - radius; j <= y + radius; j++) {
            if (i >= 0 && i <= app->GetImage().getSize().x && j >= 0 && j <= app->GetImage().getSize().y) {
                if ((pow(i - x, 2) + pow(j - y, 2)) <= pow(radius, 2)) {

                    // concatenate i and j coordinates for key
                    std::string c = "x";
                    c += std::to_string(i);
                    c += "y";
                    c += std::to_string(j);

                    // save pixels
                    painted_cords.insert(std::pair<std::string, std::tuple<int, int>>(c, std::make_tuple(i,j)));
                }
            }
        }
    }

    // scan through image to see if pixels that should be changed are changed
    for ( int i = 0; i < app->GetImage().getSize().x; i++ ){
        for ( int j = 0; j < app->GetImage().getSize().y; j++ ){
            // concatinate i and j coordinates for key
            std::string c = "x";
            c += std::to_string(i);
            c += "y";
            c += std::to_string(j);

            if ( painted_cords.find(c) != painted_cords.end() ){
                REQUIRE(app->GetImage().getPixel(i, j) == sf::Color::Black);
            } else {
                REQUIRE(app->GetImage().getPixel(i, j) == sf::Color::White);
            }
        }
    }
}

#endif