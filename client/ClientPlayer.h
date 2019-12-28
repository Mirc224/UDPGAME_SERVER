#pragma once
#include "Odin.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string>
#include "DrawableObject.h"
#include "Animation.h"
class ClientPlayer : public DrawableObject
{
public:
	enum Status : uint8 { ALIVE, DEATH };
	enum Direction : uint8 { DOWN, UP, LEFT, RIGHT };
	enum Action : uint8 { IDLE, MOVE, KILLED = 8 };
	enum Activity: uint8 { ACTIVE, DISABLED};
	ClientPlayer(sf::Vector2f bodyPosition, sf::RenderWindow* window, std::string texturePath,float switchTime, sf::Vector2u imageCount);
	ClientPlayer::Status getPlayerStatus();
	ClientPlayer::Direction getPlayerDirection();
	ClientPlayer::Action getPlayerAction();
	ClientPlayer::Activity getPlayerActivity();
	uint8 getPlayerAmmo();
	void setCordX(float x);
	void setCordY(float y);
	void setPlayerActivity(ClientPlayer::Activity activity);
	float getPlayerCordX();
	float getPlayerCordY();
	void UploadState(int8* buffer, int32& bytes_written);
	void InsertState(int8* buffer, int32& bytes_read);
	void Update(double deltaTime);
	virtual ~ClientPlayer();
private:
	Status playerStatus;
	Direction playerDirection;
	Action playerAction;
	Activity playerActivity = ClientPlayer::Activity::DISABLED;
	uint8 ammo;
	float cordX;
	float cordY;
	Animation* animation;
	sf::Texture* texture;
	bool faceRight = false;
};

