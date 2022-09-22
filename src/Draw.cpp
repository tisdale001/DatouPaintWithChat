#ifndef Draw_CPP
#define Draw_CPP

/**
 *  @file   Command.cpp
 *  @brief  Implementation of Command.hpp
 *  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
 *  @date   November 2021
 ***********************************************/

// Include our Third-Party SFML header
#include <SFML/Graphics.hpp>

// Include standard library C++ libraries.

// Project header files
#include <Draw.hpp>
#include <App.hpp>
#include <Command.hpp>

/*! \brief 	Constructor for the Draw class. Attributes were added
*		to get the information needed to create an executable and undoable command.
*   @param app a reference to App class.
*   @param x an int: representing an x coordinate.
*   @param y an int: representing a y coordinate.
*   @param color an sf::Color, the color we're changing the pixel to.
*   @param beforeColor an sf::Color, the previous color of the pixel (for undo function).
*/
Draw::Draw(App& app, int x, int y, sf::Color color, sf::Color beforeColor) :
    Command(app),
    x_(x),
    y_(y),
    color_(color),
    beforeColor_(beforeColor)
{}

/*! \brief 	Deconstructor for the Draw class.
*		
*/
Draw::~Draw()= default;

/*! \brief	This is the execute() method of the Command pattern.
*		In this case it changes one pixel to the desired color.
*/
bool Draw::execute(){
    if (m_app.isServer()) {
        if (!m_app.serverSocketClosed()) {
            // create and send packet
            sf::Packet packet;
            packet << 1 << (sf::Uint32)m_app.getID() << (sf::Uint32)this->x_ << 
            (sf::Uint32)this->y_ << this->color_.r <<
                this->color_.g << this->color_.b << 
                this->beforeColor_.r << this->beforeColor_.g << this->beforeColor_.b;
            m_app.sendPacket(packet);
        }
    }
	m_app.GetImage().setPixel(this->x_, this->y_, this->color_);
    
    return m_app.GetImage().getPixel(this->x_, this->y_) == this->color_;
}

/*!	\brief	This is the undo() method of the Command pattern.
*		In this case it changes one pixel to the previous color.
*/
bool Draw::undo(){
	
    if (m_app.isServer()) {
        // Server
        if (!m_app.serverSocketClosed()) {
            sf::Packet packet;
            packet << 1 << (sf::Uint32)m_app.getID() << (sf::Uint32)x_ << (sf::Uint32)y_ << this->beforeColor_.r <<
                this->beforeColor_.g << this->beforeColor_.b << m_app.GetImage().getPixel(x_, y_).r <<
                m_app.GetImage().getPixel(x_, y_).g << m_app.GetImage().getPixel(x_, y_).b;
            m_app.sendPacket(packet);
        }
    }

    m_app.GetImage().setPixel(this->x_, this->y_, this->beforeColor_);

    return m_app.GetImage().getPixel(this->x_, this->y_) == this->beforeColor_;
}

/*! \brief This method sends the command as a packet.
 *
 * @return true if succesfull, otherwise false.
 */
bool Draw::send_as_packet(sf::TcpSocket &socket) {
    if (!m_app.serverSocketClosed()) {
        sf::Packet packet;
        packet << 1 << (sf::Uint32)m_app.getID() << (sf::Uint32)this->x_ <<
               (sf::Uint32)this->y_ << this->color_.r <<
               this->color_.g << this->color_.b <<
               this->beforeColor_.r << this->beforeColor_.g << this->beforeColor_.b;
        socket.send(packet);
    } else {
        // if the socket is closed, return false
        return false;
    }
    // otherwise, it would have been succesful
    return true;
}

#endif