#include "Server.h"

bool operator==(const IP_Endpoint& a, const IP_Endpoint& b) { return a.address == b.address && a.port == b.port; }

Server::Server()
{
}

void Server::Init()
{
	WORD winsock_version = 0x202;
	WSADATA winsock_data;
	if (WSAStartup(winsock_version, &winsock_data))
	{
		printf("WSAStartup failed: %d\n", WSAGetLastError());
		return;
	}
	int address_family = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;
	sock = socket(address_family, type, protocol);

	if (sock == INVALID_SOCKET)
	{
		printf("socket failed: %d\n", WSAGetLastError());
		return;
	}

	local_address.sin_family = AF_INET;
	local_address.sin_port = htons(PORT);
	local_address.sin_addr.s_addr = INADDR_ANY;
	if (bind(sock, (SOCKADDR*)&local_address, sizeof(local_address)) == SOCKET_ERROR)
	{
		printf("bind failed: %d\n", WSAGetLastError());
		return;
	}
	u_long enabled = 1;
	ioctlsocket(sock, FIONBIO, &enabled);

	sleep_granularity_ms = 1;
	sleep_granularity_was_set = timeBeginPeriod(sleep_granularity_ms) == TIMERR_NOERROR;
	QueryPerformanceFrequency(&clock_frequency);

	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		client_endpoints[i] = {};
	}
	printf("Server started\n");

	for (uint16 i = 0; i < MAX_PROJECTILES * MAX_CLIENTS; ++i)
	{
		projectil_objects[i].setOwnerSlot(i / (MAX_PROJECTILES));
		projectil_objects[i].setProjectilNumber(i % MAX_PROJECTILES);
		projectil_objects[i].setProjectilStatus(Projectil::Projectil_Status::DISABLED);
	}
	map_border[0].getBody().setPosition(-MAP_WIDTH / 2.0f, 0.0f);
	map_border[0].getBody().setSize(0.0, MAP_HEIGHT);
	map_border[1].getBody().setPosition(0.0f, MAP_HEIGHT / 2.0f);
	map_border[1].getBody().setSize(MAP_WIDTH, 0.0f);
	map_border[2].getBody().setPosition(MAP_WIDTH / 2.0f, 0.0f);
	map_border[2].getBody().setSize(0.0, MAP_HEIGHT);
	map_border[3].getBody().setPosition(0.0f, -MAP_HEIGHT / 2.0f);
	map_border[3].getBody().setSize(MAP_WIDTH, 0.0f);

	time_without_players = 0.0f;
}

void Server::Run()
{

	bool32 updated = false;
	std::string input;
	while (is_running)
	{
		has_player = false;
		updated = false;
		LARGE_INTEGER tick_start_time;
		QueryPerformanceCounter(&tick_start_time);
		/*
		ZeroMemory(listenBuffer, SOCKET_BUFFER_SIZE);
		// read all available packets
		while (true)
		{

			int flags = 0;
			SOCKADDR_IN from;
			int from_size = sizeof(from);
			int bytes_received = recvfrom(sock, listenBuffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);

			if (bytes_received == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d\n", error);
				}
				break;
			}

			IP_Endpoint from_endpoint;
			from_endpoint.address = from.sin_addr.S_un.S_addr;
			from_endpoint.port = from.sin_port;
			//char volba = buffer[0];
			//int tmp = buffer[0] - '0';	

			switch ((uint8)listenBuffer[0])
			{
			case (uint8)Client_Message::Join:
				AddNewClient(from_endpoint, from);
				break;
			case (uint8)Client_Message::Leave:
				RemoveClient(from_endpoint, from);
				break;
			case (uint8)Client_Message::Input:
				//printf("%s\n", buffer+1);
				HandlePlayerInput();
				//SendToAll();
				break;

			default:
				break;
			}
			
			
			updated = true;
			/*
			ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
			buffer[0] = '1';
			sendto(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, from_size);
			*/

			/*
			switch (tmp)
			{
			case (int)Client_Message::Join:
			{
				printf("Client_Message::Join from %u:%hu\n", from_endpoint.address, from_endpoint.port);

				uint16 slot = uint16(-1);
				for (uint16 i = 0; i < MAX_CLIENTS; ++i)
				{
					if (client_endpoints[i].address == 0)
					{
						slot = i;
						break;
					}
				}

				buffer[0] = (int8)Server_Message::Join_Result;
				if (slot != uint16(-1))
				{
					printf("client will be assigned to slot %hu\n", slot);
					buffer[1] = 1;
					memcpy(&buffer[2], &slot, 2);

					flags = 0;
					if (sendto(sock, buffer, 4, flags, (SOCKADDR*)&from, from_size) != SOCKET_ERROR)
					{
						client_endpoints[slot] = from_endpoint;
						time_since_heard_from_clients[slot] = 0.0f;
						client_objects[slot] = {};
						client_inputs[slot] = {};
					}
					else
					{
						printf("sendto failed: %d\n", WSAGetLastError());
					}
				}
				else
				{
					printf("could not find a slot for player\n");
					buffer[1] = 0;

					flags = 0;
					if (sendto(sock, buffer, 2, flags, (SOCKADDR*)&from, from_size) == SOCKET_ERROR)
					{
						printf("sendto failed: %d\n", WSAGetLastError());
					}
				}
			}
			break;

			case (int)Client_Message::Leave:
			{
				uint16 slot;
				memcpy(&slot, &buffer[1], 2);

				if (client_endpoints[slot] == from_endpoint)
				{
					client_endpoints[slot] = {};
					printf("Client_Message::Leave from %hu(%u:%hu)\n", slot, from_endpoint.address, from_endpoint.port);
				}
			}
			break;

			case (int)Client_Message::Input:
			{
				uint16 slot;
				memcpy(&slot, &buffer[1], 2);

				printf("%d %hu\n", bytes_received, slot);

				if (client_endpoints[slot] == from_endpoint)
				{
					uint8 input = buffer[3];

					client_inputs[slot].up = input & 0x1;
					client_inputs[slot].down = input & 0x2;
					client_inputs[slot].left = input & 0x4;
					client_inputs[slot].right = input & 0x8;

					time_since_heard_from_clients[slot] = 0.0f;

					printf("Client_Message::Input from %hu:%d\n", slot, int32(input));
				}
				else
				{
					printf("Client_Message::Input discarded, was from %u:%hu but expected %u:%hu\n", from_endpoint.address, from_endpoint.port, client_endpoints[slot].address, client_endpoints[slot].port);
				}
			}
			break;
			}
			}
			*/
		
			UpdateGame();
			SendGameStateToAll();

		float32 time_taken_s = time_since(tick_start_time, clock_frequency);
		if (time_since_spawn >= NEXT_RESPAWN)
		{
			time_since_spawn = 0.0f;
			RespawnPlayer(0);
		}
		while (time_taken_s < SECONDS_PER_TICK)
		{
			if (sleep_granularity_was_set)
			{
				DWORD time_to_wait_ms = DWORD((SECONDS_PER_TICK - time_taken_s) * 1000);
				if (time_to_wait_ms > 0)
				{
					Sleep(time_to_wait_ms);
				}
			}

			time_taken_s = time_since(tick_start_time, clock_frequency);
		}
		time_since_spawn += SECONDS_PER_TICK;
		if (has_player)
		{
			time_without_players = 0.0f;
		}
		else
		{
			if (time_without_players >= EMPTY_SERVER_TIMEOUT)
			{
				is_running = false;
			}
			time_without_players += SECONDS_PER_TICK;
		}

		// process input and update state
		/*
		for (uint16 i = 0; i < MAX_CLIENTS; ++i)
		{
			if (client_endpoints[i].address)
			{
				if (client_inputs[i].up)
				{
					client_objects[i].speed += ACCELERATION * SECONDS_PER_TICK;
					if (client_objects[i].speed > MAX_SPEED)
					{
						client_objects[i].speed = MAX_SPEED;
					}
				}
				if (client_inputs[i].down)
				{
					client_objects[i].speed -= ACCELERATION * SECONDS_PER_TICK;
					if (client_objects[i].speed < 0.0f)
					{
						client_objects[i].speed = 0.0f;
					}
				}
				if (client_inputs[i].left)
				{
					client_objects[i].facing -= TURN_SPEED * SECONDS_PER_TICK;
				}
				if (client_inputs[i].right)
				{
					client_objects[i].facing += TURN_SPEED * SECONDS_PER_TICK;
				}

				client_objects[i].x += client_objects[i].speed * SECONDS_PER_TICK * sinf(client_objects[i].facing);
				client_objects[i].y += client_objects[i].speed * SECONDS_PER_TICK * cosf(client_objects[i].facing);

				time_since_heard_from_clients[i] += SECONDS_PER_TICK;
				if (time_since_heard_from_clients[i] > CLIENT_TIMEOUT)
				{
					printf("client %hu timed out\n", i);
					client_endpoints[i] = {};
				}
			}
		}

		// create state packet
		buffer[0] = (int8)Server_Message::State;
		int32 bytes_written = 1;
		for (uint16 i = 0; i < MAX_CLIENTS; ++i)
		{
			if (client_endpoints[i].address)
			{
				memcpy(&buffer[bytes_written], &i, sizeof(i));
				bytes_written += sizeof(i);

				memcpy(&buffer[bytes_written], &client_objects[i].x, sizeof(client_objects[i].x));
				bytes_written += sizeof(client_objects[i].x);

				memcpy(&buffer[bytes_written], &client_objects[i].y, sizeof(client_objects[i].y));
				bytes_written += sizeof(client_objects[i].y);

				memcpy(&buffer[bytes_written], &client_objects[i].facing, sizeof(client_objects[i].facing));
				bytes_written += sizeof(client_objects[i].facing);
			}
		}

		// send back to clients
		int flags = 0;
		SOCKADDR_IN to;
		to.sin_family = AF_INET;
		to.sin_port = htons(PORT);
		int to_length = sizeof(to);

		for (uint16 i = 0; i < MAX_CLIENTS; ++i)
		{
			if (client_endpoints[i].address)
			{
				to.sin_addr.S_un.S_addr = client_endpoints[i].address;
				to.sin_port = client_endpoints[i].port;

				if (sendto(sock, buffer, bytes_written, flags, (SOCKADDR*)&to, to_length) == SOCKET_ERROR)
				{
					printf("sendto failed: %d\n", WSAGetLastError());
				}
			}
		}

		// wait until tick complete
		float32 time_taken_s = time_since(tick_start_time, clock_frequency);

		while (time_taken_s < SECONDS_PER_TICK)
		{
			if (sleep_granularity_was_set)
			{
				DWORD time_to_wait_ms = DWORD((SECONDS_PER_TICK - time_taken_s) * 1000);
				if (time_to_wait_ms > 0)
				{
					Sleep(time_to_wait_ms);
				}
			}

			time_taken_s = time_since(tick_start_time, clock_frequency);
		}
		*/
	}
}

void Server::Listen()
{
	while (is_running)
	{
		int flags = 0;
		ZeroMemory(&listenBuffer, SOCKET_BUFFER_SIZE);
		SOCKADDR_IN from;
		int from_size = sizeof(from);
		while (true)
		{
			int bytes_received = recvfrom(sock, listenBuffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);
			if (bytes_received == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d\n", error);
				}

				break;
			}

			IP_Endpoint from_endpoint;
			from_endpoint.address = from.sin_addr.S_un.S_addr;
			from_endpoint.port = from.sin_port;
			//char volba = buffer[0];
			//int tmp = buffer[0] - '0';	

			switch ((uint8)listenBuffer[0])
			{
			case (uint8)Client_Message::Join:
				AddNewClient(from_endpoint, from);
				break;
			case (uint8)Client_Message::Leave:
				RemoveClient(from_endpoint, from);
				break;
			case (uint8)Client_Message::Input:
				//printf("%s\n", buffer+1);
				HandlePlayerInput();
				//SendToAll();
				break;

			default:
				break;
			}
		}
	}
}
/*
bool Server::Receive()
{
	int flags = 0;
	SOCKADDR_IN from;
	int from_size = sizeof(from);
	int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);

	if (bytes_received == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d\n", error);
		}

		return false;
	}
	return true;
}
*/
bool Server::AddNewClient(IP_Endpoint& from_endpoint, SOCKADDR_IN& from)
{
	printf("Client_Message::Join from %d.%d.%d.%d:%d\n", from.sin_addr.S_un.S_un_b.s_b1,
		from.sin_addr.S_un.S_un_b.s_b2,
		from.sin_addr.S_un.S_un_b.s_b3,
		from.sin_addr.S_un.S_un_b.s_b4,
		from.sin_port);
	int flags = 0;
	int from_size = sizeof(from);
	uint16 slot = uint16(-1);
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (client_endpoints[i].address == 0)
		{
			slot = i;
			break;
		}
	}
	send_buf_mtx.lock();
	ZeroMemory(&buffer, SOCKET_BUFFER_SIZE);
	buffer[0] = (int8)Server_Message::Join_Result;
	if (slot != uint16(-1))
	{
		printf("client will be assigned to slot %hu\n", slot);
		buffer[1] = 1;
		memcpy(&buffer[2], &slot, 2);
		flags = 0;
		if (sendto(sock, buffer, 4, flags, (SOCKADDR*)&from, from_size) != SOCKET_ERROR)
		{
			client_endpoints[slot] = from_endpoint;
			time_since_heard_from_clients[slot] = 0.0f;
			//client_objects[slot] = {};
			client_inputs[slot] = {};
			this->RespawnPlayer(slot);
		}
		else
		{
			send_buf_mtx.unlock();
			printf("sendto failed: %d\n", WSAGetLastError());
			return false;
		}
	}
	else
	{
		printf("could not find a slot for player\n");
		buffer[1] = 0;

		flags = 0;
		if (sendto(sock, buffer, 2, flags, (SOCKADDR*)&from, from_size) == SOCKET_ERROR)
		{
			printf("sendto failed: %d\n", WSAGetLastError());
		}
		send_buf_mtx.unlock();
		return false;
	}
	send_buf_mtx.unlock();
	return true;
}

bool Server::RemoveClient(IP_Endpoint& from_endpoint, SOCKADDR_IN& from)
{

	uint16 slot;
	memcpy(&slot, &listenBuffer[1], sizeof(slot));

	if (client_endpoints[slot] == from_endpoint)
	{
		client_endpoints[slot] = {};
		printf("Client_Message::Disconnect from slot %d Address %d.%d.%d.%d:%d\n", slot, from.sin_addr.S_un.S_un_b.s_b1,
			from.sin_addr.S_un.S_un_b.s_b2,
			from.sin_addr.S_un.S_un_b.s_b3,
			from.sin_addr.S_un.S_un_b.s_b4,
			from.sin_port);
	}
	return true;
}

bool Server::SendGameStateToAll()
{
	int flags = 0;
	SOCKADDR_IN to;
	to.sin_family = AF_INET;
	to.sin_port = htons(PORT);
	int to_length = sizeof(to);

	send_buf_mtx.lock();
	ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
	FillBufferWithGameState();
	//ParseBuffer();

	/*
	buffer[0] = (uint8)Server_Message::State;
	int32 bytes_written = 1;
	uint16 numberOfObjects = 0;
	int16 slot = 0;
	float x = -1000.3;
	float y = 3.16;
	memcpy(&buffer[bytes_written], &slot, sizeof(slot));
	bytes_written += sizeof(slot);
	memcpy(&buffer[bytes_written], &x, sizeof(x));
	bytes_written += sizeof(x);
	memcpy(&buffer[bytes_written], &y, sizeof(y));
	bytes_written += sizeof(y);
	x = 0;
	int32 bytes_read = 1;
	int16 recievedSlot;
	memcpy(&recievedSlot, &buffer[bytes_read], sizeof(recievedSlot));
	bytes_read += sizeof(recievedSlot);
	float x_read;
	memcpy(&x_read, &buffer[bytes_read], sizeof(x_read));
	bytes_read += sizeof(x_read);
	float y_read;
	memcpy(&y_read, &buffer[bytes_read], sizeof(y_read));
	*/
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (client_endpoints[i].address)
		{
			to.sin_addr.S_un.S_addr = client_endpoints[i].address;
			to.sin_port = client_endpoints[i].port;

			if (sendto(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&to, to_length) == SOCKET_ERROR)
			{
				printf("sendto failed: %d\n", WSAGetLastError());
			}
		}
	}
	send_buf_mtx.unlock();
	return true;
}

void Server::FillBufferWithGameState()
{
	ZeroMemory(buffer, SOCKET_BUFFER_SIZE);
	buffer[0] = (uint8)Server_Message::State;
	int32 bytes_written = 1;
	uint16 numberOfObjects = 0;
	bytes_written += sizeof(numberOfObjects);
	// Loop - all object states
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (client_endpoints[i].address)
		{
			buffer[bytes_written++] = (uint8)Game_Object_Type::Player;
			memcpy(&buffer[bytes_written], &i, sizeof(i));
			bytes_written += sizeof(i);
			player_objects[i].UploadState(buffer, bytes_written);
			++numberOfObjects;
			for (uint16 projectil_it = 0; projectil_it < MAX_PROJECTILES; ++projectil_it)
			{
				if (projectil_objects[i * MAX_PROJECTILES + projectil_it].getProjectilStatus() == Projectil::Projectil_Status::ACTIVE)
				{
					buffer[bytes_written++] = (uint8)Game_Object_Type::Projectil;
					projectil_objects[i * MAX_PROJECTILES + projectil_it].UploadState(buffer, bytes_written);
					++numberOfObjects;
				}
				else
					continue;
			}
		}
	}
	memcpy(&buffer[1], &numberOfObjects, sizeof(numberOfObjects));
}

void Server::ParseBuffer()
{
	Server_Message type_of_message = (Server_Message)buffer[0];
	int32 bytes_read = 1;
	switch (type_of_message)
	{
	case Server_Message::State:
		//HandleState(buffer, bytes_read);
		break;
	default:
		break;
	}
}

void Server::HandlePlayerInput()
{
	uint16 playerSlot = -1;
	uint8 playerActions = 0;
	int bytesRead = 1;
	memcpy(&playerSlot, &listenBuffer[bytesRead], sizeof(playerSlot));
	bytesRead += sizeof(playerSlot);
	playerActions = listenBuffer[bytesRead++];
	time_since_heard_from_clients[playerSlot] = 0.0f;
	Player& tmpPlayer = player_objects[playerSlot];
	bool fireRequest = false;
	//printf("Player action %d\n", playerActions);
	if (playerActions & 0x10)
	{
		// handleShooting
   		//printf("Player %d shot \n", playerSlot);
		playerActions &= ~(1 << 4);
		fireRequest = true;
	}
	// handle move
	if (tmpPlayer.getPlayerStatus() == Player::Status::ALIVE)
	{
		if (playerActions)
		{
			Vector move(0,0);
			switch ((Player_Input_Action)playerActions)
			{
			case Player_Input_Action::UP:
				//printf("Player %d move up\n", playerSlot);
				move = Vector(0.0, -1 * SECONDS_PER_TICK * PLAYER_SPEED);
				//tmpPlayer.addCordY(-1 * SECONDS_PER_TICK * PLAYER_SPEED);
				tmpPlayer.setPlayerDirection(Player::Direction::UP);
				tmpPlayer.setPlayerAction(Player::Action::MOVE);
				break;
			case Player_Input_Action::DOWN:
				//printf("Player %d move down\n", playerSlot);
				move = Vector(0.0, 1 * SECONDS_PER_TICK * PLAYER_SPEED);
				//tmpPlayer.addCordY(1 * SECONDS_PER_TICK * PLAYER_SPEED);
				player_objects[playerSlot].setPlayerDirection(Player::Direction::DOWN);
				tmpPlayer.setPlayerAction(Player::Action::MOVE);
				break;
			case Player_Input_Action::LEFT:
				//printf("Player %d move left\n", playerSlot);
				move = Vector(-1 * SECONDS_PER_TICK * PLAYER_SPEED, 0.0);
				//tmpPlayer.addCordX(-1 * SECONDS_PER_TICK * PLAYER_SPEED);
				tmpPlayer.setPlayerDirection(Player::Direction::LEFT);
				tmpPlayer.setPlayerAction(Player::Action::MOVE);
				break;
			case Player_Input_Action::RIGHT:
				//printf("Player %d right\n", playerSlot);
				move = Vector(1 * SECONDS_PER_TICK * PLAYER_SPEED, 0.0);
				//tmpPlayer.addCordX(1 * SECONDS_PER_TICK * PLAYER_SPEED);
				tmpPlayer.setPlayerDirection(Player::Direction::RIGHT);
				tmpPlayer.setPlayerAction(Player::Action::MOVE);
				break;
			default:
				break;
			}
			bool notCollide = true;
			for (uint16 i = 0; i < MAX_CLIENTS; ++i)
			{
				if (!client_endpoints[i].address)
					continue;
				if (&tmpPlayer == &player_objects[i])
					continue;
				if (tmpPlayer.CheckCollision(player_objects[i], move))
				{
					notCollide = false;
					break;
				}
			}
			if (notCollide)
			{
				for (uint16 i = 0; i < MAP_BORDERS; ++i)
				{
					if (tmpPlayer.CheckCollision(map_border[i], move))
					{
						notCollide = false;
						break;
					}
				}
				if (notCollide)
				{
					tmpPlayer.Move(move);
				}
			}
		}
		else
		{
			//printf("Player %d standing\n", playerSlot);
			tmpPlayer.setPlayerAction(Player::Action::IDLE);
		}
		if (fireRequest)
		{
			if (tmpPlayer.readyToFire() && tmpPlayer.getPlayerAmmo() > 0)
			{
				uint16 offset = playerSlot * MAX_PROJECTILES;
				for (uint16 i = 0; i < MAX_PROJECTILES; ++i)
				{
					if (projectil_objects[offset + i].getProjectilStatus() == Projectil::DISABLED)
					{
						projectil_objects[offset + i].Activate(tmpPlayer.getBody(), (Projectil::Projectil_Direction)tmpPlayer.getPlayerDirection());
						printf("Projectil %d of player %d was Activated \n", projectil_objects[offset + i].getProjectilNumber(), projectil_objects[offset + i].getOwnerSlot());
						tmpPlayer.Shoot();
						break;
					}
				}
			}
		}
	}
	else
	{
		tmpPlayer.setPlayerAction(Player::Action::KILLED);
	}


}

void Server::UpdateGame()
{
	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		if (client_endpoints[i].address)
		{
			has_player = true;
			time_since_heard_from_clients[i] += SECONDS_PER_TICK;
			if (time_since_heard_from_clients[i] > CLIENT_TIMEOUT)
			{
				printf("client %hu timed out\n", i);
				client_endpoints[i] = {};
			}
			else
			{
				Player& tmpPlayer = player_objects[i];
				if (tmpPlayer.getPlayerStatus() == Player::Status::ALIVE)
				{
					if (!tmpPlayer.readyToFire())
					{
						if (tmpPlayer.getPlayerAmmo() <= 0)
						{
							if (tmpPlayer.getReloadTime() >= RELOAD_TIME)
							{
								tmpPlayer.setReloadTime(0);
								tmpPlayer.Reload();
								printf("Player %d reloaded\n", i);
							}
							else
								tmpPlayer.Update(SECONDS_PER_TICK);
						}
						else
						{
							if (tmpPlayer.getLastShotTime() >= RECOIL_TIME)
							{
								tmpPlayer.setShotTime(0);
								tmpPlayer.setReadyToFire(true);
								printf("Player %d ready to shoot \n", i);
							}
							else
								tmpPlayer.Update(SECONDS_PER_TICK);
						}
					}
				}
				else
				{
					if (tmpPlayer.getRespawnTime() >= RESPAWN_TIME)
					{
						this->RespawnPlayer(i);
					}
					else
						tmpPlayer.Update(SECONDS_PER_TICK);
				}
				for (uint16 i_projectil = 0; i_projectil < MAX_PROJECTILES; ++i_projectil)
				{
					Projectil& tmpProjectil = projectil_objects[i * MAX_PROJECTILES + i_projectil];
					if (tmpProjectil.getProjectilStatus() == Projectil::Projectil_Status::ACTIVE)
					{
						Vector projectilPosition = tmpProjectil.getPosition();

						if (projectilPosition.x < -(MAP_WIDTH / 2)
							|| projectilPosition.x >(MAP_WIDTH / 2)
							|| projectilPosition.y < -(MAP_HEIGHT / 2)
							|| projectilPosition.y > +(MAP_HEIGHT / 2))
						{
							tmpProjectil.setProjectilStatus(Projectil::Projectil_Status::DISABLED);
						}
						else
						{
							tmpProjectil.moveInDirection(SECONDS_PER_TICK * PROJECTIL_SPEED);
						}
					}
				}
				
			}
		}
	}

}
void Server::RespawnPlayer(uint16 playerSlot)
{
	float inerval_down_X = 0;
	float inerval_up_X = 0;
	float inerval_down_Y = 0;
	float inerval_up_Y = 0;
	Player& tmpPlayer = player_objects[playerSlot];
	Vector tmpPlayerSize = tmpPlayer.getBody().getSize();
	int possibleWidth = (int)MAP_WIDTH - tmpPlayerSize.x;
	int possibleHeight = (int)MAP_HEIGHT - tmpPlayerSize.y;
	printf("Od %.2f  po   %0.2f \n", 0 - possibleWidth / 2.0f, possibleWidth - possibleWidth / 2.0f);
	printf("Od %.2f  po   %0.2f \n", 0 - possibleHeight / 2.0f, possibleHeight - possibleHeight / 2.0f);
	float randX = rand() % possibleWidth - possibleWidth / 2.0f;
	float randY = rand() % possibleHeight - possibleHeight / 2.0f;
	printf("X: %.2f Y%.2f \n", randX, randY);
	tmpPlayer.getBody().setPosition(randX, randY);
}
/*
void Server::HandleState(int8 * buffer, int32 bytes_read)
{
	uint16 number_of_objects;
	uint16 player_slot;
	uint8 projectil_number;
	Game_Object_Type game_object_type;
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
			player_objects[player_slot].InsertState(buffer, bytes_read);
			break;
		case Game_Object_Type::Projectil:
			memcpy(&player_slot, &buffer[bytes_read], sizeof(player_slot));
			bytes_read += sizeof(player_slot);
			projectil_number = buffer[bytes_read++];
			projectil_objects[player_slot + projectil_number].InsertState(buffer, bytes_read);
			break;
		default:
			break;
		}
	}

}
*/
Server::~Server()
{
	WSACleanup();
}
