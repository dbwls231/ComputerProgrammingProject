#include <bangtal.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

#define UPDATETIME	0.01f
#define BALL 1
#define BOMB 2

SceneID lobbyScene, playScene;
ObjectID player;
ObjectID playButton, exitButton;
SoundID inGameSound, deathSound, bombSound, ballSound, countdownSound;



int ballN = 0;
int dx = 0, dy = 0;
int bombAnimN = -1, playerAnimN=1;


float playerSpeed = 7, playerX = 600, playerY = 300;
float spawnTimer = 0, spawnTime = 5, scoreTimer=0, bombAnimTime = 1.5, playerAnimTimer=0;

bool isPlaying = false, isBombOn=false;


ObjectID createObject(const char* image, SceneID scene, int x, int y, bool shown = true)
{
	ObjectID object = createObject(image);
	locateObject(object, scene, x, y);

	if (shown) {
		showObject(object);
	}

	return object;
}

class Ball
{
public:
	int x, y = 690;
	void respawn();
	void move();
	ObjectID ball;
private:
	float dt, dy;
};

Ball balls[8];

void Ball::respawn() {
	srand((unsigned int)time(NULL));
	x = rand() % 1100 + 70;
	y = 690;
	ball=createObject("Images/ball.png",playScene,x, y);
	showObject(ball);
	if ((rand() % 10) < 5) {
		dt = (rand() % 30 + 35) / 10;
		dy = 10 - dt;
	}
	else {
		dt = -1 * (rand() % 30 + 35) / 10;
		dy = -1 * (10 + dt);
	}
}

void Ball::move() {
	x += dt;
	y += dy;
	locateObject(ball, playScene, x, y);
	if (x < 0) {
		dt *= -1;
		x = 0;
		playSound(ballSound);
	}
	else if (x > 1250) {
		dt *= -1;
		x = 1250;
		playSound(ballSound);
	}
	if (y > 690) {
		dy *= -1;
		y = 690;
		playSound(ballSound);
	}
	else if (y < 0) {
		dy *= -1;
		y = 0;
		playSound(ballSound);
	}
}

//게임 끝 : 점수계산, 씬 바꾸기
void endGame(float score) {
	stopSound(inGameSound);
	stopSound(countdownSound);
	playSound(deathSound);
	isPlaying = false;
	enterScene(lobbyScene);
	string str = to_string((int)score);
	str += "점 획득";
	const char* c = str.c_str();
	showMessage(c);
	
	
}

class Bomb
{
public:
	int x,y;
	void respawn();
	void animation(int i);
	ObjectID bomb;
};

Bomb bomb;

void startGame() {
	for (int i = 0; i < 8; i++)
		hideObject(balls[i].ball);

	hideObject(bomb.bomb);

	isPlaying = true;
	ballN = 1;
	balls[0].respawn();

	dx = 0, dy = 0;
	bombAnimN = -1;
	playerX = 600, playerY = 300;
	spawnTimer = 0, spawnTime = 6, scoreTimer = 0, bombAnimTime = 1.5;
	isBombOn = false;

	enterScene(playScene);
	playSound(inGameSound);
}

void Bomb::respawn() {
	x = rand() % 800 + 50;
	y = rand() % 250 + 50;
	locateObject(bomb, playScene, x, y);
	showObject(bomb);
}

float calculateDistance(float x1, float x2, float y1, float y2) {
	return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

bool checkCollision(int n) {
	if (n == BALL) {
		for (int i = 0; i < ballN; i++) {
			if (calculateDistance(playerX + 50, balls[i].x + 15, playerY + 37.5, balls[i].y + 15) <= 2809) {
				return true;
			}
		}
	}
	else if (n == BOMB) {
		if (calculateDistance(playerX + 50, bomb.x + 200, playerY + 37.5, bomb.y + 200)<= 56644)
			return true;
	}


	return false;
}

void Bomb::animation(int i) {
	if (i == 6) {
		hideObject(bomb);
		bombAnimTime = 1.5;
		bombAnimN = -1;
		setObjectImage(bomb,"Images/redCircle.png");
		isBombOn = false;
		return;
	}
	else if (i <4&&i>1) {
		if (checkCollision(BOMB)) {
			endGame(scoreTimer);
		}
	}
	else if (i == 1) {
		stopSound(countdownSound);
		playSound(bombSound);
	}
	
	string str;
	const char* c;
	str = "Images/bomb";
	str += to_string(i);
	str += ".png";
	c = str.c_str();
	setObjectImage(bomb, c);

}


//플레이어 이동
void keyboardCallback(KeyCode code, KeyState state) {
	if (code == KeyCode::KEY_UP_ARROW) {			// UP
		dy += (state == KeyState::KEY_PRESSED ? playerSpeed : -playerSpeed);
	}
	else if (code == KeyCode::KEY_DOWN_ARROW) {		// DOWN
		dy -= (state == KeyState::KEY_PRESSED ? playerSpeed : -playerSpeed);
	}
	else if (code == KeyCode::KEY_RIGHT_ARROW) {		// RIGHT
		dx += (state == KeyState::KEY_PRESSED ? playerSpeed : -playerSpeed);
	}
	else if (code == KeyCode::KEY_LEFT_ARROW) {		// LEFT
		dx -= (state == KeyState::KEY_PRESSED ? playerSpeed : -playerSpeed);
	}
}

void mouseCallback(ObjectID object, int x, int y, MouseAction action) {
	if (object == playButton) {
		startGame();
	}
	else if (object == exitButton) {
		endGame();
	}
}

void playerMove() {
	playerX += dx;
	playerY += dy;

	if (playerX < 0)
		playerX = 0;
	else if (playerX > 1180)
		playerX = 1180;

	if (playerY < 0)
		playerY = 0;
	else if (playerY > 645)
		playerY = 645;

	locateObject(player, playScene, playerX, playerY);
}

void obstacleMg() {
	if (spawnTimer > spawnTime) {
		//ball spawn
		if (ballN < 8) { 
			if (spawnTime > 3)
				spawnTime -= 0.5f;
			setTimer(spawnTimer, spawnTime);
			balls[ballN].respawn();
			ballN++;
			spawnTimer = 0;
			
		}
		//bomb spawn(공 8개 다 나오면 폭발 시작)
		else if(!isBombOn){
			if (spawnTime > 2.5)
				spawnTime -= 0.5f;
			isBombOn = true;
			setTimer(spawnTimer, spawnTime);
			bomb.respawn();
			spawnTimer = 0;
		}
	}

	//bomb animation
	if (isBombOn) {
		if (bombAnimN == -1) {
			playSound(countdownSound);
			bombAnimN = 0;
		}
		bombAnimTime -= 0.01f;
		if (bombAnimTime < 0) {
			
			bombAnimTime = 0.07f;
			bombAnimN++;
			bomb.animation(bombAnimN);
		}
	}
	

	//ball moving
	for (int i = 0; i < ballN; i++) {
		balls[i].move();
	}
	
	
}

void playerAnim() {
	if (playerAnimN == 1) {
		playerAnimN = 2;
	}
	else {
		playerAnimN = 1;
	}
	string str;
	const char* c;
	str = "Images/player";
	str += to_string(playerAnimN);
	str += ".png";
	c = str.c_str();
	setObjectImage(player, c);
	playerAnimTimer = 0;
}

void timerCallback(TimerID timer) {

	setTimer(timer, UPDATETIME);
	startTimer(timer);

	if (isPlaying) {
		

		spawnTimer += 0.01f;
		scoreTimer += 50;
		playerAnimTimer += 0.01f;
		if (playerAnimTimer > 0.3f)
			playerAnim();
		playerMove();
		obstacleMg();

		//충돌시 게임오버
		if (checkCollision(BALL)) { 
			endGame(scoreTimer);
		}
	}
	
}

void soundCallback(SoundID sound) {
	if (sound == inGameSound)
		playSound(inGameSound);
}

int main() {
	setGameOption(GameOption::GAME_OPTION_ROOM_TITLE, false);
	setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
	setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);

	setMouseCallback(mouseCallback);
	setTimerCallback(timerCallback);
	setKeyboardCallback(keyboardCallback);
	setSoundCallback(soundCallback);

	lobbyScene = createScene("Lobby", "Images/background.png");
	playScene = createScene("InGame", "Images/background.png");

	TimerID timer = createTimer(UPDATETIME);
	startTimer(timer);

	playButton = createObject("Images/playButton.png", lobbyScene, 475, 400);
	scaleObject(playButton, 1.5);
	exitButton = createObject("Images/exitButton.png", lobbyScene, 475, 200);
	scaleObject(exitButton, 1.5f);


	player = createObject("Images/player1.png", playScene, playerX, playerY);
	bomb.bomb = createObject("Images/redCircle.png", playScene, 0, 0, false);

	inGameSound = createSound("Audios/bgm.mp3");
	deathSound = createSound("Audios/death.mp3");
	bombSound = createSound("Audios/bomb.mp3");
	ballSound = createSound("Audios/ball.mp3");
	countdownSound = createSound("Audios/count.mp3");

	startGame(lobbyScene);
	
}