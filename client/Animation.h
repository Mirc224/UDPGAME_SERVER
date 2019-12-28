#pragma once
#include <SFML/Graphics.hpp>

class Animation {
public:
	Animation(sf::Texture* texture, sf::Vector2u imageCount, float switchTime);
	~Animation() {};

	void Update(int row, float deltaTime, bool faceRight = false);

public:
	sf::IntRect uvRect;
private:
	sf::Vector2u imageCount;
	sf::Vector2u currentImg;
	float totalTime;
	float switchTime;
};