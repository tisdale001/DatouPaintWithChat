

#ifndef APP_CLEAR_HPP
#define APP_CLEAR_HPP

#include<vector>
#include <App.hpp>
#include <Command.hpp>


class App;
//class Command;

// The Clear class
class Clear : public Command {

private:
sf::Color color;



public:

Clear(App& app_,sf::Color color_);

// Destructor for a command
~Clear();

// Returns true or false if the command was able to successfully
// execute.
bool execute() override;
bool undo() override;
bool send_as_packet(sf::TcpSocket &socket) override;
};

#endif //APP_CLEAR_HPP
