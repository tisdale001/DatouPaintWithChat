/** 
 *  @file   main.cpp 
 *  @brief  Entry point into the program.
 *  @author Mike and ???? 
 *  @date   yyyy-dd-mm 
 ***********************************************/
#ifndef MAIN_CPP
#define MAIN_CPP

// Compile: Use a CMakeLists.txt to generate a build file or you can try compiling with:
// g++ -std=c++17 ./src/*.cpp -I./include/ -o App -lsfml-graphics -lsfml-window -lsfml-system
//
// Note:	If your compiler does not support -std=c++17, 
//		    then try -std=c++14 then -std=c++11.
//		
// HOW TO RUN
//
// ./App

// Include our Third-Party SFML header
#include <SFML/Graphics/Image.hpp>
#include <SFML/Network.hpp>
#include <SFML/OpenGL.hpp>

// NUKLEAR - for our GUI

#include "nuklear.h"
#include "nuklear_sfml_gl2.h"
#include "../nuklear/demo/style.c"

// Include standard library C++ libraries.
#include <string>

// Project header files
#include <App.hpp>
#include <Command.hpp>
#include <Draw.hpp>
#include <Paint.hpp>
#include <Clear.hpp>
#include <App_Factory.h>

/*! \brief 	This method sets up a server or client to begin communicating.
*
*	@param app a reference to an instance of App class.		
*/
void initialization(App& app){
	if (app.isServer()) {
		// server code here
		app.initializeServerID();
		app.setIPAddressCreateListener();
		app.connectServer();
	}
	else {
		// Client code here
		app.connect();
		app.communicate();
	}
}

/*! \brief 	In the update method I used two different
*		methods for handling mouse events and keyboard
*		presses. The sf::Event event was used when I only
*		wanted to register the event once.
*/
void update(App& app){
	// Update our canvas
	sf::Event event;
	while(app.GetWindow().pollEvent(event)){
		if(event.type == sf::Event::Closed){
			app.GetWindow().close();
		}

		// Capture any keys that are released
		else if(event.type == sf::Event::KeyReleased){
			//std::cout << "Key Pressed" << std::endl;
			// Check if the escape key is pressed.
			if(event.key.code == sf::Keyboard::Escape){
				//std::cout << "escape Pressed" << std::endl;
				app.GetWindow().close();
			}
		}
        if(event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
				// Drawing has begun: clear Redo stack
                app.ClearRedo();
            }
        }

		if(event.type == sf::Event::MouseButtonReleased){
			if (event.mouseButton.button == sf::Mouse::Left) {
			    //std::cout << "Mouse released\n";
			    app.AddToDeque();
            }
		}
		if(event.type == sf::Event::KeyPressed){
			if(event.key.code == sf::Keyboard::Z) {
				//std::cout << "Z pressed\n";
				app.Undo();
			}
			else if(event.key.code == sf::Keyboard::Y) {
				//std::cout << "Y pressed\n";
				app.Redo();
			}
			else if (event.key.code == sf::Keyboard::Space) {
				std::shared_ptr<Command> blank = std::make_shared<Clear>(app, app.getCurrentColor());
                if (app.isServer()) {
                    // App is server

                    if (app.serverSocketClosed()) {
                        // server not connected to any clients
                        std::shared_ptr<std::vector<std::shared_ptr<Command>>> vec = 
                        	std::make_shared<std::vector<std::shared_ptr<Command>>>();
                        vec->push_back(blank);
                        app.m_commands.push_back(vec);
                        blank->execute();
                    }
                    else {
                        // server has clients
                        std::shared_ptr<std::vector<std::shared_ptr<Command>>> vec = 
                        	std::make_shared<std::vector<std::shared_ptr<Command>>>();
                        vec->push_back(blank);
                        app.m_commands.push_back(vec);
                        blank->execute();
                    }
                } else {

                    // App is client
                    if (app.clientSocketClosed()) {
                        // Client is not connected
                        std::shared_ptr<std::vector<std::shared_ptr<Command>>> vec = 
                        	std::make_shared<std::vector<std::shared_ptr<Command>>>();
                        vec->push_back(blank);
                        app.m_commands.push_back(vec);
                        blank->execute();
                    } else {
                        // Client connected
						sf::Packet packet;
						packet << (sf::Uint32)9 << app.getCurrentColor().r << app.getCurrentColor().g <<
							app.getCurrentColor().b;
						app.sendPacketFromClient(packet);
                        blank->execute();
                    }
                }
            }
		}
	}
	// Capture input from the nuklear GUI
	nk_input_begin(app.getCTX());
	while(app.GetGUIWindow().pollEvent(event)){
		// Our close event.
		// Note: We only have a 'minimize' button
		//       in our window right now, so this event is not
		//       going to fire.
		if(event.type == sf::Event::Closed){
			app.GetGUIWindow().close();
		}

		// Capture any keys that are released
		else if(event.type == sf::Event::KeyReleased){
			// Check if the escape key is pressed.
			if(event.key.code == sf::Keyboard::Escape){
				app.GetGUIWindow().close();
			}
		}
		nk_sfml_handle_event(&event);
	}

	// Complete input from nuklear GUI
	nk_input_end(app.getCTX());

	// Draw our GUI
	app.drawLayout(app.getCTX(), app.getBG(), app.getMessages());

	// We can otherwise handle events normally
	if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
		//app.ClearRedo();
		sf::Vector2i coordinate = sf::Mouse::getPosition(app.GetWindow());
		if (!((coordinate.x > app.GetImage().getSize().x || coordinate.x < 0) || (coordinate.y > app.GetImage().getSize().y
		|| coordinate.y < 0))) {

			sf::Color beforeColor = app.GetImage().getPixel(coordinate.x, coordinate.y);
			//sf::Color color = cur_color;
			app.mouseX = coordinate.x;
			app.mouseY = coordinate.y;
			if (!(coordinate.x == app.pmouseX && coordinate.y == app.pmouseY) ){
				// Here is where the command pattern is used
				if (app.getCurrentRadius() == 1) {
                	std::shared_ptr<Command> draw = std::make_shared<Draw>(app, coordinate.x, coordinate.y, 
						app.getCurrentColor(), beforeColor);
					if (app.isServer()) {
						// App is server
						if (app.serverSocketClosed()) {
							// server not connected to any clients
							app.addCommandToServer(draw);
							draw->execute();
						} else {
							// server has clients: execute draw
							app.addCommandToServer(draw);
							draw->execute();
						}
					} else {
						// App is client
						if (app.clientSocketClosed()) {
							// No connection: normal code
							app.AddCommand(draw);
							draw->execute();
						} else {
							// Client is connected: send packet
							draw->execute();
							sf::Packet packet;
							packet << 1 << (sf::Uint32)app.getID() << (sf::Uint32)coordinate.x << 
								(sf::Uint32)coordinate.y << app.getCurrentColor().r <<
								app.getCurrentColor().g << app.getCurrentColor().b << 
								beforeColor.r << beforeColor.g << beforeColor.b;
							app.sendPacketFromClient(packet);
							
						}
					}

				} else {
					//std::cout << "Paint() got called\n";
                    std::shared_ptr<Command> paint = std::make_shared<Paint>(app, coordinate.x, coordinate.y, 
						app.getCurrentRadius(), app.getCurrentColor());
					if (app.isServer()) {
						// App is server
						if (app.serverSocketClosed()) {
							// server not connected to any clients
							app.addCommandToServer(paint);
							paint->execute();
						} else {
							// server has clients: execute draw
							app.addCommandToServer(paint);
							paint->execute();
						}
					} else {
						// App is client
						if (app.clientSocketClosed()) {
							// Client is not connected
							app.AddCommand(paint);
							paint->execute();
						} else {
							// Client connected
							paint->execute();
						}
					}
				}
			}
		}
	}

	// Where was the mouse previously before going to the next frame
	app.pmouseX = app.mouseX;
	app.pmouseY = app.mouseY;
}


/*! \brief 	The draw call 
*		
*	@param app a reference to instance of App class.
*/
void draw(App& app){
    //                             ↓ update this variable to adjust number
    //                             ↓ of frames per second
    const float framerate = 1.f / 10.f;

    // when elapsed time is greater than framerate,
    // update the image and reset the clock
    if( ( app.GetClock().getElapsedTime().asSeconds() ) > framerate ){
        app.GetTexture().loadFromImage(app.GetImage());
        app.GetClock().restart();
    }
}

 
/*! \brief 	The entry point into our program.
*		
*/
int main(){
    App_Factory app_factory;
    std::unique_ptr<App> app = app_factory.Create_App();
	// Call any setup function
	// Passing a function pointer into the 'init' function.
	// of our application.
	app->Init(&initialization);
	// Setup your keyboard
	app->UpdateCallback(&update);
	// Setup the Draw Function
	app->DrawCallback(&draw);
	// Call the main loop function
	app->Loop();
	// Terminate the Nuklear GUI library
    nk_sfml_shutdown();
	exit(EXIT_SUCCESS);

	return 0;
}

#endif
