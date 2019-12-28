#include "Animation.h"
#include <iostream>

Animation::Animation(sf::Texture* texture, sf::Vector2u imageCount, float switchTime)
{
	this->imageCount = imageCount;
	this->switchTime = switchTime;
	this->totalTime = 0.0f;
	currentImg.x = 0;
	uvRect.width = texture->getSize().x / float(imageCount.x);
	uvRect.height = texture->getSize().y / float(imageCount.y);
}

void Animation::Update(int row, float deltaTime, bool faceRight)
{
	currentImg.y = row;
	//std::cout << totalTime << std::endl;
	totalTime += deltaTime;
	if (totalTime >= switchTime)
	{
		totalTime -= switchTime;
		currentImg.x++;
		if (currentImg.x >= imageCount.x)
		{
			currentImg.x = 0;
		}
	}

	uvRect.top = currentImg.y * uvRect.height;
	if (faceRight)
	{
		uvRect.left = currentImg.x * uvRect.width;
		uvRect.width = abs(uvRect.width);
	}
	else
	{
		uvRect.left = (currentImg.x + 1) * abs(uvRect.width);
		uvRect.width = -abs(uvRect.width);
	}


}