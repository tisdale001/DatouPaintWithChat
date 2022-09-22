#ifndef DRAW_HPP
#define DRAW_HPP

/**
 *  @file   Draw.hpp
 *  @brief  Drawing actions interface.
 *  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
 *  @date   November 2021
 ***********************************************/

// Include our Third-Party SFML header
#include <SFML/Graphics/Color.hpp>

// Include standard library C++ libraries.

// Project header files
#include "Command.hpp"


class Draw : public Command {
private:
	int x_;
	int y_;
	sf::Color color_;
	sf::Color beforeColor_;

public:
    // Constructor
	Draw(App& app, int x, int y, sf::Color color, sf::Color beforeColor);
	
	// Destructor for a command
	~Draw();

	// Returns true or false if the command was able to successfully
	// execute.
	bool execute() override;
	bool undo() override;
    bool send_as_packet(sf::TcpSocket &socket) override;
};

#endif