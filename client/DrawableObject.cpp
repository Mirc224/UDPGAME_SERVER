#include "DrawableObject.h"

DrawableObject::DrawableObject(sf::Vector2f bodySize, sf::Vector2f bodyPosition, sf::RenderWindow * window)
{
	this->window = window;

	this->body.setSize(bodySize);

	this->body.setOrigin(bodySize / 2.0f);

	this->body.setPosition(bodyPosition);
}

void DrawableObject::setCoords(sf::Vector2f vector)
{
	this->body.setPosition(vector);
}

void DrawableObject::draw()
{
	this->window->draw(this->body);
}

void DrawableObject::setPosition(sf::Vector2f position)
{
	body.setPosition(position);
}

void DrawableObject::setOrigin(sf::Vector2f origin)
{
	body.setOrigin(origin);
}

void DrawableObject::setSize(sf::Vector2f size)
{
	body.setSize(size);
}

void DrawableObject::setWindow(sf::RenderWindow * win)
{
	this->window = win;
}
