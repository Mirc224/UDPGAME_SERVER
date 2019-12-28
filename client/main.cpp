#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <SFML/Graphics.hpp>
#include "Client.h"
static const float VIEW_HEIGHT = 680;
static const float VIEW_WIDTH = 960;
static short gameStatus = 1;

void ResizeView(const sf::RenderWindow& window, sf::View& view)
{
	float aspectRation = float(window.getSize().x) / float(window.getSize().y);
	view.setSize(VIEW_HEIGHT * aspectRation, VIEW_HEIGHT);
}



void main(int argc, char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	sf::RenderWindow window(sf::VideoMode(VIEW_WIDTH, VIEW_HEIGHT), "Hra");
	Client klient(&window);
	if (argc > 1)
	{
		klient.testingDummy = true;
	}
	sf::View view(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(VIEW_WIDTH, VIEW_HEIGHT));
	float deltaTime = 0.0;
	sf::Clock clock;
	if (klient.Init() && klient.TryToConnect())
	{
		std::thread listenThread(&Client::Listen, &klient);
		while (window.isOpen())
		{
			deltaTime = clock.restart().asSeconds();
			sf::Event event;
			while (window.pollEvent(event))
			{
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
					klient.setIsRunning(false);
					break;
				case sf::Event::Resized:
					//std::cout << "New wideo width: " << event.size.width << " event.size.height " << event.size.height << std::endl;
					ResizeView(window, view);
					break;
				case sf::Event::TextEntered:
					//if (event.text.unicode < 128)
					//printf("%c\n", event.text.unicode);
					break;
				default:
					break;
				}

				if (event.type == sf::Event::Closed)
					window.close();
			}
			klient.ReadInput();
			window.setView(view);
			window.clear(sf::Color(0, 255, 255, 255));
			klient.Update(deltaTime);
			klient.Draw();
			window.display();
			Sleep(20);
			
		}
		listenThread.join();
	}
	/*
	WORD winsock_version = 0x202;
	WSADATA winsock_data;
	if (WSAStartup(winsock_version, &winsock_data))
	{
		printf("WSAStartup failed: %d", WSAGetLastError());
		return;
	}

	int address_family = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;
	SOCKET sock = socket(address_family, type, protocol);

	if (sock == INVALID_SOCKET)
	{
		printf("socket failed: %d", WSAGetLastError());
		return;
	}

	SOCKADDR_IN server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.S_un.S_un_b.s_b1 = 127;
	server_address.sin_addr.S_un.S_un_b.s_b2 = 0;
	server_address.sin_addr.S_un.S_un_b.s_b3 = 0;
	server_address.sin_addr.S_un.S_un_b.s_b4 = 1;

	int8 buffer[SOCKET_BUFFER_SIZE];
	int32 player_x;
	int32 player_y;

	printf("type w, a, s, or d to move, q to quit\n");
	bool32 is_running = 1;
	while (is_running)
	{
		// get input
		scanf_s("\n%c", &buffer[0], 1);

		// send to server
		int buffer_length = 1;
		int flags = 0;
		SOCKADDR* to = (SOCKADDR*)&server_address;
		int to_length = sizeof(server_address);
		if (sendto(sock, buffer, buffer_length, flags, to, to_length) == SOCKET_ERROR)
		{
			printf("sendto failed: %d", WSAGetLastError());
			return;
		}

		// wait for reply
		flags = 0;
		SOCKADDR_IN from;
		int from_size = sizeof(from);
		int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);

		if (bytes_received == SOCKET_ERROR)
		{
			printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError());
			break;
		}

		// grab data from packet
		int32 read_index = 0;

		memcpy(&player_x, &buffer[read_index], sizeof(player_x));
		read_index += sizeof(player_x);

		memcpy(&player_y, &buffer[read_index], sizeof(player_y));
		read_index += sizeof(player_y);

		memcpy(&is_running, &buffer[read_index], sizeof(is_running));

		printf("x:%d, y:%d, is_running:%d\n", player_x, player_y, is_running);
	}
	*/
}