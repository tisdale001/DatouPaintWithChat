//#pragma once
#ifndef APP_CPP
#define APP_CPP

/** 
 *  @file   App.cpp 
 *  @brief  Main class for program
 *  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
 *  @date   November 2021
 ***********************************************/

// Include our Third-Party SFML header
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/OpenGL.hpp>

// NUKLEAR - for our GUI
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SFML_GL2_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sfml_gl2.h"
#include "../nuklear/demo/style.c"

// Include standard library C++ libraries.
#include <vector>
#include <deque>
#include <iostream>
#include <sstream>
#include <cstring>

// Project header files
#include <App.hpp>
#include <Command.hpp>
#include <Draw.hpp>
#include <Clear.hpp>

//global variable
char* buf = new char[4096];

/*! \brief	Cur_vec stands for 'current vector'.
*		It holds all the commands for the current pattern
*		being drawn by the user.
*
*/

/*!	\brief	M_commands is a deque that stores the commands so that
*		they can be 'undone'. The deque structure was chosen because
*		the size can be easily maintained by popping elements off the
*		front. Growing the deque indefinitely could crash the program.
*
*/

/*! \brief	M_redo is a stack that holds the commands so that
*		they can be redone. Further implementation reduces the
*		possible size of the stack to 100. If the user begins drawing
*		again, the stack is cleared.
*
*/


/*! \brief	Constructor for App class.
*
*/
App::App(){
    this->cur_color = sf::Color::Black;
    this->cur_radius = 1;

    this->is_server = false;
    this->newClientID = 1;
    this->appID = 0;
    this->pmouseX = 0;
    this->pmouseY = 0;
    this->mouseX = 0;
    this->mouseY = 0;
}

/*! \brief	Constructor for when App class is functioning as server.
 *
 * @param port the port to be used for the connection, a long
 * @param username the user's name, a string
*/
App::App(long port, std::string username) : App() {
    this->is_server = true;
    this->username = username;
    this->port_number = port;
    this->ip_address = "";
}

/*! \brief	Constructor for when App class is functioning as client.
 *
 * @string ip the ip address for the socket, a string
 * @param port the port to be used for the connection, a long
 * @param username the user's name, a string
*/
App::App(std::string ip, long port, std::string username) : App() {
    this->ip_address = ip;
    this->port_number = port;
    this->username = username;
}


/*! \brief Destructor for the App class.
 *
 */
App::~App(){

    // delete structs
    delete bg;
    delete ctx;

    // delete buffer
    delete []buf;


    // free the raw pointers in the message vector
    for (auto & i : *msg_vec){
        delete []i;
    }

    // clear data structures to store commands
    if (!cur_vec.empty()) {
        cur_vec.clear();
    }

    if (!m_commands.empty()) {
        m_commands.clear();
    }

    this->ClearRedo();

	// join the threads
    for (auto & i : this->thread_vector){
        i.join();
    }

    // close connections
    if ( !is_server ) {
        this->client_socket.disconnect();
    }else {
        for (int i = 0; i < this->socket_vector.size(); i++) {
            this->socket_vector.at(i)->disconnect();
        }
    }
}

/*! \brief 	AddCommand() method adds a command to
 *		    current vector.
 *
 * @param c a shared pointer to a command object
 */
void App::AddCommand(std::shared_ptr<Command> c){
	cur_vec.push_back(c);
}

/*!	\brief	AddToDeque() deep copies the current vector of commands
*		and adds it to m_commands deque. It regulates the size of
*		m_commands to size 100.
*
*/
void App::AddToDeque() {
    if (this->isServer()) {
        //deep copy current_vectors[0]
        std::shared_ptr<std::vector<std::shared_ptr<Command>>> ptrVec = std::make_shared<std::vector<std::shared_ptr<Command>>>();
	    for (int i = 0; i < current_vectors.at(0)->size(); i++) {
		    ptrVec->push_back(current_vectors.at(0)->at(i));
	    }
        while (!current_vectors.at(0)->empty()) {
            current_vectors.at(0)->pop_back();
        }
        m_commands.push_back(ptrVec);

        if (m_commands.size() > 100) {
            m_commands.front()->clear();
            m_commands.pop_front();
        }
    }
    else {
        // Client
        if (this->clientSocketClosed()) {
            // Client is disconnected: normal code
            //deep copy cur_vec
            std::shared_ptr<std::vector<std::shared_ptr<Command>>> ptrVec = std::make_shared<std::vector<std::shared_ptr<Command>>>();
            for (int i = 0; i < cur_vec.size(); i++) {
                ptrVec->push_back(cur_vec.at(i));
            }
            while (!cur_vec.empty()) {
                cur_vec.pop_back();
            }
            m_commands.push_back(ptrVec);

            // This if statement limits the size of the deque to 100.
            if (m_commands.size() > 100) {
                m_commands.front()->clear();
                m_commands.pop_front();
            }
        }
        else {
            // Client connected: send packet
            sf::Packet packet;
            packet << (sf::Uint32)5 << (sf::Uint32)this->getID();
            this->sendPacketFromClient(packet);
        }
    }
	
}

/*! \brief Undo method for when App is functioning as server.
 *
 */
void App::server_undo() {
    // Server
    // 1st check all current vectors
    for (int i = 0; i < (int)current_vectors.size(); i++) {
        if (!current_vectors.at(i)->empty()) {
            // undo all commands contained in vector
            for (int j = (int)current_vectors.at(i)->size() - 1; j >= 0; j--) {
                current_vectors.at(i)->at(j)->undo();
            }
            // Deep copy and add current vector to Redo stack
            std::shared_ptr<std::vector<std::shared_ptr<Command>>> ptrVec = std::make_shared<std::vector<std::shared_ptr<Command>>>();
            for (int k = 0; k < current_vectors.at(i)->size(); k++) {
                ptrVec->push_back(current_vectors.at(i)->at(k));
            }
            while (!current_vectors.at(i)->empty()) {
                current_vectors.at(i)->pop_back();
            }
            m_redo.push(ptrVec);
            return;
        }
    }
    // 2nd pop from undo deque
    if (m_commands.empty()) {
        return;
    }
    else {
        // m_commands not empty: undo one vector and pop
        for (int i = m_commands.back()->size() - 1; i >= 0; i--) {
            m_commands.back()->at(i)->undo();
        }
        m_redo.push(m_commands.back());
        m_commands.pop_back();
        return;
    }
}

/*! \brief Undo method for when App is functioning as client.
 *
 */
void App::client_undo() {
    // Client
    if (!clientSocketClosed()) {
        // Client is connected
        sf::Packet packet;
        packet << (sf::Uint32)6;
        this->sendPacketFromClient(packet);
        return;
    }
    else {
        // Client is not connected: normal code
        if (cur_vec.empty()) {
            // current vector is empty: the user is not drawing
            if (m_commands.empty()){
                return;
            } else {
                for (int i = m_commands.back()->size() - 1; i >= 0; i--) {
                    m_commands.back()->at(i)->undo();
                }
                m_redo.push(m_commands.back());
                m_commands.pop_back();
            }
        } else {
            // current vector is not empty: the user is still drawing
            int vec_size = cur_vec.size();
            for (int i = vec_size - 1; i >= 0; i--) {
                cur_vec[i]->undo();
            }
            std::shared_ptr<std::vector<std::shared_ptr<Command>>> ptrVec =
                    std::make_shared<std::vector<std::shared_ptr<Command>>>();

            for (int i = 0; i < cur_vec.size(); i++) {
                ptrVec->push_back(cur_vec.at(i));
            }
            while (!cur_vec.empty()) {
                cur_vec.pop_back();
            }
            m_redo.push(ptrVec);
        }
    }
}


/*!	\brief	Undo() method holds the logic for performing
*		'undos' of commands. If an undo is possible it
*		calls each Command class's undo() method within
*		the vector. This 'undos' all the pixel changes
*		while the user held down the Left mouse button.
*		In my implementation, if Undo() is called before the
*		user is completed drawing, it undos the current vector
*		and allows the user to keep drawing.
*
*/
void App::Undo() {
    // We need shared lock here in case more draw commands come in
    std::shared_lock(this->m_mutex);
    if (this->isServer()) {
        this->server_undo();
    } else {
        this->client_undo();
    }
}

/*!	\brief	ClearRedo() method clears the redo stack
*		in case the user begins drawing again. No redos
*		are possible at that point.
*
*/
void App::ClearRedo() {
    
    if (this->isServer()) {
        // Server
        while (!m_redo.empty()) {
            m_redo.top()->clear();
            m_redo.pop();
        }
    }
    else {
        // Client
        if (this->clientSocketClosed()) {
            // Client disconnected
            while (!m_redo.empty()) {
                m_redo.top()->clear();
                m_redo.pop();
            }
        }
        else {
            // Client still connected
            sf::Packet packet;
            packet << 4;
            this->sendPacketFromClient(packet);
        }
    }
}

/*!	\brief	Redo() method calls Command's execute() method
*		for all Commands in the first vector on the stack.
*		If stack is empty then redo is not possible. If redo
*		is successful then the vector is pushed onto the deque
*		so that it can be undone again.
*
*/
void App::Redo() {
    if (this->isServer()) {
        // Server
        if (m_redo.empty()) {
            return;
        }
        else {
            std::shared_ptr<std::vector<std::shared_ptr<Command>>> ptrVec = m_redo.top();
            int vec_size = ptrVec->size();
            for (int i = 0; i < vec_size; i++) {
                ptrVec->at(i)->execute();
            }
            m_commands.push_back(ptrVec);
            m_redo.pop();
        }
    }
    else {
        // Client
        if (!this->clientSocketClosed()) {
            // Client connected
            sf::Packet packet;
            packet << (sf::Uint32)7;
            this->sendPacketFromClient(packet);
        }
        else {
            // Client not connected: normal code
            if (m_redo.empty()) {
                return;
            }
            else {
                std::shared_ptr<std::vector<std::shared_ptr<Command>>> ptrVec = m_redo.top();
                int vec_size = ptrVec->size();
                for (int i = 0; i < vec_size; i++) {
                    ptrVec->at(i)->execute();
                }
                m_commands.push_back(ptrVec);
                m_redo.pop();
            }
        }
    }
	
}

/*! \brief 	Return a reference to our m_image, so that
*		we do not have to publicly expose it.
*		
*/
sf::Image& App::GetImage(){
	return m_image;
}

/*! \brief 	Return a reference to our m_Texture so that
*		we do not have to publicly expose it.
*		
*/
sf::Texture& App::GetTexture(){
	return m_texture;
}

/*! \brief 	Return a reference to our m_window so that we
*		do not have to publicly expose it.
*		
*/
sf::RenderWindow& App::GetWindow(){
	return m_window;
}

/*! \brief Return a reference to the m_clock
 *
 */
sf::Clock& App::GetClock() {
    return m_clock;
}


/*! \brief Sets the current pen color.
 *
 * @param color a sf::Color object.
 */
void App::setCurrentColor(sf::Color color) {
    this->cur_color = color;
}


/*! \brief Returns a reference to the current pen color.
 *
 * @return a reference to the current pen color, a sf::Color object.
 */
sf::Color& App::getCurrentColor(){
    return this->cur_color;
}

/*! \brief Sets the current radius, for Paint command.
 *
 *  @param new_radius an int.
 */
void App::setRadius(int new_radius){
    this->cur_radius = new_radius;
}

/*! \brief Returns the current radius.
 *
 *  @return value of current radius.
 */
int App::getCurrentRadius() {
    return cur_radius;
}

/*! \brief Returns 'ctx' for GUI.
 *
 *  @return reference to nk_context struct object.
 */
struct nk_context* App::getCTX() {
    return ctx;
}

/*! \brief Returns reference to GUI window.
 *
 *  @return reference to GUI window.
 */
sf::RenderWindow& App::GetGUIWindow() {
    return gui_window;
}

/*! \brief Returns reference to 'bg', needed for GUI.
 *
 *  @return reference to nk_colorf struct object.
 */
struct nk_colorf* App::getBG() {
    return bg;
}

/*! \brief Returns reference to smart pointer of message vector. Used for logging
 *  messages in chat feature.
 *  @return reference to message vector.
 */
std::shared_ptr<std::vector<char*>> App::getMessages() {
    return msg_vec;
}

// Here is where the server and client code begins
/*! \brief Sets the boolean that indicates if the App is functioning as a server.
 *
 * @param b a boolean value, True if App is Server, False if App is Client
 */
void App::setServer(bool b) {
	this->is_server = b;
}

// Server code
/*! \brief Returns a boolean indicating if the App is functioning as a server.
 *
 * @return true if App is functioning as server, otherwise false
 */
bool App::isServer() {
	return is_server;
}

// Server and client code
/*! \brief Returns the user name for app.
 *
 *  @return a string: username for app.
 */
std::string App::getUsername() {
    return username;
}

// Server and client code
/*! \brief Method to return ID of the user on the network.
 *
 * @return the ID of the user on the network, an int.
 */
int App::getID() {
    return appID;
}

// Server and Client code
/*! \brief Method to ask user for their name and update the "username" member variable.
 *
 */
void App::setUsername() {
    // collect username info
    std::cout << "Enter your name and hit enter: ";
    getline(std::cin, this->username);
}


// Server and Client code
/*! \brief Switch statement to extract incoming packet and follow given instructions
 *
 * @param packet a packet of data sent over the network, an sf::Packet
 */
void App::extractPacket(sf::Packet packet) {
    // extract packet based on first int value
    sf::Uint32 command_type;
    if ( packet >> command_type ) {
        switch (command_type) {
            case 1 : {
                sf::Uint32 appID, x, y;
                sf::Uint8 r, g, b, beforeR, beforeG, beforeB;
                if (packet >> appID >> x >> y >> r >> g >> b >> beforeR >> beforeG >> beforeB) {
                    sf::Color color(r, g, b);
                    sf::Color beforeColor(beforeR, beforeG, beforeB);
                    std::shared_ptr<Command> draw = std::make_shared<Draw>(*this, (int) x, (int) y, color,
                                                                           beforeColor);
                    // Server
                    if (this->isServer()) {

                        this->current_vectors.at(appID)->push_back(draw);
                        // Do not call execute() here because it sets up an infinite loop of packets
                        this->GetImage().setPixel((int) x, (int) y, color);
                        // Client
                    } else {
                        draw->execute();
                    }
                }
            }
            break;
            case 2 :
            {
                // if client: change appID num
                if (!this->isServer()) {
                    sf::Uint32 newID;
                    if (packet >> newID) {
                        this->setID((int)newID);
                    }
                }
            }
            break;
            case 3 :
            {
                // Incoming message
                std::string message;
                if (packet >> message) {
                    char* char_msg = new char();
                    strcpy(char_msg, message.c_str());
                    msg_vec->push_back(char_msg);
                }
            }
            break;
            case 4 :
            {
                // ClearRedo
                if (this->isServer()) {
                    this->ClearRedo();
                }
            }
            break;
            case 5 :
            {
                // AddToDeque command: get index (client ID)
                if (this->isServer()) {
                    sf::Uint32 index;
                    if (packet >> index) {
                        //deep copy current_vectors[index]
                        std::shared_ptr<std::vector<std::shared_ptr<Command>>> ptrVec = std::make_shared<std::vector<std::shared_ptr<Command>>>();
                        for (int i = 0; i < current_vectors.at(index)->size(); i++) {
                            ptrVec->push_back(current_vectors.at(index)->at(i));
                        }
                        // clear appropriate current vector
                        while (!current_vectors.at(index)->empty()) {
                            current_vectors.at(index)->pop_back();
                        }
                        m_commands.push_back(ptrVec);

                        if (m_commands.size() > 100) {
                            m_commands.front()->clear();
                            m_commands.pop_front();
                        }
                    }
                }
            }
            break;
            case 6 :
            {
                // Undo()
                if (this->isServer()) {
                    this->Undo();
                }
            }
            break;
            case 7 :
            {
                // Redo()
                if (this->isServer()) {
                    this->Redo();
                }
            }
            break;
            case 8 :
            {
                // send copy of the screen
                // collect appID to send connection
                sf::Uint32 appID;
                packet >> appID;

                // send current canvas by iterating over the vector of vectors
                std::shared_lock(this->m_mutex);
                for ( int i = 0; i < this->m_commands.size(); i++ ){
                    for ( int j = 0; j < this->m_commands[i]->size(); j++ ){
                        std::shared_ptr<Command> command_to_send = this->m_commands[i]->at(j);

                        bool success = command_to_send->send_as_packet(*this->socket_vector.at(appID));
                        if ( !success ) std::cout << "Error sending canvas to new user." << std::endl;
                    }
                }
            }
            break;
            case 9:{
                sf::Uint8 r, g, b;
                if(packet >> r >> g >> b){ //unpacking the r, g, b
                    sf::Color color(r,g,b);
                    this->GetImage().create(win_width, win_height, color); //clear their window to the color
                    if (this->isServer()) {
                        std::shared_ptr<Command> clear = std::make_shared<Clear>(*this, color);
                        std::shared_ptr<std::vector<std::shared_ptr<Command>>> vec = 
                            std::make_shared<std::vector<std::shared_ptr<Command>>>();
                        vec->push_back(clear);
                        m_commands.push_back(vec);
                    }
                }
            }
            break;
            case 10 : {
                // receiving message for terminal
                std::string msg;
                if (packet >> msg) {
                    std::cout << msg << std::endl;
                }
            }
            break;
            default :
                std::cout << "Error occured: Bad Packet!\n";
        }
    }
}

// Server code
/*! \brief Method to create the first current vector for use of the server.
 *
 */
void App::initializeServerID() {
    // create vector[0] for current_vectors
    std::shared_ptr<std::vector<std::shared_ptr<Command>>> ptrVec = std::make_shared<std::vector<std::shared_ptr<Command>>>();
    current_vectors.push_back(ptrVec);
}

// Server code
/*! \brief Method to initialize the listener for the server.
 *
 */
void App::setIPAddressCreateListener() {
    bool connected = false;

    while ( !connected ) {
        // Set the listener for the given port
        std::shared_lock(this->m_mutex);
        sf::Socket::Status status = this->listener.listen(this->port_number);
        if (status != sf::Socket::Done) {
            std::cerr << "Error occurred with connecting via given socket. Error code: "
                      << status << std::endl;

            // If the given port doesn't work, ask for it again
            this->port_number = get_port();

        } else {
            // add the listener to the socket selector
            this->socket_selector.add(this->listener);

            std::cout << "Users should connect using this IP address: " << sf::IpAddress::getLocalAddress()
                      << " on port: " << this->port_number << std::endl;
            connected = true;
        }
    }
}

/*! \brief Method to ask the user for the port number that they wish to use.
*
* @return the port number entered by the user, a long.
*/
long App::get_port() {
    bool done = false;
    std::string text_port;
    long port;

    while (!done) {
        std::cout << "Enter the port that you want to use for this session: ";
        getline(std::cin, text_port);
        std::stringstream conversion(text_port);
        conversion >> port;
        if ( port >= 0 ) done = true;
    }

    return port;
}

// Server code
/*! \brief Method to send a packet to all connected clients.
 *
 * @param outgoing_packet A reference to an sf::Packet
 */
void App::sendPacket(sf::Packet &outgoing_packet){
    // send it to each client in our socket_vector
    std::shared_lock(this->m_mutex);
    for (auto & i : this->socket_vector) {
        i->send(outgoing_packet);
    }
}

// Server code
/*! \brief Starts a thread so that the server can constantly
 *         listen for new incoming connections or packets
 *         of information.
 *
 */
void App::connectServer() {
    this->thread_vector.push_back(std::thread(&App::communicateServer, this));
}

/*! \brief Method used by the server to add a connection to the socket selector and vector list.
 *
 */
void App::add_connection(){
    std::shared_ptr<sf::TcpSocket> new_connection = std::make_shared<sf::TcpSocket>();
    if (listener.accept(*new_connection) == sf::Socket::Done ) {
        // Add the new new_connection to the clients list
        std::shared_lock(this->m_mutex);
        // send appID to new client
        sf::Packet packet;
        packet << 2 << (sf::Uint32)newClientID;
        // add vector pointer to current_vectors
        std::shared_ptr<std::vector<std::shared_ptr<Command>>> client_vector =
                std::make_shared<std::vector<std::shared_ptr<Command>>>();
        this->current_vectors.push_back(client_vector);
        newClientID++;
        new_connection->send(packet);
        this->socket_vector.push_back(new_connection);
        // Add the new new_connection to the selector so that we will
        // be notified when he sends something
        this->socket_selector.add(*new_connection);
    } else {
        // Error, we won't get a new connection
        // Socket will auto delete because of smart pointer
    }
}

/*! \brief Method used by server to receive packets of data
 *         from all connected clients.
 */
void App::server_receive_packet(){
    std::shared_lock(this->m_mutex);
    // iterate through the sockets to find the one that received data
    for (int i = 0; i < this->socket_vector.size(); i++) {
        sf::TcpSocket &ready_socket = *this->socket_vector[i];
        if ( this->socket_selector.isReady(ready_socket) ) {

            sf::Packet packet;
            // if the socket received data
            if ( ready_socket.receive(packet) == sf::Socket::Done ) {
                // extract packet
                this->extractPacket(packet);

                // send packet to everyone except the current sender
                for (int j = 0; j < this->socket_vector.size(); j++) {
                    if (this->socket_vector[j] != this->socket_vector[i]) {
                        this->socket_vector[j]->send(packet);
                    }
                }
            }
        }
    }
}

// Server code
/*! \brief Method that uses the socket_selector to listen for either
 *         new connections or incoming packets
 *
 */
void App::communicateServer() {
    while ( true ) {
        // Make the selector wait for data on any socket
        if ( this->socket_selector.wait() ) {
            // If the socket_selector is saying it is ready, first check to see if it is trying to add a new
            // connection. If so, attempt to add the connection
            if ( this->socket_selector.isReady(this->listener) ) {
                // The listener is ready: there is a pending connection
                this->add_connection();
            }
                // if it wasn't a new connection attempt, data must have been received on one of the sockets
            else{
                this->server_receive_packet();
            }
        }
    }
}


// Server code
/*! \brief Method to establish if server is not connected to clients.
 *  Returns false if there is a connection, true if all connections are void.
 * @return bool, false if there is a connection, true is all connections are void.
 */
bool App::serverSocketClosed() {
    // check all in socket_vector
    for (int i = 0; i < (int)socket_vector.size(); i++) {
        if (socket_vector[i]->getRemotePort() != 0) {
            return false;
        }
    }
    return true;
}

// Server code
/*! \brief Adds a single command to server's current vector, located at position
 *  0 pf current_vectors.
 * @param draw represents any command to be added to server's current vector.
 */
void App::addCommandToServer(std::shared_ptr<Command> draw) {
    current_vectors.at(0)->push_back(draw);
}

// Client code here
/*! \brief Method to collect an IP address from the user.
 *
 * @return an IP address from the user, a string
 */
std::string App::get_ip(){
    bool done = false;
    std::string ip;
    while (!done) {
        std::cout << "Enter the host IP address for this session: ";
        getline(std::cin, this->ip_address);
        if (!ip.empty()) done = true;
    }
    return ip;
}

// Client code
/*! \brief Method used by client to connect to a paint session.
 *
 */
void App::connect() {
    // the entire connect block is in a loop
    // if a connection cannot be made, the user is asked to re-enter their information
    bool connected = false;
    while ( !connected ){
        // Try and connect
        sf::Socket::Status status = this->client_socket.connect(this->ip_address, this->port_number);
        if ( status != sf::Socket::Done ) {
            std::cerr << "Error occurred with connecting to the server. Error code: "
                      << status << std::endl;
            std::cout << "Let's try entering your connection details again." << std::endl;

            // ask for ip and port again
            this->ip_address = this->get_ip();
            this->port_number = this->get_port();
        } else {
            std::string welcome;
            welcome.append(this->username);
            welcome.append(" has joined the session!");
            // send a message to the terminal (packet number 10)
            sf::Packet packet;
            packet << 10 << welcome;
            this->client_socket.send(packet);
            std::cout << welcome << std::endl;
            connected = true;
        }
    }
}

// Client code
/*! \brief method that starts a thread so that messages can be received
 *         in the background when the App is acting as client
 *
 */
void App::communicate(){
    this->thread_vector.push_back(std::thread (&App::receivePacket, this));

}

// Client code
/*! \brief method that sends a packet to the server.
 *
 * @param packet a packet (sf::Packet) to be sent to the server
 */
void App::sendPacketFromClient(sf::Packet packet) {
    std::shared_lock(this->m_mutex);
    this->client_socket.send(packet);
}

// Client code
/*! \brief Method to listen for incoming packets from the server.
 *         Before entering the while loop, it first asks for a copy
 *         of the screen.
 *
 */
void App::receivePacket(){

   bool screen_requested = false;

    while ( true ) {

        sf::Packet packet;
        // Wait for a packet to be received
        sf::Socket::Status status = this->client_socket.receive(packet);
        if ( status != sf::Socket::Done ) {
            std::cerr << "Error occurred with receiving data. The host as disconnected.";
            break;
        }

        // Extract packet information and execute
        this->extractPacket(packet);

        // Ask for a copy of the screen
        if ( !screen_requested ){
            sf::Packet ask_for_screen;
            ask_for_screen << 8 << (this->getID() - 1);
            this->client_socket.send(ask_for_screen);
            screen_requested = !screen_requested;
        }
    }
}

// Client code
/*! \brief Method to check if connection to client is closed.
 *
 * @return true, if the connection is closed, otherwise false
 */
bool App::clientSocketClosed() {
    // check to see if client_socket is closed
    if (this->client_socket.getRemotePort() == 0) {
        return true;
    }
    return false;
}

//Client code
/*! \brief Method to set the ID of the client.
 *
 * @param newID the new ID, an int.
 */
void App::setID(int newID) {
    appID = newID;
}


/*! \brief 	Initializes the App and sets up the main
*		rendering window(i.e. our canvas) and main GUI window.
*/
void App::Init(std::function<void(App& app)> initFunction){
	// Create our window
	m_window.create(sf::VideoMode(win_width, win_height),"DATOU Paint", sf::Style::Titlebar);
	m_window.setVerticalSyncEnabled(true);
    m_window.setActive(true);
	// Create an image which stores the pixels we will update
	m_image.create(win_width,win_height,sf::Color::White);
	// Create a texture which lives in the GPU and will render our image
	m_texture.loadFromImage(m_image);
	// Create a sprite which is the entity that can be textured
	m_sprite.setTexture(m_texture);
    
    /*  GUI
        We are creating a GUI context, and it needs
        to attach to our 'window'.
    */
	// Create a GUI window to draw to
	gui_window.create(sf::VideoMode(700,win_height), "DATOU Paint Settings",sf::Style::Titlebar);
	gui_window.setVerticalSyncEnabled(true);
    gui_window.setActive(true);
    glViewport(0, 0, gui_window.getSize().x, gui_window.getSize().y);
    	/* from openGL library, this function specifies the affine transformation of x and y from normalized device
	 * coordinates to window coordinates. It determines the portion of the window to which OpenGL is drawing to.
	 * parameters: x,y which are lower left corner, then  width, height.
	 */ 
	
    ctx = new nk_context();
    ctx = nk_sfml_init(&gui_window); //pass by reference 
    // Load Fonts: if none of these are loaded a default font will be used
    //Load Cursor: if you uncomment cursor loading please hide the cursor
    struct nk_font_atlas *atlas;
    nk_sfml_font_stash_begin(&atlas);
    nk_sfml_font_stash_end();
    set_style(ctx, THEME_RED);

    // Setup a color for the nuklear gui
    bg = new nk_colorf();
    bg->r = 0.10f, bg->g = 0.18f, bg->b = 0.24f, bg->a = 1.0f;

	// Set our initialization function to perform any user
	// initialization
	m_initFunc = initFunction;
    
}

/*! \brief 	Set a callback function which will be called
		each iteration of the main loop before drawing.
*		
*/
void App::UpdateCallback(std::function<void(App& app)> updateFunction){
	m_updateFunc = updateFunction;
}

/*! \brief 	Set a callback function which will be called
		each iteration of the main loop after update.
*		
*/
void App::DrawCallback(std::function<void(App& app)> drawFunction){
	m_drawFunc = drawFunction;
}

/*! \brief 	The main loop function which handles initialization
		and will be executed until the main window is closed.
		Within the loop function the update and draw callback
		functions will be called.
*		
*/
void App::Loop(){
	// Call the init function
	m_initFunc(*this);

	// Start the main rendering loop
	while(m_window.isOpen() && gui_window.isOpen()){
        // OpenGL is the background rendering engine,
        // so we are going to clear our GUI graphics system.
        gui_window.setActive(true);
        gui_window.clear();
        glClearColor(bg->r, bg->g, bg->b, bg->a);
        glClear(GL_COLOR_BUFFER_BIT);
        nk_sfml_render(NK_ANTI_ALIASING_ON);
        gui_window.display();

		// Clear the window
        m_window.setActive(true);
		m_window.clear();
		// Updates specified by the user
		m_updateFunc(*this);
		// Additional drawing specified by user
		m_drawFunc(*this);
		// Update the texture
		// Note: This can be done in the 'draw call'
		// Draw to the canvas	
		m_window.draw(m_sprite);
		// Display the canvas
		m_window.display();
	}
}

// Function to render our GUI
/*! \brief drawLayout: a method to render our GUI and manage GUI button presses.
 *
 *  @param ctx1 a reference to an nk_context struct object.
 *  @param bg1 a reference to an nk_colorf struct object.
 *  @param msgs a smartpointer reference to a 'char*' vector.
 */
void App::drawLayout(struct nk_context* ctx1, struct nk_colorf* bg1, std::shared_ptr<std::vector<char*>> msgs){ 
	//ctx1 is the main nk context, need to be passed into all functions

    /* GUI */
	//this is initial window position and window size 50,50,230,250, first two numbers are positoins, last numbers are the dimension   
    if (nk_begin(ctx1, "DATOU Paint", nk_rect(0, 0, 400, 800), 
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE))//| NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
	//nk_begin, returns the first draw command in the context draw command list to be drawn 
    	//nk_begin, and nk_end specifices the persistent 'window' in nk. 
	//any function needs to be inserted in between the function pair 
    	//nk_begin needs to be called every frame, for every window, otherwise, window gets removed 

	//NK_WINDOW are all flags, e.g. nk_window_border draws the border around it 
	//movable: window is movale 
	//scalable: can be scaled 
	//title: forces a header at the top to show a title 

    { //this marks the start of the widgets 
        static int property = 20;
	/* layouting function, provides each widget with same horizontal pixel width inside the row and 
	 * does not grow if the owning window scales smallers or bigger
	 * (30,80,2) means first row with height 30, composed of two widgets with width 80 
	 */
     
    //special features, redo, undo and clear  
	nk_layout_row_dynamic(ctx1, 30, 1);
        nk_label(ctx1, "Features:", NK_TEXT_LEFT); 

	nk_layout_row_begin(ctx1, NK_STATIC, 30, 6);
	nk_layout_row_push(ctx, 10);
	nk_label(ctx1,"    ", NK_TEXT_LEFT);

	nk_layout_row_push(ctx1, 100);
	if (nk_button_label(ctx1, "UNDO (Z)")) {
		//std::cout<<"Undo button pressed";
        this->Undo();
    	}
	nk_layout_row_push(ctx, 10);
	nk_label(ctx1,"    ", NK_TEXT_LEFT);
	nk_layout_row_push(ctx1, 100);
	if (nk_button_label(ctx1, "REDO (Y)")) {
		//std::cout<<"Redo button pressed";
        this->Redo();
    	} 
    	
    nk_layout_row_push(ctx, 10);
	nk_label(ctx1,"    ", NK_TEXT_LEFT);
	nk_layout_row_push(ctx1, 120);
	if (nk_button_label(ctx1, "CLEAR (SPACE)")) {
		//std::cout<<"Clear button pressed";
        std::shared_ptr<Command> blank = std::make_shared<Clear>(*this, this->getCurrentColor());
        if (this->isServer()) {
            // App is server

            if (this->serverSocketClosed()) {
                // server not connected to any clients
                std::shared_ptr<std::vector<std::shared_ptr<Command>>> vec = 
                    std::make_shared<std::vector<std::shared_ptr<Command>>>();
                vec->push_back(blank);
                this->m_commands.push_back(vec);
                blank->execute();
            }
            else {
                // server has clients
                
                std::shared_ptr<std::vector<std::shared_ptr<Command>>> vec = 
                    std::make_shared<std::vector<std::shared_ptr<Command>>>();
                vec->push_back(blank);
                this->m_commands.push_back(vec);
                blank->execute();
            }
        }
        else {

            // App is client
            if (this->clientSocketClosed()) {
                // Client is not connected
                std::shared_ptr<std::vector<std::shared_ptr<Command>>> vec = 
                    std::make_shared<std::vector<std::shared_ptr<Command>>>();
                vec->push_back(blank);
                this->m_commands.push_back(vec);
                blank->execute();
            }
            else {
                // Client connected
                sf::Packet packet;
                packet << (sf::Uint32)9 << this->getCurrentColor().r << this->getCurrentColor().g <<
                    this->getCurrentColor().b;
                this->sendPacketFromClient(packet);
                blank->execute();
            }
        }
    	
	}

    //brush size
	nk_layout_row_dynamic(ctx1, 30, 1);
    nk_label(ctx1, "Brush Size:", NK_TEXT_LEFT);
	nk_layout_row_dynamic(ctx1, 30, 3);
	if (nk_option_label(ctx1, "Small", bs == SMALL)){
		bs = SMALL;
        this->setRadius(1);
	}
	if (nk_option_label(ctx1, "Medium", bs == MEDIUM)){
		bs = MEDIUM;
        this->setRadius(4);
	}
	if (nk_option_label(ctx1, "Large", bs == LARGE)){
		bs = LARGE;
        this->setRadius(8);
	}

	//spacing 
	nk_layout_row_dynamic(ctx1, 30, 1);
    nk_label(ctx1, "          ", NK_TEXT_LEFT);

	nk_layout_row_dynamic(ctx1, 30, 1);
    nk_label(ctx1, "Paint Color:", NK_TEXT_LEFT);

    nk_layout_row_static(ctx1, 30, 62, 2); //layouting, dynamically grows if the window rescales 
        //row height is 30, and 2 widgets per row. 

	//option labels, or clickable circles (use of enum global variable from the beginning)
	if (nk_option_label(ctx1, "Red", op == RED)){
            op = RED;
            this->setCurrentColor(sf::Color::Red);
    }

    struct nk_command_buffer*canvas1=nk_window_get_canvas(ctx1);
    struct nk_rect space1;
    nk_widget(&space1,ctx1);
    nk_fill_rect(canvas1, space1, 2, nk_rgb(255,0,0));    
    
    if (nk_option_label(ctx1, "Black", op == BLACK)){
            op = BLACK;
            this->setCurrentColor(sf::Color::Black);
        } 
    struct nk_command_buffer*canvas2=nk_window_get_canvas(ctx1);
    struct nk_rect space2;
    nk_widget(&space2,ctx1);
    nk_fill_rect(canvas2, space2, 2, nk_rgb(0,0,0));
    
    if (nk_option_label(ctx1, "Green", op == GREEN)){
        op = GREEN;
        this->setCurrentColor(sf::Color::Green);
    }

    struct nk_command_buffer*canvas3=nk_window_get_canvas(ctx1);
    struct nk_rect space3;
    nk_widget(&space3,ctx1);
    nk_fill_rect(canvas3, space3, 2, nk_rgb(0,128,0)); 
       
	if (nk_option_label(ctx1, "Blue", op == BLUE)){
		op = BLUE;
        this->setCurrentColor(sf::Color::Blue);
	}

	struct nk_command_buffer*canvas4=nk_window_get_canvas(ctx1);
    struct nk_rect space4;
    nk_widget(&space4,ctx1);
    nk_fill_rect(canvas4, space4, 2, nk_rgb(0,0,255));
    
	if (nk_option_label(ctx1, "White", op == WHITE)){
		op = WHITE;
        this->setCurrentColor(sf::Color::White);
	}

	struct nk_command_buffer*canvas5=nk_window_get_canvas(ctx1);
    struct nk_rect space5;
    nk_widget(&space5,ctx1);
    nk_fill_rect(canvas5, space5, 2, nk_rgb(255,255,255));
    
	if (nk_option_label(ctx1, "Yellow", op == YELLOW)){
		op = YELLOW;
        this->setCurrentColor(sf::Color::Yellow);
	}

	struct nk_command_buffer*canvas6=nk_window_get_canvas(ctx1);
    struct nk_rect space6;
    nk_widget(&space6,ctx1);
    nk_fill_rect(canvas6, space6, 2, nk_rgb(255,255,0));
    
	if (nk_option_label(ctx1, "Magenta", op == MAGENTA)){
		op = MAGENTA;
        this->setCurrentColor(sf::Color::Magenta);
	}

	struct nk_command_buffer*canvas7=nk_window_get_canvas(ctx1);
    struct nk_rect space7;
    nk_widget(&space7,ctx1);
    nk_fill_rect(canvas7, space7, 2, nk_rgb(255,0,255));
    
	if (nk_option_label(ctx1, "Cyan", op == CYAN)){
		op = CYAN;
        this->setCurrentColor(sf::Color::Cyan);
	}
	struct nk_command_buffer*canvas8=nk_window_get_canvas(ctx1);
    struct nk_rect space8;
    nk_widget(&space8,ctx1);
    nk_fill_rect(canvas8, space8, 2, nk_rgb(0,255,255));

	/* property_int modifies a passed in value 
	 * after the name parameter, you have min, *int, max, step, inc_per_pixel 
	 * so min=0, property is an integer, max=100, step-10, inc per pixel is values per pixel added or subtracted on dragging
	 */

	//spacing 
	nk_layout_row_dynamic(ctx1, 30, 1);
    nk_label(ctx1, "          ", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx1, 20, 1);
    nk_label(ctx1, "Background:", NK_TEXT_LEFT); //force the word "background" to the leftside
    nk_layout_row_dynamic(ctx1, 25, 1);

	/* combo begin color starts a combo block with color as displayed item
	 * parameters: context, panel, color, and max height.
	 * combo block just means you can click on the arrow to expand the menu  
	 */
    if (nk_combo_begin_color(ctx1, nk_rgb_cf(*bg1), nk_vec2(nk_widget_width(ctx),400))) {
        nk_layout_row_dynamic(ctx1, 120, 1);
        *bg1 = nk_color_picker(ctx1, *bg1, NK_RGBA);//creats color picker widge with COLOR=bg as initially picked color
        nk_layout_row_dynamic(ctx1, 25, 1);
	    //propertyf returns the modified float value 
	    //context, name, min, *val, max, step, inc per pixel 
        bg1->r = nk_propertyf(ctx1, "#R:", 0, bg1->r, 1.0f, 0.01f,0.005f);
        bg1->g = nk_propertyf(ctx1, "#G:", 0, bg1->g, 1.0f, 0.01f,0.005f);
        bg1->b = nk_propertyf(ctx1, "#B:", 0, bg1->b, 1.0f, 0.01f,0.005f);
        bg1->a = nk_propertyf(ctx1, "#A:", 0, bg1->a, 1.0f, 0.01f,0.005f);
        nk_combo_end(ctx1);
    }//end of combo block

	//spacing 
	nk_layout_row_dynamic(ctx1, 30, 1);
    nk_label(ctx1, "          ", NK_TEXT_LEFT);
    }
    nk_end(ctx1); //end of 'window' 

    //second window (chat history window)
    if (nk_begin(ctx1, "Chat History", nk_rect(400, 0, 300, 300), 
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE)){
    	nk_layout_row_dynamic(ctx1, 20, 1);
        nk_label(ctx1, "Chat:", NK_TEXT_LEFT);

        for (int i = 0; i < msgs->size(); i++) {
            nk_layout_row_dynamic(ctx1, 30, 1);
            nk_label(ctx1, msgs->at(i),NK_TEXT_LEFT);
        }
    }
    nk_end(ctx1); //end of 'window' 

    //third window (chat window) 
    if (nk_begin(ctx1, "Chat Window", nk_rect(400, 300, 300, 500), 
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE)){ 
        static int buf_len;
        nk_layout_row_dynamic(ctx1,30,1);
        nk_edit_string(ctx1,NK_EDIT_FIELD,buf,&buf_len,4096,
        nk_filter_default);
        nk_layout_row_static(ctx1,30,80,1);
        if (nk_button_label(ctx1, "Send")) {
            // event handling
            std::string new_message;
            new_message += this->getUsername();
            new_message += ": ";

            for (int i = 0; i < buf_len; i++) {
                new_message += buf[i];
            }
            
            // create new char*
            char* char_msg = new char();
            strcpy(char_msg, new_message.c_str());
            
            // create a packet
            sf::Packet packet;
            packet << (sf::Uint32)3 << new_message;
            if (this->isServer()) {
                if (!this->serverSocketClosed()) {
                    // Server connected
                    this->msg_vec->push_back(char_msg);
                    this->sendPacket(packet);
                }
                else {
                    // Server not connected
                    this->msg_vec->push_back(char_msg);
                }
            }
            else {
                if (!this->clientSocketClosed()) {
                    // Client connected
                    this->msg_vec->push_back(char_msg);
                    this->sendPacketFromClient(packet);
                }
                else {
                    // Client not connected
                    this->msg_vec->push_back(char_msg);
                }
            }
        }
        nk_end(ctx1); //end of 'window'
    }
}

#endif
