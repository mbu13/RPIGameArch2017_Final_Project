/*
** RPI Game Architecture Engine 2017
**
** Author: Matthew Bu
*/

#include "ga_network_module.h"

#include <iostream>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <string.h>
#include <random>

using namespace std::chrono;

ga_network_module::ga_network_module()
{
	_cube_move_other = NULL;
	
	memset(_msg, '\0', BUFFER);
	memset(_addr, '\0', BUFFER);
	
	_add_player = false;
	_is_server = false;
	_send_message = false;
	_sim_lag = false;

	_min_lag = 0;
	_port = 0;
	_ping = 0.0f;
}

Type ga_network_module::get_request_type(const char *buffer) {
	char type[5];
	memcpy(type, buffer, sizeof(type));
	type[4] = '\0';

	if (strcmp(type, "PING") == 0)
	{
		return K_PING;
	}
	else if (strcmp(type, "PONG") == 0)
	{
		return K_PONG;
	}
	else if (strcmp(type, "MESG") == 0)
	{
		return K_MESSAGE;
	}
	else if (strcmp(type, "MOVE") == 0)
	{
		return K_CLIENT_MOVEMENT;
	}
	else if (strcmp(type, "ADDP") == 0)
	{
		return K_ADD_PLAYER;
	}
	else if (strcmp(type, "SMOV") == 0)
	{
		return K_SERVER_MOVEMENT;
	}
	else
	{
		return K_UNKNOWN;
	}
}

void ga_network_module::start_reading_chat()
{
	std::cout << "CHAT ROOM STARTED\n>> ";

	while (1)
	{
		char in[BUFFER];
		std::cin.getline(in, BUFFER);

		if (!_is_server)
		{
			send_chat_msg_to_server(in);
		}
		else
		{
			copy_string_to_buffer(in, _msg);
			_send_message = true;
		}
	}
}

void ga_network_module::start_server()
{
	_is_server = true;
	std::thread t1(&ga_network_module::start_listener, this);
	std::thread t2(&ga_network_module::start_reading_chat, this);
	t1.detach();
	t2.detach();
}

void ga_network_module::start_listener()
{
	SOCKET sd;
	struct sockaddr_in server;
	struct sockaddr_in si_other;
	int slen;
	int recv_len;
	char data[BUFFER - 4];
	char buffer[BUFFER];
	char reply[BUFFER];
	WSADATA wsa;

	slen = sizeof(si_other);
	
	if (initialize_socket_to_client(&sd, &wsa))
	{
		std::cout << "ERROR: Failed to initialize socket\n";
		return;
	}

	/* Clear out server struct */
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
	{
		std::cout << "ERROR: Failed to bind socket\n";
		return;
	}

	_server._sd = sd;
	_server._slen = slen;
	_server._server = server;

	memset(_addr, '\0', BUFFER);
	int ret = snprintf(_addr, BUFFER / 8, "%s", SERVER);
	_addr[ret] = '\0';
	_port = PORT;

	// Start server
	while (1)
	{
		// Clear the buffer
		memset(buffer, '\0', BUFFER);
		memset(reply, '\0', BUFFER);
		memset(_msg, '\0', BUFFER);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(sd, buffer, BUFFER, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			std::cout << "ERROR: recv_len() failed\n";
			break;
		}

		if (_sim_lag)
		{
			simulate_lag(_min_lag, _min_lag + 10);
		}

		Type type = get_request_type(buffer);

		switch (type) 
		{
		case K_PING:
		{
			copy_string_to_buffer("PONG", reply);
		}
		break;
		case K_MESSAGE:
		{
			copy_string_to_buffer("ACK", reply);

			char msg[BUFFER - 4];

			memcpy(msg, buffer + 4, recv_len - 4);
			msg[recv_len - 4] = '\0';

			std::cout << "Client: " << msg << std::endl;
		}
		break;
		case K_SERVER_MOVEMENT:
		{

			int n = snprintf(reply, sizeof(reply), "MOVE%f%f%f",
				_cube_move_this->_new_transform.x,
				_cube_move_this->_new_transform.y,
				_cube_move_this->_new_transform.z);

			reply[n] = '\0';
			
			if(_send_message && strlen(_msg) > 0)
			{
				reply[0] = '\0';
				int n = snprintf(reply, sizeof(reply), "MESG%s", _msg);

				reply[n] = '\0';
				_send_message = false;
			}
		}
		break;
		case K_CLIENT_MOVEMENT:
		{
			copy_string_to_buffer("ACK", reply);

			char x_val[5];
			char y_val[5];
			char z_val[5];

			memcpy(x_val, buffer + 4, 4);
			x_val[4] = '\0';
			float x = atof(x_val);

			memcpy(y_val, buffer + 12, 4);
			y_val[4] = '\0';
			float y = atof(y_val);

			memcpy(z_val, buffer + 20, 4);
			z_val[4] = '\0';
			float z = atof(z_val);

			_cube_move_other->_move_left = (x < 0) ? true : false;
			_cube_move_other->_move_right = (x > 0) ? true : false;
			_cube_move_other->_move_forward = (y > 0) ? true : false;
			_cube_move_other->_move_backward = (y < 0) ? true : false;

			memset(_msg, '\0', BUFFER);
			int ret = snprintf(_msg, BUFFER - 1, "x %.2f, y %.2f, z %.2f", x, y, z);
			_msg[ret] = '\0';
		}
		break;
		case K_ADD_PLAYER:
		{
			_add_player = true;
		}
		break;
		}

		// Reply to client
		if (sendto(sd, reply, sizeof(reply), 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
		{
			std::cout << "ERROR: sendto() failed\n";
			break;
		}
	}

	closesocket(sd);
	WSACleanup();
}

void ga_network_module::start_client()
{
	_is_server = false;
	std::thread t1(&ga_network_module::start_sender, this);
	std::thread t2(&ga_network_module::start_reading_chat, this);
	t1.detach();
	t2.detach();
}

void ga_network_module::start_sender()
{
	struct sockaddr_in si_other;
	int s = 0;
	int slen = sizeof(si_other);
	char data[BUFFER - 4];
	char buffer[BUFFER];
	char message[BUFFER];
	WSADATA wsa;

	if (initialize_socket_to_server(&s, &wsa) != 0)
	{
		std::cout << "ERROR: Failed to initialize socket\n";
		return;
	}

	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

	_client._s = s;
	_client._slen = slen;
	_client._si_other = si_other;

	memset(_addr, '\0', BUFFER);
	int ret = snprintf(_addr, BUFFER / 8, "%s", inet_ntoa(si_other.sin_addr));
	_addr[ret] = '\0';
	_port = ntohs(si_other.sin_port);

	send_add_player_to_server();

	// Start pinging
	std::cout << "PINGING...\n";
	while (1)
	{
		copy_string_to_buffer("SMOV", message);

		// Send buffer
		milliseconds ms1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		int n = 0;
		if ((n = send_and_receive(s, &slen, &si_other, message, buffer)) != 0)
		{
			break;
		}

		if (_sim_lag)
		{
			simulate_lag(_min_lag, _min_lag + 10);
		}

		milliseconds ms2 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

		Type type = get_request_type(buffer);

		switch (type)
		{
		case K_ADD_PLAYER:
			_add_player = true;
			break;
		case K_MESSAGE:
		{
			char msg[BUFFER - 4];

			memcpy(msg, buffer + 4, BUFFER / 8);
			msg[BUFFER / 8] = '\0';

			std::cout << "Server: " << msg << std::endl;
		}
		break;
		case K_CLIENT_MOVEMENT:
		{
			_ping = ms2.count() - ms1.count();

			char x_val[5];
			char y_val[5];
			char z_val[5];

			memcpy(x_val, buffer + 4, 4);
			x_val[4] = '\0';
			float x = atof(x_val);

			memcpy(y_val, buffer + 12, 4);
			y_val[4] = '\0';
			float y = atof(y_val);

			memcpy(z_val, buffer + 20, 4);
			z_val[4] = '\0';
			float z = atof(z_val);

			_cube_move_other->_move_left = (x < 0) ? true : false;
			_cube_move_other->_move_right = (x > 0) ? true : false;
			_cube_move_other->_move_forward = (y > 0) ? true : false;
			_cube_move_other->_move_backward = (y < 0) ? true : false;

			memset(_msg, '\0', BUFFER);
			int ret = snprintf(_msg, BUFFER - 1, "x %.2f, y %.2f, z %.2f", x, y, z);
			_msg[ret] = '\0';
		}
		break;
		}

		memset(buffer, '\0', BUFFER);
		memset(message, '\0', BUFFER);
	}

	closesocket(s);
	WSACleanup();
}

int ga_network_module::initialize_socket_to_server(int *s, WSADATA *wsa)
{
	// Initialize
	if (WSAStartup(MAKEWORD(2, 2), wsa) != 0)
	{
		std::cout << "ERROR: Failed to start WINSOCK\n";
		return -1;
	}
	printf("Initialised.\n");

	// Create socket
	if ((*s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		std::cout << "ERROR: Failed to create socket\n";
		return -1;
	}

	return 0;
}

int ga_network_module::initialize_socket_to_client(SOCKET *sd, WSADATA *wsa)
{
	// Initialize
	if (WSAStartup(0x0101, wsa) != 0)
	{
		std::cout << "ERROR: Failed to start WINSOCK\n";
		return -1;
	}

	// Create a socket
	*sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (*sd == INVALID_SOCKET)
	{
		std::cout << "ERROR: Failed to create socket\n";
		return -1;
	}

	return 0;
}

int ga_network_module::send_and_receive(int s, int *slen, struct sockaddr_in *si_other, const char *message, char *buffer)
{
	if (sendto(s, message, strlen(message), 0, (struct sockaddr *) si_other, *slen) == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
		return -1;
	}

	memset(buffer, '\0', BUFFER);
	int n = 0;
	if (n = recvfrom(s, buffer, BUFFER, 0, (struct sockaddr *) si_other, slen) == SOCKET_ERROR)
	{
		std::cout << "ERROR: recvfrom() failed\n";
		return -1;
	}

	return n;
}

void ga_network_module::simulate_lag(int min, int max)
{
	std::random_device rd;							// obtain a random number from hardware
	std::mt19937 eng(rd());							// seed the generator
	std::uniform_int_distribution<> distr(min, max);	// define the range

	std::this_thread::sleep_for(std::chrono::milliseconds(distr(eng)));
}

void ga_network_module::copy_string_to_buffer(std::string str, char *buffer) 
{
	for (int i = 0; i < str.length(); i++)
		buffer[i] = str[i];
	buffer[str.length()] = '\0';
}

void ga_network_module::set_send_type(Type type) 
{
	_send_type = type;
}

void ga_network_module::send_transform_msg_to_server(const ga_vec3f new_transform)
{
	// Init message
	char transform[BUFFER / 4];
	char buffer[BUFFER];

	int n = snprintf(transform, sizeof(transform), "MOVE%f%f%f", 
		new_transform.x, 
		new_transform.y,
		new_transform.z);

	transform[n] = '\0';

	if (send_and_receive(_client._s, &_client._slen, &_client._si_other, transform, buffer) != 0)
	{
		return;
	}
}

void ga_network_module::send_chat_msg_to_server(const char *msg)
{
	// Init message
	char message[BUFFER / 4];
	int n = snprintf(message, sizeof(message), "MESG%s", msg);

	message[n] = '\0';

	char buffer[BUFFER];

	if (send_and_receive(_client._s, &_client._slen, &_client._si_other, message, buffer) != 0)
	{
		return;
	}
}

void ga_network_module::send_add_player_to_server()
{
	// Init message
	char message[BUFFER / 4];
	char buffer[BUFFER];
	int n = snprintf(message, sizeof(message), "ADDP");

	message[n] = '\0';

	if (send_and_receive(_client._s, &_client._slen, &_client._si_other, message, buffer) != 0)
	{
		return;
	}
}

void ga_network_module::add_input_component_other(ga_input_component *cube_move)
{
	_cube_move_other = cube_move;
}

void ga_network_module::add_input_component_this(ga_input_component *cube_move)
{
	_cube_move_this = cube_move;
}

float ga_network_module::get_ping()
{
	return _ping;
}

char* ga_network_module::get_message()
{
	return _msg;
}
