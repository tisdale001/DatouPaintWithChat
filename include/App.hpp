#ifndef APP_HPP
#define APP_HPP

/**
 *  @file   App.hpp
 *  @brief  App class interface
 *  @author Mike, Aditya, Chiemelie, Lucian, Mary, Nate
 *  @date   November 2021
 ***********************************************/

// TODO Run valgrind

// Include our Third-Party SFML header
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

// Include some standard libraries
// The 'C-based' libraries are needed for Nuklear GUI
#include <string.h>


// Include standard library C++ libraries.
#include <queue>
#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <functional>

// Project header files
#include <Command.hpp>

// Forward declaration of our Command class
class Command;

// Global variables for GUI and program function
enum {BLACK, WHITE, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN}; 
static int op = BLACK;

enum {SMALL, MEDIUM, LARGE};
static int bs = SMALL;

class App {
private:
// Member variables
	// Cur_vec is 'current vector' which holds the current paint stroke
	// being drawn. When mouse button is released, the cur_vec is stored
	// in the m_commands deque so it can be 'undoed'.
    std::vector<std::shared_ptr<Command>> cur_vec;

	// current_vectors is a vector of vetors which holds each client's brush strokes
	std::vector<std::shared_ptr<std::vector<std::shared_ptr<Command>>>> current_vectors;

	// vector for messages
	std::shared_ptr<std::vector<char*>> msg_vec = std::make_shared<std::vector<char*>>();

    // Main image
	sf::Image m_image;

	// Create a sprite that we overlay
	// on top of the texture.
	sf::Sprite m_sprite;
	
	// Texture sent to the GPU for rendering
	sf::Texture m_texture;

	// Our rendering window
    sf::RenderWindow m_window;

	// Our GUI window
	sf::RenderWindow gui_window;
	struct nk_colorf* bg;
	struct nk_context *ctx;

    // Clock to update screen
    sf::Clock m_clock;

    // Window size variables
	int win_width = 1200;
	int win_height = 800;

    // Drawing variables
    sf::Color cur_color;
	int cur_radius; //for Paint feature

    // General networking variables
    long port_number;
    std::vector<std::thread> thread_vector;
    std::string username;
    std::shared_mutex m_mutex;
    int newClientID;
    int appID;
    bool is_server;

    // Server variables
	sf::SocketSelector socket_selector;
    std::vector<std::shared_ptr<sf::TcpSocket>> socket_vector;
    //std::vector<std::string> message_history;
    sf::TcpListener listener;

	// Client variables
    sf::TcpSocket client_socket;
    std::string ip_address;

    // Member functions
	
	// Store the address of our function pointer
	// for each of the callback functions.
	std::function<void(App& app)> m_initFunc = nullptr;
    std::function<void(App& app)> m_updateFunc = nullptr;
    std::function<void(App& app)> m_drawFunc = nullptr;

    // private helper functions
    long get_port();
    std::string get_ip();
    void server_undo();
    void client_undo();
    void add_connection();
    void server_receive_packet();

public:
    // Member Variables
	int pmouseX;
	int pmouseY;
	int mouseX;
	int mouseY;

	// Deque stores the next command to do. Deque is similar to a stack
	// except you can pop off the front. I use this feature to limit the 
	// size of the deque to prevent stack overflow. We had to make this public
	// to function in the 'Clear' command class.
	std::deque<std::shared_ptr<std::vector<std::shared_ptr<Command>>>> m_commands;

	// Stack that stores the redo items. We had to make public to function in
	// the 'Clear' command class.
    std::stack<std::shared_ptr<std::vector<std::shared_ptr<Command>>>> m_redo;
	
    //  Constructor and destructor
	App();
    ~App();

    // Constructor for when app is functioning as the server
    App(long port, std::string username);

    // Constructor for the app is functioning as the client
    App(std::string ip, long port, std::string username);

    // Delete the copy constructor and the copy assignment constructor
    App (const App& other) = delete;
    App& operator=(const App& other) = delete;

    // Member functions
	void AddCommand(std::shared_ptr<Command>);
	void AddToDeque();
	void Undo();
	void ClearRedo();
	void Redo();
	sf::Image& GetImage();
	sf::Texture& GetTexture();
	sf::RenderWindow& GetWindow();
    sf::Clock& GetClock();

    
    void setRadius(int new_radius);
	int getCurrentRadius();
    sf::Color& getCurrentColor();
    void setCurrentColor(sf::Color color);

	// GUI functions
	struct nk_context* getCTX();
	sf::RenderWindow& GetGUIWindow();
	struct nk_colorf* getBG();
	std::shared_ptr<std::vector<char*>> getMessages();

	// Server and Client methods
	void setServer(bool b);
	bool isServer();
	std::string getUsername();
	int getID();
	void extractPacket(sf::Packet);

	// Server methods
	void initializeServerID();
	void setUsername();
	void setIPAddressCreateListener();
	void sendPacket(sf::Packet &outgoing_packet);
	void connectServer();
	void communicateServer();
	bool serverSocketClosed();
	void addCommandToServer(std::shared_ptr<Command>);

	// Client methods
	void connect();
	void communicate();
	void receivePacket();
	void sendPacketFromClient(sf::Packet packet);
	bool clientSocketClosed();
	void setID(int);

	// methods to be called in 'main'
	void Init(std::function<void(App& app)> initFunction);
	void UpdateCallback(std::function<void(App& app)> updateFunction);
	void DrawCallback(std::function<void(App& app)> drawFunction);
	void Loop();
	void drawLayout(struct nk_context* ctx, struct nk_colorf* bg1, std::shared_ptr<std::vector<char*>> msgs);
};


#endif