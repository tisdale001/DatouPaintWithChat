#ifndef PAINT_HPP
#define PAINT_HPP

/**
*  @file   Paint.hpp
*  @brief  Paint actions interface.
*  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
*  @date   November 2021
***********************************************/

// Include our Third-Party SFML header
#include <SFML/Graphics/Color.hpp>

// Include standard library C++ libraries.
#include<vector>
#include <App.hpp>
#include <Command.hpp>

class App;

// The Paint class
class Paint : public Command {

private:
	int x;
	int y;
    int radius;
	sf::Color color;
    std::vector<std::shared_ptr<Command>> drawVec;

    // a private method used in creating a vector or draw commands
    void createDrawVec();

public:
	// Note that the Paint command also takes in the radius of the circle that
	// it paints
	Paint(App& app_, int x_, int y_, int radius_, sf::Color color_);
	
	// Destructor for a command
	~Paint();

	// Returns true or false if the command was able to successfully
	// execute.
	bool execute() override;
	bool undo() override;
    bool send_as_packet(sf::TcpSocket &socket) override;
};
#endif