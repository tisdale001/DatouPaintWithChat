#ifndef COMMAND_HPP
#define COMMAND_HPP

/**
 *  @file   Command.hpp
 *  @brief  Represents an actionable command by the user.
 *  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
 *  @date   November 2021
 ***********************************************/

// Include our Third-Party SFML header
#include <SFML/Graphics/Color.hpp>

// Include standard library C++ libraries.

// Include project header files
#include <App.hpp>

class App;

// The command class
class Command{
protected:
    App& m_app;

private:

public:
	virtual bool execute() = 0;
	virtual bool undo() = 0;
    virtual bool send_as_packet(sf::TcpSocket &socket) = 0;
    explicit Command(App& app);
	};

#endif