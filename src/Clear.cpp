#ifndef CLEAR_CPP
#define CLEAR_CPP

// Include project header files
#include <App.hpp>
#include <Clear.hpp>
#include <Command.hpp>


/*! \brief Constructor for Clear class.
 *
 *  @param app a reference to App class.
 *  @param color_ an sf::Color to blank the screen to.
 */
Clear::Clear(App& app,sf::Color color_) :
    Command(app),
    color(color_)
{}


/*! \brief Deconstructor for Clear class.
 *
 */
Clear::~Clear()= default;

/*! \brief a method to execute the clear screen function.
 *
 *  @return a bool: true if execution was successful.
 */
bool Clear::execute() {
    if(m_app.isServer()){ //if server is connected
        if(!m_app.serverSocketClosed()){
            sf::Packet findClearPacket;
            findClearPacket << (sf::Uint32)9 << color.r << color.g << color.b;
            m_app.sendPacket(findClearPacket); //send the packet to everyone
        }
    }
    m_app.GetImage().create(m_app.GetImage().getSize().x, m_app.GetImage().getSize().y, color);

    return true;
}

/*! \brief A method to undo clearing the screen to one color. Undo() instructs the app
 *  to redraw the entire drawing/command history.
 *  @return a bool: true if undo completed successfully.
 */
bool Clear::undo() {
    if(m_app.isServer()){
        if(!m_app.serverSocketClosed()){
            //blank the screen to white
            m_app.GetImage().create(m_app.GetImage().getSize().x, m_app.GetImage().getSize().y, sf::Color::White);
            //tell all the clients to blank their screens to white (send a packet)
            sf::Packet tellClientBlankScreen;
            tellClientBlankScreen << (sf::Uint32)9 << sf::Color::White.r << sf::Color::White.g << sf::Color::White.b;
            m_app.sendPacket(tellClientBlankScreen);
            // Execute everything in m_commands except last Clear command :
            for(int i = 0; i < m_app.m_commands.size() - 1; i++){
                for(int j = 0; j < m_app.m_commands.at(i)->size(); j++){
                    m_app.m_commands.at(i)->at(j)->execute();
                }
            }
        } else {
            // server not connected to clients
            //blank the screen to white
            m_app.GetImage().create(m_app.GetImage().getSize().x, m_app.GetImage().getSize().y, sf::Color::White);
            // Execute everything in m_commands except last Clear command :
            for(int i = 0; i < m_app.m_commands.size() - 1; i++){
                for(int j = 0; j < m_app.m_commands.at(i)->size(); j++){
                    m_app.m_commands.at(i)->at(j)->execute();
                }
            }
        }
    } else {
        //client
        // first clear screen to white
        m_app.GetImage().create(m_app.GetImage().getSize().x, m_app.GetImage().getSize().y, sf::Color::White);
        // Execute everything in m_commands except last Clear command
        for(int i = 0; i < m_app.m_commands.size() - 1; i++){
            for(int j = 0; j < m_app.m_commands.at(i)->size(); j++){
                m_app.m_commands.at(i)->at(j)->execute();
            }
        }

    }
    return true;
}

/*! \brief This method sends the command as a packet.
 *
 * @return true if succesfull, otherwise false.
 */
bool Clear::send_as_packet(sf::TcpSocket &socket) {
    sf::Packet findClearPacket;
    findClearPacket << (sf::Uint32)9 << color.r << color.g << color.b;
    socket.send(findClearPacket);
    return true;
}

#endif
