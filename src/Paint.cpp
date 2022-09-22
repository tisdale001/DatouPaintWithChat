#ifndef PAINT_CPP
#define PAINT_CPP

/**
 *  @file   Paint.cpp
 *  @brief  Implementation of Paint.hpp
 *  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
 *  @date   November 2021
 ***********************************************/

// Include our Third-Party SFML header
#include <SFML/Graphics/Color.hpp>

// Include standard library C++ libraries.
#include <cmath>

// Include project header files
#include <Paint.hpp>
#include <App.hpp>
#include <Draw.hpp>

/*! \brief 	Constructor for the Paint class. Attributes were added
*		to get the information needed to create an executable and undoable command.
*   @param app a reference to App class.
*   @param x_ an int: represents an x coordinate.
*   @param y_ an int: represents a y coordinate.
*   @param radius_ an int: radius of circle.
*   @param color_ an sf::Color: color to be painted.
*/
Paint::Paint(App& app, int x_, int y_, int radius_, sf::Color color_) :
    Command(app),
    x(x_),
    y(y_),
    radius(radius_),
    color(color_)
{
    createDrawVec();
}
	
/*! \brief Deconstructor for the Paint class.
*
*/
Paint::~Paint() {
    drawVec.clear();
}

/*! \brief	This is the execute() method of the Command pattern.
*		In this case it changes all pixels within a circle to desired color.
*/
bool Paint::execute() {
    // iterate over drawVec and execute each Draw command
    for (auto item: drawVec) {
        item->execute();
    }
    return true;
}

/*!	\brief	This is the undo() method of the Command pattern.
*		In this case it changes all pixels to previous color.
*/
bool Paint::undo() {
    // iterate over drawVec and undo each Draw command
    for (auto item: drawVec) {
        item->undo();
    }
    return true;
}

/*! \brief This method creates a vector of all the 'Draw' commands necessary
*   to render a circle on the app window.
*
*/
void Paint::createDrawVec() {
    // within possible square determined by radius
    // create Draw command for each pixel within circle
    for (int i = x - radius; i <= x + radius; i++) {
        for (int j = y - radius; j <= y + radius; j++) {
            if (i >= 0 && i <= m_app.GetImage().getSize().x && j >= 0 && j <= m_app.GetImage().getSize().y) {
                if ((pow(i - x, 2) + pow(j - y, 2)) <= pow(radius, 2)) {
                    // If Client create and send packet
                    if (!m_app.isServer()) {
                        if (!m_app.clientSocketClosed()) {
                            sf::Packet packet;
                            packet << (sf::Uint32)1 << (sf::Uint32)m_app.getID() << (sf::Uint32)i << (sf::Uint32)j <<
                                color.r << color.g << color.b << m_app.GetImage().getPixel(i, j).r << 
                                m_app.GetImage().getPixel(i, j).g << m_app.GetImage().getPixel(i, j).b;
                            m_app.sendPacketFromClient(packet);
                        }
                    }
                    // Create draw command for each pixel in circle
                    std::shared_ptr<Command> draw = std::make_shared<Draw>(m_app, i, j, color, m_app.GetImage().getPixel(i, j));
                    drawVec.push_back(draw);
                }
            }
        }
    }
}

/*! \brief This method sends the command as a packet.
 *
 * @return true if succesfull, otherwise false.
 */
bool Paint::send_as_packet(sf::TcpSocket &socket) {
    // iterate over drawVec and send each draw command
    for (auto item: drawVec) {
        item->send_as_packet(socket);
    }
    // if we got here, it was able to send everything
    return true;
}

#endif