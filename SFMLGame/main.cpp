#include <SFML/Graphics.hpp>

#include "Game.h"
#include "Vec2.h"

int main()
{
	Game game("config.txt");
	game.run();
	//Vec2<float> a(1, 2);
	//Vec2<float> b(3, 4);
	//Vec2<float> c = a + b;
	//printf("%.2f, %.2f", c.x, c.y);

    return 0;
}