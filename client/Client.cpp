#include "Client.h"





Client::Client(sf::RenderWindow * window)
	: m_window(window)
{
	std::string textures[MAX_CLIENTS] = { "animblack.png", "animgreen.png", "animred.png", "animblue.png" };
	for (uint16 i = 0; i < MAX_PROJECTILES * MAX_CLIENTS; ++i)
	{
		projectil_objects[i].setOwnerSlot(i / (MAX_PROJECTILES));
		projectil_objects[i].setProjectilNumber(i % MAX_PROJECTILES);
	}
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		player_objects[i] = new ClientPlayer(sf::Vector2f(0.0, -200.0 + 70*i), window, textures[i], 0.3, sf::Vector2u(3,9));
	}
	for (uint16 i = 0; i < NUMBER_OF_PROJECTILES; ++i)
	{
		projectil_objects[i].setWindow(window);
	}
}

bool Client::Init(std::string IPAddress)
{
	WORD winsock_version = 0x202;
	WSADATA winsock_data;
	if (WSAStartup(winsock_version, &winsock_data))
	{
		printf("WSAStartup failed: %d\n", WSAGetLastError());
		return false;
	}

	int address_family = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;
	sock = socket(address_family, type, protocol);

	if (sock == INVALID_SOCKET)
	{
		printf("socket failed: %d\n", WSAGetLastError());
		return false;
	}

	int octet[4];
	size_t pos = 0;
	std::string s = IPAddress;
	std::string delimiter = ".";
	std::string token;
	int i = 0;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		octet[i++] = std::stoi(token);
		s.erase(0, pos + delimiter.length());
	}
	octet[i] = std::stoi(s);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.S_un.S_un_b.s_b1 = octet[0];
	server_address.sin_addr.S_un.S_un_b.s_b2 = octet[1];
	server_address.sin_addr.S_un.S_un_b.s_b3 = octet[2];
	server_address.sin_addr.S_un.S_un_b.s_b4 = octet[3];
	return true;
}

bool Client::TryToConnect()
{
	printf("Connecting to server.\n");
	int buffer_length = SOCKET_BUFFER_SIZE;
	int flags = 0;
	SOCKADDR* to = (SOCKADDR*)&server_address;
	int to_length = sizeof(server_address);
	ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
	buffer[0] = 0;
	if (sendto(sock, buffer, buffer_length, flags, to, to_length) == SOCKET_ERROR)
	{
		printf("sendto failed: %d\n", WSAGetLastError());
		return false;
	}

	ZeroMemory(listenBuffer, SOCKET_BUFFER_SIZE);
	// Wait for response
	flags = 0;
	SOCKADDR_IN from;
	int from_size = sizeof(from);

	int bytes_received = recvfrom(sock, listenBuffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);

	if (bytes_received == SOCKET_ERROR)
	{
		printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d\n", WSAGetLastError());
		printf("Connection failed.\n");
		return false;
	}
 
	if ((int8)listenBuffer[1] == (int8)Join_Results::DENIED)
	{
		printf("Server is full.\n");
		printf("Connection failed.\n");
		return false;
	}
	memcpy(&slot, &listenBuffer[2], 2);
	printf("Connection success.\n");
	return true;
}

void Client::setIsRunning(bool isRunning)
{
	this->isRunning = isRunning;
}

void Client::Disconnect()
{
	int buffer_length = SOCKET_BUFFER_SIZE;
	int flags = 0;
	SOCKADDR* to = (SOCKADDR*)&server_address;
	int to_length = sizeof(server_address);
	ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
	buffer[0] = (uint8)Client_Message::Leave;
	memcpy(&buffer[2], &slot, 2);
	if (sendto(sock, buffer, buffer_length, flags, to, to_length) == SOCKET_ERROR)
	{
		printf("sendto failed: %d\n", WSAGetLastError());
		return;
	}
	printf("Disconnected.\n");
}

void Client::HandleState(int8 * buffer, int32 & bytes_read)
{
	uint16 number_of_objects;
	uint16 player_slot;
	uint8 projectil_number;
	Game_Object_Type game_object_type;
	// Disable projectiles
	for (uint16 player_i = 0; player_i < MAX_CLIENTS; ++player_i)
	{
		if (player_objects[player_i]->getPlayerActivity() == ClientPlayer::Activity::ACTIVE)
		{
			for (uint16 projectil_i = 0; projectil_i < MAX_PROJECTILES; ++projectil_i)
			{
				projectil_objects[player_i * MAX_PROJECTILES + projectil_i].setProjectilStatus(ClientProjectil::Projectil_Status::DISABLED);
			}
		}
	}
	memcpy(&number_of_objects, &buffer[bytes_read], sizeof(number_of_objects));
	bytes_read += sizeof(number_of_objects);
	for (uint16 i = 0; i < number_of_objects; ++i)
	{
		game_object_type = (Game_Object_Type)buffer[bytes_read++];
		switch (game_object_type)
		{
		case Game_Object_Type::Player:
			memcpy(&player_slot, &buffer[bytes_read], sizeof(player_slot));
			bytes_read += sizeof(player_slot);
			player_objects[player_slot]->InsertState(buffer, bytes_read);
			time_since_heard_from_player[player_slot] = 0.0f;
			break;
		case Game_Object_Type::Projectil:
 			memcpy(&player_slot, &buffer[bytes_read], sizeof(player_slot));
			bytes_read += sizeof(player_slot);
			projectil_number = buffer[bytes_read++];
			projectil_objects[player_slot * MAX_PROJECTILES + projectil_number].InsertState(buffer, bytes_read);
			//printf("Slot %d  projectil number %d\n", player_slot, projectil_number);
			break;
		default:
			break;
		}
	}
}

void Client::Draw()
{
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (player_objects[i]->getPlayerActivity() == ClientPlayer::Activity::ACTIVE)
		{
			player_objects[i]->draw();
			for (uint16 playerProjectil = 0; playerProjectil < MAX_PROJECTILES; ++playerProjectil)
			{
				ClientProjectil& tmpProjectil = projectil_objects[i * MAX_PROJECTILES + playerProjectil];
				if (tmpProjectil.getProjectilStatus() == ClientProjectil::Projectil_Status::ACTIVE)
					tmpProjectil.draw();

			}
		}
	}
}

void Client::ReadInput()
{
	uint8 input = 0;
	if (!testingDummy)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
		{
			input += (uint8)Player_Input::FIRE;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
		{
			input += (uint8)Player_Input::UP;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
		{
			input += (uint8)Player_Input::LEFT;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		{
			input += (uint8)Player_Input::RIGHT;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		{
			input += (uint8)Player_Input::DOWN;
		}
	}
	SendInput(input);
}

void Client::Update(double deltaTime)
{
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (player_objects[i]->getPlayerActivity() == ClientPlayer::Activity::ACTIVE )
		{
			if (time_since_heard_from_player[i] >= CLIENT_TIMEOUT)
			{
				player_objects[i]->setPlayerActivity(ClientPlayer::Activity::DISABLED);
				continue;
			}
			player_objects[i]->Update(deltaTime);
			time_since_heard_from_player[i] += deltaTime;
		}
			
	}
}

void Client::SendInput(uint8 input)
{
	int flags = 0;
	SOCKADDR* to = (SOCKADDR*)&server_address;
	int to_length = sizeof(server_address);
	ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
	buffer[0] = (uint8)Client_Message::Input;
	int32 bytesWritten = 1;
	memcpy(&buffer[bytesWritten], &slot, sizeof(slot));
	bytesWritten += sizeof(slot);
	buffer[bytesWritten++] = (uint8)input;
	if (sendto(sock, buffer, bytesWritten + 1, flags, to, to_length) == SOCKET_ERROR)
	{
		printf("sendto failed: %d \n", WSAGetLastError());
		return;
	}
}

void Client::Run()
{

	if (TryToConnect())
	{
		printf("Connection success.\n");
	}
	else
	{
		printf("Connection failed.\n");
		return;
	}
	
	//Sleep(2000);
	//Disconnect();
	/*
	std::thread listenThread(&Client::Listen, this);
	bool32 is_running = 1;
	std::thread listenThread(&Client::Listen, this);
	while (is_running)
	{
		ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
		buffer[0] = (uint8)Client_Message::Input;
		// get input
		scanf_s("\n%s", &buffer[1], SOCKET_BUFFER_SIZE);
		// send to server
		Send();
		// wait for reply
		/*
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
	listenThread.join();
	*/
}

void Client::Send()
{
	int buffer_length = SOCKET_BUFFER_SIZE;
	int flags = 0;
	SOCKADDR* to = (SOCKADDR*)&server_address;
	int to_length = sizeof(server_address);
	if (sendto(sock, buffer, buffer_length, flags, to, to_length) == SOCKET_ERROR)
	{
		printf("sendto failed: %d", WSAGetLastError());
		return;
	}
}

void Client::Listen()
{
	while (isRunning)
	{
		ZeroMemory(listenBuffer, SOCKET_BUFFER_SIZE);
		int flags = 0;
		SOCKADDR_IN from;
		int from_size = sizeof(from);
		int bytes_received = recvfrom(sock, listenBuffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);

		if (bytes_received == SOCKET_ERROR)
		{
			printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d\n", WSAGetLastError());
			return;
		}
		//printf("Server echo: %s\n", listenBuffer + 1);
		//heard_from_server = 0.0;
		if (bytes_received > 0)
			ParseBuffer(listenBuffer);
	}
}

void Client::ParseBuffer(int8* buffer)
{
	Server_Message type_of_message = (Server_Message)buffer[0];
	int32 bytes_read = 1;
	switch (type_of_message)
	{
	case Server_Message::State:
		HandleState(buffer, bytes_read);
		break;
	default:
		break;
	}
}


Client::~Client()
{
	Disconnect();
	WSACleanup();
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		delete player_objects[i];
	}
}
