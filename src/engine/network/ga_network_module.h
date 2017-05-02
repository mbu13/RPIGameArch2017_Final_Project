#pragma once

/*
** RPI Game Architecture Engine 2017
**
** Author: Matthew Bu
*/

#include "entity/ga_input_component.h"
#include <winsock2.h>
#include "math/ga_vec3f.h"
#include "entity/ga_entity.h"

#pragma comment(lib,"ws2_32.lib")

#define SERVER "127.0.0.1"
#define BUFFER 1024
#define PORT 8888

enum Type 
{ 
	K_PING, 
	K_PONG, 
	K_MESSAGE, 
	K_CLIENT_MOVEMENT, 
	K_SERVER_MOVEMENT, 
	K_ADD_PLAYER,
	K_SIM_LAG,
	K_UNKNOWN
};

struct ga_server
{
	SOCKET _sd;
	struct sockaddr_in _server;
	int _slen;
};

struct ga_client
{
	int _s;
	struct sockaddr_in _si_other;
	int _slen = sizeof(_si_other);
};

/*
** A network module that implements both UDP server
** and client. Holds two input components to dictate
** movement upon message receive.
*/
class ga_network_module
{
private:
	struct ga_server _server;
	struct ga_client _client;

	bool _is_server;
	bool _send_message;

	float _ping;
	char _msg[BUFFER];

	Type _send_type;
	ga_input_component *_cube_move_other;
	ga_input_component *_cube_move_this;

	void start_listener();
	void start_sender();
	void start_reading_chat();
	int initialize_socket_to_server(int *s, WSADATA *wsa);
	int initialize_socket_to_client(SOCKET *s, WSADATA *wsa);
	int send_and_receive(int s, int *slen, struct sockaddr_in *si_other, const char *message, char *buffer);
	Type get_request_type(const char *buffer);

	void copy_string_to_buffer(std::string str, char *buffer);
public:
	char _addr[BUFFER];
	int _port;
	bool _add_player;
	bool _sim_lag;
	unsigned int _min_lag;

	ga_network_module();
	void start_server();
	void start_client();
	void add_input_component_other(ga_input_component *cube_move);
	void add_input_component_this(ga_input_component *cube_move);

	void set_send_type(Type type);
	void send_transform_msg_to_server(const ga_vec3f new_transform);
	void send_chat_msg_to_server(const char *msg);
	void send_add_player_to_server();

	void simulate_lag(int min, int max);

	float get_ping();
	char *get_message();
};