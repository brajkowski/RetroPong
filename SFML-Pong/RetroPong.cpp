// SFMLPong.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "SFML\Graphics.hpp"
#include <Windows.h>
using namespace sf;
using namespace std;

Clock hitTime;
Clock gameTime;

enum eDir { STOP = 0, LEFT = 1, UPLEFT = 2, DOWNLEFT = 3, RIGHT = 4, UPRIGHT = 5, DOWNRIGHT = 6 };
enum eAICode { AISTOP = 0, AIRESETP2 = 1, AITRACK = 2 };

class Ball
{
private:
	Vector2f coord;
	Vector2f origin;
	CircleShape sBall;
	Rect<float> rBall;
	eDir direction;
	Vector2f dVector;

public:
	//Constructor
	Ball(float xOrig, float yOrig, float dia)
	{
		origin.x = xOrig, origin.y = yOrig;
		coord.x = xOrig, coord.y = yOrig;
		direction = STOP;
		dVector = { 0,0 };
		sBall.setRadius(dia);
		sBall.setPosition(coord);
		rBall = sBall.getGlobalBounds();
	}
	
	//Accessors
	inline float getX() { return coord.x; }
	inline float getY() { return coord.y; }
	inline eDir getDir() { return direction; }
	inline CircleShape getShape() { return sBall; }
	inline Rect<float> getBounds() { return rBall; }
	inline Vector2f getDirection() { return dVector; }

	//Mutators
	void changeDirection(eDir d) { direction = d; }
	void reset()
	{
		direction = STOP;
		dVector = { 0,0 };
		coord = origin;
		sBall.setPosition(coord);
		rBall = sBall.getGlobalBounds();
	}
	inline void wallBounce() { dVector.y = -dVector.y; }
	void paddleBounce(Vector2f paddleCoord, float paddleHeight, float ballSpeed)
	{
		if (hitTime.getElapsedTime().asMilliseconds() > 100)
		{
			float angle = 134 / paddleHeight * (coord.y + 10/*offset*/ - paddleCoord.y) - 67; //134 and 67 come from +/- 67 degrees for the ball direction

			if (angle < -50) angle = -50;
			if (angle > 50) angle = 50;
			angle = float(3.14) * angle / 180;
			if (dVector.x > 0) dVector.x = -ballSpeed * fabs(cos(angle));
			else dVector.x = ballSpeed * fabs(cos(angle));
			dVector.y = ballSpeed * sin(angle);
			hitTime.restart();
		}
	}
	void physicsMove()
	{
		coord.x = coord.x + dVector.x;
		coord.y = coord.y + dVector.y;
		sBall.setPosition(coord);
		rBall = sBall.getGlobalBounds();
	}
	void randomDirection(float ballSpeed)
	{
		srand(time(NULL));
		dVector.y = rand() % 3;
		if ((rand() % 10 + 1) % 2 > 0) { dVector.y = -dVector.y; }
		dVector.x = sqrtf((ballSpeed * ballSpeed) - (dVector.y * dVector.y));
		if ((rand() % 10 + 1) % 2 > 0) { dVector.x = -dVector.x; }
		//dVector.x = ballSpeed;
	}
};

class Paddle
{
private:
	Vector2f coord;
	Vector2f origin;
	Vector2f dim;
	RectangleShape sPaddle;
	Rect<float> rPaddle;

public:
	//Constructor
	Paddle(float xOrig, float yOrig, float w, float h)
	{
		origin.x = xOrig, origin.y = yOrig;
		coord.x = xOrig, coord.y = yOrig;
		dim.x = w, dim.y = h;
		sPaddle.setPosition(coord);
		sPaddle.setSize(dim);
		rPaddle = sPaddle.getGlobalBounds();
	}

	//Accessors
	inline float getX() { return coord.x; }
	inline float getY() { return coord.y; }
	inline Vector2f getCoords() { return coord; }
	inline RectangleShape getShape() { return sPaddle; }
	inline Rect<float> getBounds() { return rPaddle; }
	inline float getOriginY() { return origin.y; }

	//Mutators
	void moveUp(float s)
	{
		coord.y = coord.y - s;
		sPaddle.setPosition(coord);
		rPaddle = sPaddle.getGlobalBounds();
	}
	void moveDown(float s)
	{
		coord.y = coord.y + s;
		sPaddle.setPosition(coord);
		rPaddle = sPaddle.getGlobalBounds();
	}
	void reset()
	{
		coord = origin;
		sPaddle.setPosition(coord);
		rPaddle = sPaddle.getGlobalBounds();
	}


};

int main()
{
	//Disable the console
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	//Set game environment parameters
	int mode = 0;
	float gameWidth = 800;
	float gameHeight = 500;
	int frameLimit = 120;
	float ballSpeed = 700 / float(frameLimit);
	float paddleSpeed = 500 / float(frameLimit);
	float startBallSpeed = ballSpeed;
	float maxSpeed = 1300;
	float ballDiameter = 10;
	float playerWidth = 15;
	float playerHeight = 80;
	int score1, score2;
	score1 = score2 = 0;
	eAICode AICode = AISTOP;

	//Create game objects
	RenderWindow gameEnvironment(VideoMode(int(gameWidth), int(gameHeight)), "Retro Pong by BTR");
	gameEnvironment.setFramerateLimit(frameLimit);
	Event eInput;
	Ball gameBall(gameWidth/2-ballDiameter, gameHeight/2-ballDiameter, ballDiameter);
	Paddle player1(0, gameHeight / 2 - playerHeight / 2, playerWidth, playerHeight);
	Paddle player2(gameWidth - playerWidth, gameHeight / 2 - playerHeight / 2, playerWidth, playerHeight);
	//Half court line
	RectangleShape midLine({ 2,gameHeight });
	midLine.setPosition({ gameWidth / 2 - 1,0 });
	midLine.setFillColor(Color::Color(180,180,180,125));
	//Game boundary rectangles
	Rect<float> uBound(0, 0, gameWidth, 1);
	Rect<float> bBound(0, gameHeight, gameWidth, -1);
	Rect<float> lBound(0, 0, 1, gameHeight);
	Rect<float> rBound(gameWidth, 0, -1, gameHeight);
	//Score texts and font
	Font gothic;
	if (!gothic.loadFromFile("Fonts/gothic.ttf")) { cout << "Cannot load font." << endl; }
	Text tScore1, tScore2;
	tScore1.setFont(gothic), tScore2.setFont(gothic);
	tScore1.setString("0"), tScore2.setString("0");
	tScore1.setPosition({ gameWidth / 4 - 23,10 });
	tScore2.setPosition({ 3 * gameWidth / 4 + 5,10 });

	//Create splash objects
	Mouse mouse;
	Font agency;
	if (!agency.loadFromFile("Fonts/agencyr_0.ttf")) { cout << "Cannot load font." << endl; }
	Text title("RETRO PONG", agency);
	Text subTitle("Created By: BTR", agency);
	Text p1("1 - Player", agency);
	Text p2("2 - Player", agency);
	Text controls("Controls", agency);
	title.setCharacterSize(100);
	title.setPosition(gameWidth / 2 - 190, 20);
	subTitle.setCharacterSize(20);
	subTitle.setPosition(gameWidth / 2 - 50, 150);
	subTitle.setFillColor(Color::Color(100, 100, 100, 255));
	p1.setCharacterSize(50);
	p2.setCharacterSize(50);
	p1.setPosition(gameWidth / 2 - 75, 200);
	p2.setPosition(gameWidth / 2 - 75, 300);
	controls.setCharacterSize(50);
	controls.setPosition(gameWidth / 2 - 75, 400);

	//Main game environment loop
	while (gameEnvironment.isOpen())
	{
		//SPLASH SCREEN
		//Reset the text color here so mouseover works
		p1.setFillColor(Color::White);
		p2.setFillColor(Color::White);
		controls.setFillColor(Color::White);
		Vector2u currentSize;
		currentSize = gameEnvironment.getSize();

		//Splash screen window management
		while (gameEnvironment.pollEvent(eInput))
		{

			if (eInput.type == Event::Closed) { gameEnvironment.close(); }
			if (eInput.type == Event::Resized) { gameEnvironment.setSize(currentSize); }

				currentSize = gameEnvironment.getSize();
		}
		if (Keyboard::isKeyPressed(Keyboard::Escape)) { gameEnvironment.close(); }

		//Check for mouseover text and start 1 player game if clicked
		if (p1.getGlobalBounds().contains(float(mouse.getPosition(gameEnvironment).x), float(mouse.getPosition(gameEnvironment).y)))
		{
			p1.setFillColor(Color::Color(100, 100, 100, 255));
			if (Mouse::isButtonPressed(Mouse::Left)) { mode = 1; }
		}
		//Check for mouseover text and start 2 player game if clicked
		if (p2.getGlobalBounds().contains(float(mouse.getPosition(gameEnvironment).x), float(mouse.getPosition(gameEnvironment).y)))
		{
			p2.setFillColor(Color::Color(100, 100, 100, 255));
			if (Mouse::isButtonPressed(Mouse::Left)) { mode = 2; }
		}
		//Check for mouseover text and load instruction screen if clicked
		if (controls.getGlobalBounds().contains(float(mouse.getPosition(gameEnvironment).x), float(mouse.getPosition(gameEnvironment).y)))
		{
			controls.setFillColor(Color::Color(100, 100, 100, 255));
			if (Mouse::isButtonPressed(Mouse::Left))
			{
				Text instr("Text", agency);
				instr.setString("Player 1 \
				\nUp: W\
				\nDown: S\
				\n\
				\nPlayer 2\
				\nUp: Up Arrow Key\
				\nDown: Down Arrow Key\
				\n\
				\nStart Game: Space\
				\nReset Game: R\
				\nPrevious Screen: Q\
				\nExit Program: ESC\
				");
				instr.setPosition(gameWidth / 2 - 100, 30);
					
				//General instruction window management
				while (!Keyboard::isKeyPressed(Keyboard::Q) && gameEnvironment.isOpen())
				{
					while(gameEnvironment.pollEvent(eInput))
						if (eInput.type == Event::Closed) { gameEnvironment.close(); }

					if (Keyboard::isKeyPressed(Keyboard::Escape)) { gameEnvironment.close(); }
						
					//Display for instruction window
					gameEnvironment.clear();
					gameEnvironment.draw(instr);
					gameEnvironment.display();
				}
			}
		}

		//Display for splash screen
		gameEnvironment.clear();
		gameEnvironment.draw(title);
		gameEnvironment.draw(subTitle);
		gameEnvironment.draw(p1);
		gameEnvironment.draw(p2);
		gameEnvironment.draw(controls);
		gameEnvironment.display();

		//GAME LOOP
		while (gameEnvironment.isOpen() && !mode == 0)
		{
			//Random seed for random ball direction
			srand(int(time(0)));

			//Take event poll
			while (gameEnvironment.pollEvent(eInput))
				if (eInput.type == Event::Closed) { gameEnvironment.close(); }

			//Input
			if (Keyboard::isKeyPressed(Keyboard::Escape)) { gameEnvironment.close(); }
			if (Keyboard::isKeyPressed(Keyboard::Q)) { player1.reset(), player2.reset(), gameBall.changeDirection(STOP), gameBall.reset(), score1 = 0, score2 = 0, mode = 0, ballSpeed = startBallSpeed; }
			if (Keyboard::isKeyPressed(Keyboard::R)) { player1.reset(), player2.reset(), gameBall.changeDirection(STOP), gameBall.reset(), score1 = 0, score2 = 0, ballSpeed = startBallSpeed; }
			if (Keyboard::isKeyPressed(Keyboard::W) && !player1.getBounds().intersects(uBound)) { player1.moveUp(paddleSpeed); }
			if (Keyboard::isKeyPressed(Keyboard::S) && !player1.getBounds().intersects(bBound)) { player1.moveDown(paddleSpeed); }
			if (Keyboard::isKeyPressed(Keyboard::Up) && !player2.getBounds().intersects(uBound)) { if (mode == 2) player2.moveUp(paddleSpeed); }		//Cannot move player 2 during 1-player game
			if (Keyboard::isKeyPressed(Keyboard::Down) && !player2.getBounds().intersects(bBound)) { if (mode == 2) player2.moveDown(paddleSpeed); }	//Cannot move player 2 during 1-player game
			if (Keyboard::isKeyPressed(Keyboard::Space) && gameBall.getDirection().x == 0) { ballSpeed = startBallSpeed, gameBall.randomDirection(ballSpeed), gameTime.restart(), hitTime.restart(); }

			//Collision logic and generate AI codes
			if (gameBall.getBounds().intersects(uBound)) { gameBall.wallBounce(); }
			else if (gameBall.getBounds().intersects(bBound)) { gameBall.wallBounce(); }
			else if (gameBall.getBounds().intersects(lBound)) { player1.reset(), player2.reset(), gameBall.reset(), ballSpeed = startBallSpeed, score2++; }
			else if (gameBall.getBounds().intersects(rBound)) { player1.reset(), player2.reset(), gameBall.reset(), ballSpeed = startBallSpeed, score1++; }
			else if (gameBall.getBounds().intersects(player1.getBounds())) { gameBall.paddleBounce(player1.getCoords(), playerHeight, ballSpeed); }
			else if (gameBall.getBounds().intersects(player2.getBounds()))
			{
				gameBall.paddleBounce(player2.getCoords(), playerHeight, ballSpeed);
				if (mode == 1) AICode = AIRESETP2;
			}

			//Start tracking statement
			if (mode == 1 && gameBall.getDirection().x > 0 && gameBall.getX() > gameWidth - gameWidth / 3) AICode = AITRACK;

			//AI methods
			switch (AICode)
			{
			case AISTOP:
				break;
			case AIRESETP2:
				if (player2.getY() != player2.getOriginY())
				{
					if (player2.getY() > player2.getOriginY()) { player2.moveUp(paddleSpeed); }
					else player2.moveDown(paddleSpeed);
				}
				else AICode = AISTOP;
				break;
			case AITRACK:
				if (gameBall.getY() > player2.getY() + playerHeight / 2 && !player2.getBounds().intersects(bBound)) { player2.moveDown(paddleSpeed); }
				else if (gameBall.getY() < player2.getY() + playerHeight / 2 && !player2.getBounds().intersects(uBound)) { player2.moveUp(paddleSpeed); }
				AICode = AISTOP;
				break;
			}
			//Increase ball speed as time progresses
			if (gameTime.getElapsedTime().asSeconds() > 1 && ballSpeed < maxSpeed)
			{
				ballSpeed += 0.2;
				gameTime.restart();
			}

			//Change score text and move ball
			tScore1.setString(to_string(score1));
			tScore2.setString(to_string(score2));
			gameBall.physicsMove();

			//Display
			gameEnvironment.clear();
			gameEnvironment.draw(midLine);
			gameEnvironment.draw(gameBall.getShape());
			gameEnvironment.draw(player1.getShape());
			gameEnvironment.draw(player2.getShape());
			gameEnvironment.draw(tScore1);
			gameEnvironment.draw(tScore2);
			gameEnvironment.display();
		}
	}
	return 0;
}