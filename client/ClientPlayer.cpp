#include "ClientPlayer.h"

ClientPlayer::ClientPlayer(sf::Vector2f bodyPosition, sf::RenderWindow * window, std::string texturePath, float switchTime, sf::Vector2u imageCount)
	: DrawableObject(sf::Vector2f(0.0f, 0.0f), bodyPosition, window)
{
	texture = new sf::Texture();
	texture->loadFromFile(texturePath);
	animation = new Animation(texture, imageCount, switchTime);


	float ratio = 1;

	float sirka = 50.0f * ratio;
	float vyska = 70.0f * ratio;


	body.setSize(sf::Vector2f(sirka, vyska));
	body.setOrigin(body.getSize() / 2.0f);
	body.setPosition(bodyPosition);
	body.setTexture(this->texture);
	body.setTextureRect(animation->uvRect);
	playerStatus = ClientPlayer::Status::ALIVE;
	playerDirection = ClientPlayer::Direction::DOWN;
}

ClientPlayer::Status ClientPlayer::getPlayerStatus()
{
	return playerStatus;
}

ClientPlayer::Direction ClientPlayer::getPlayerDirection()
{
	return playerDirection;
}

ClientPlayer::Action ClientPlayer::getPlayerAction()
{
	return playerAction;
}

ClientPlayer::Activity ClientPlayer::getPlayerActivity()
{
	return playerActivity;
}

uint8 ClientPlayer::getPlayerAmmo()
{
	return ammo;
}

void ClientPlayer::setCordX(float x)
{
	cordX = x;
}

void ClientPlayer::setCordY(float y)
{
	cordY = y;
}

void ClientPlayer::setPlayerActivity(ClientPlayer::Activity activity)
{
	this->playerActivity = activity;
}

float ClientPlayer::getPlayerCordX()
{
	return cordX;
}

float ClientPlayer::getPlayerCordY()
{
	return cordY;
}

void ClientPlayer::UploadState(int8 * buffer, int32 & bytes_written)
{
	buffer[bytes_written++] = playerStatus;
	buffer[bytes_written++] = playerDirection;
	buffer[bytes_written++] = playerAction;
	buffer[bytes_written++] = ammo;
	memcpy(&buffer[bytes_written], &cordX, sizeof(cordX));
	bytes_written += sizeof(cordX);
	memcpy(&buffer[bytes_written], &cordY, sizeof(cordY));
	bytes_written += sizeof(cordY);
}

void ClientPlayer::InsertState(int8 * buffer, int32 & bytes_read)
{
	playerStatus = (ClientPlayer::Status)buffer[bytes_read++];
	playerDirection = (ClientPlayer::Direction)buffer[bytes_read++];
	playerAction = (ClientPlayer::Action)buffer[bytes_read++];
	ammo = buffer[bytes_read++];
	memcpy(&cordX, &buffer[bytes_read], sizeof(cordX));
	bytes_read += sizeof(cordX);
	memcpy(&cordY, &buffer[bytes_read], sizeof(cordY));
	bytes_read += sizeof(cordY);
	this->playerActivity = ClientPlayer::Activity::ACTIVE;
}

void ClientPlayer::Update(double deltaTime)
{
	short unsigned int vyslednaAnimacia = 0;
	if(playerStatus == ClientPlayer::Status::ALIVE)
		vyslednaAnimacia = playerDirection * 2 + playerAction;
	else
		vyslednaAnimacia = ClientPlayer::Action::KILLED;
	this->animation->Update(vyslednaAnimacia, deltaTime, faceRight);
	this->body.setTextureRect(animation->uvRect);
	this->body.setPosition(cordX, cordY);

}


ClientPlayer::~ClientPlayer()
{
	delete texture;
	delete animation;
}
