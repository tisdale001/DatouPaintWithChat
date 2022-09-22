
#ifndef COMMAND_CPP
#define COMMAND_CPP
/** 
 *  @file   Command.cpp 
 *  @brief  Implementation of Command.hpp
 *  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
 *  @date   November 2021
 ***********************************************/

// Include our Third-Party SFML header

// Include standard library C++ libraries.

// Project header files
#include <App.hpp>
#include <Command.hpp>

/*! \brief 	Constructor for the Command class. Attributes were added
*		to get the information needed to create an executable and undoable command.
*		
*/
Command::Command(App& app) : m_app(app) {}

#endif