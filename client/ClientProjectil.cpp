#include "ClientProjectil.h"



ClientProjectil::ClientProjectil()
	: DrawableObject(sf::Vector2f(12.0f, 12.0f), sf::Vector2f(0.0f, 0.0f), nullptr)
{
}

ClientProjectil::Projectil_Status ClientProjectil::getProjectilStatus()
{
	return m_projectilStatus;
}

ClientProjectil::Projectil_Direction ClientProjectil::getProjectilDirection()
{
	return m_projectilDirection;
}

uint16 ClientProjectil::getOwnerSlot()
{
	return m_playerSlot;
}

uint8 ClientProjectil::getProjectilNumber()
{
	return m_projectilNumber;
}

float ClientProjectil::getCordX()
{
	return body.getPosition().x;
}

float ClientProjectil::getCordY()
{
	return body.getPosition().y;
}

void ClientProjectil::setProjectilStatus(ClientProjectil::Projectil_Status status)
{
	m_projectilStatus = status;
}

void ClientProjectil::setProjectilDirection(ClientProjectil::Projectil_Direction direction)
{
	m_projectilDirection = direction;
}

void ClientProjectil::setOwnerSlot(uint16 ownerSlot)
{
	m_playerSlot = ownerSlot;
}

void ClientProjectil::setProjectilNumber(uint8 projectilNum)
{
	m_projectilNumber = projectilNum;
}

void ClientProjectil::setCordX(float x)
{
	body.setPosition(x, body.getPosition().y);
}

void ClientProjectil::setCordY(float y)
{
	body.setPosition(body.getPosition().x, y);
}

void ClientProjectil::UploadState(int8 * buffer, int32 & bytes_written)
{
	memcpy(&buffer[bytes_written], &m_playerSlot, sizeof(m_playerSlot));
	bytes_written += sizeof(m_playerSlot);
	buffer[bytes_written++] = m_projectilNumber;
	buffer[bytes_written++] = m_projectilStatus;
	buffer[bytes_written++] = m_projectilDirection;
	float tmp = body.getPosition().x;
	memcpy(&buffer[bytes_written], &tmp, sizeof(tmp));
	bytes_written += sizeof(tmp);
	tmp = body.getPosition().y;
	memcpy(&buffer[bytes_written], &tmp, sizeof(tmp));
	bytes_written += sizeof(tmp);
}

void ClientProjectil::InsertState(int8 * buffer, int32 & bytes_read)
{
	m_projectilStatus = (ClientProjectil::Projectil_Status)buffer[bytes_read++];
	m_projectilDirection = (ClientProjectil::Projectil_Direction)buffer[bytes_read++];
	float tmpX = 0;
	memcpy(&tmpX, &buffer[bytes_read], sizeof(tmpX));
	this->setCordX(tmpX);
	bytes_read += sizeof(tmpX);
	float tmpY = 0;
	memcpy(&tmpY, &buffer[bytes_read], sizeof(tmpY));
	this->setCordY(tmpY);
	bytes_read += sizeof(tmpY);
}


ClientProjectil::~ClientProjectil()
{
}
