/*
 * CSE316_Project.c
 *
 * Created: 8/18/2022 12:24:27 AM
 * Author : Anupznk
 */ 

#include <avr/io.h>
#define F_CPU 1000000
#include <util/delay.h>
#include <avr/interrupt.h>

#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTC6
#define EN eS_PORTC7

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "lcd.h"


long binImage[] = {
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000,
	0b000000000000000000000000
};

int playerPos = 1;
char bulletPos = 0;
int playerAlive = 1;
int bulletPosCount = 0;
int playerPosCount = 0;

int shiftCou = 0;
int enemyMovementWait = 0;

// for player
int currScore = 0;
int health = 100;
int damage = 1;	
// for player

int currEnemyIndex = 0;
int debug = 0;

struct Enemy {
	int hitPoint;
	int isAlive;
	int health;
	int enemyPos;	//y pos
	int enemyUpFlag;
	int waitForReload;
	int speed;
};
struct Enemy enemies[10];

void initEnemy() {
	for (int i = 0; i<10; i++){
		enemies[i].speed = 5;
		enemies[i].hitPoint = 10+5*i;
		enemies[i].enemyPos = 5;
		enemies[i].isAlive = 0;
		enemies[i].health = 100 + 10 * i;
		enemies[i].enemyUpFlag = 1;
		enemies[i].waitForReload = 26;
	}
	enemies[0].isAlive = 1;
}

struct Bullet{
	int isActive;
	int hasHit;
	int posX;
	int posY;
	int timeout;
};

struct Bullet enemyBullet[10];
void initEnemyBullets() {
	for (int i = 0; i< 10; i++)
	{
		enemyBullet[i].isActive = 0;
		enemyBullet[i].hasHit = 0;
		enemyBullet[i].timeout = 0;
	}
}

struct Bullet playerBullet;

void initPlayerBullet() {
	if(!playerBullet.isActive) {
		playerBullet.posX = 1;
		playerBullet.posY = playerPos;
		playerBullet.isActive = 1;
		playerBullet.timeout = 0;
	}
}

void updateLCD(){
	// need to be called when update
	
	char score[8];
	char healthStr[8];
	
	dtostrf(currScore, 1, 0, score);
	Lcd4_Set_Cursor(1,1);
	Lcd4_Write_String("Score: ");
	strcat(score, "   ");
	Lcd4_Set_Cursor(1,7);
	Lcd4_Write_String(score);
	
	dtostrf(health, 1, 0, healthStr);
	Lcd4_Set_Cursor(2,1);
	Lcd4_Write_String("Health: ");
	Lcd4_Set_Cursor(2,8);
	strcat(healthStr, "   ");
	Lcd4_Write_String(healthStr);
}

char images[6][8] = {
	0
};

void binToImage()
{
	long lMask = 0b111111110000000000000000;
	long cMask = 0b000000001111111100000000;
	long rMask = 0b000000000000000011111111;
	
	for (int i = 0 ; i  < 8; i++)
	{
		char lVal = (char) ((binImage[i] & lMask) >> 16);
		char cVal = (char) ((binImage[i] & cMask) >> 8);
		char rVal = (char) (binImage[i] & rMask);
		images[0][i] = lVal;
		images[1][i] = cVal;
		images[2][i] = rVal;
	}
	
	for (int i = 0 ; i  < 8; i++)
	{
		char lVal = (char) ((binImage[i+8] & lMask) >> 16);
		char cVal = (char) ((binImage[i+8] & cMask) >> 8);
		char rVal = (char) (binImage[i+8] & rMask);
		images[3][i] = lVal;
		images[4][i] = cVal;
		images[5][i] = rVal;
	}
}

void erasePlayer()
{
	binImage[playerPos-1] &= 0b011111111111111111111111;
	binImage[playerPos] &= 0b001111111111111111111111;
	binImage[playerPos+1] &= 0b011111111111111111111111;
}

void drawPlayer()
{
	binImage[playerPos-1]	|=	0b100000000000000000000000;
	binImage[playerPos]		|=	0b110000000000000000000000;
	binImage[playerPos+1]	|=	0b100000000000000000000000;
}

void eraseEnemy()
{
	binImage[enemies[currEnemyIndex].enemyPos-2] &= 0b111111111111111111111100;
	binImage[enemies[currEnemyIndex].enemyPos-1] &=	0b111111111111111111111110;
	binImage[enemies[currEnemyIndex].enemyPos] &=	0b111111111111111111111100;
	binImage[enemies[currEnemyIndex].enemyPos+1] &= 0b111111111111111111111110;
	binImage[enemies[currEnemyIndex].enemyPos+2] &= 0b111111111111111111111100;
}

void drawEnemy()
{
	
	binImage[enemies[currEnemyIndex].enemyPos-2] |=	0b000000000000000000000011;
	binImage[enemies[currEnemyIndex].enemyPos-1] |=	0b000000000000000000000001;
	binImage[enemies[currEnemyIndex].enemyPos] |=	0b000000000000000000000011;
	binImage[enemies[currEnemyIndex].enemyPos+1] |=	0b000000000000000000000001;
	binImage[enemies[currEnemyIndex].enemyPos+2] |=	0b000000000000000000000011;
	
}

int nextLevelWait = 0;

void handleEnemyMovement(){
	eraseEnemy();
	enemyMovementWait++;

	if(enemies[currEnemyIndex].enemyPos == 2){
		enemies[currEnemyIndex].enemyUpFlag = 0;
	} else if(enemies[currEnemyIndex].enemyPos == 13){
	enemies[currEnemyIndex].enemyUpFlag = 1;
	}
		
	if(enemyMovementWait == enemies[currEnemyIndex].speed)
	{
		enemyMovementWait = 0;
		if(enemies[currEnemyIndex].enemyUpFlag){
			enemies[currEnemyIndex].enemyPos--;
		} else {
			enemies[currEnemyIndex].enemyPos++;
		}
	}
	
	drawEnemy();
}

void handleEnemyBullet()
{
	for(int i = 0; i < 5; i++) {
		if(enemyBullet[i].isActive) {
			enemyBullet[i].posX--;
			if((enemyBullet[i].posX == 1 && enemyBullet[i].posY == playerPos) || (enemyBullet[i].posX == 0 && (enemyBullet[i].posY == playerPos-1 ||   enemyBullet[i].posY == playerPos+1)))
			{
				enemyBullet[i].hasHit = 1;
				health -= damage;
				PORTC |= 0b00000010;
				_delay_ms(10);
				PORTC &= 0b11111101;
				
				updateLCD();
				if(health == 0)	{
					playerAlive = 0;
				}
			}
			if(enemyBullet[i].posX < 0 || enemyBullet[i].hasHit) {
				enemyBullet[i].isActive = 0;
				enemyBullet[i].hasHit = 0;
				enemyBullet[i].timeout = 5;
				long eraseBulletMask = ~(0b110000000000000000000000 >> (enemyBullet[i].posX+1));
				binImage[enemyBullet[i].posY] &= eraseBulletMask;
				} else {
				long eraseBulletMask = ~(0b110000000000000000000000 >> (enemyBullet[i].posX+1));
				binImage[enemyBullet[i].posY] &= eraseBulletMask;
				binImage[enemyBullet[i].posY] |= (0b110000000000000000000000 >> (enemyBullet[i].posX));
			}
			} else {
			if(enemyBullet[i].timeout == 0) {
				if(enemies[currEnemyIndex].waitForReload) {
					enemies[currEnemyIndex].waitForReload--;
					} else {
					enemyBullet[i].posX = 21;
					enemyBullet[i].posY = enemies[currEnemyIndex].enemyPos;
					enemyBullet[i].isActive = 1;
					enemyBullet[i].timeout = 0;
					binImage[enemyBullet[i].posY] |= (0b000000000000000000000110);
					enemies[currEnemyIndex].waitForReload = 10;
				}
				} else {
				enemyBullet[i].timeout--;
			}
		}
	}
	
}

void handlePlayerBullet()
{
	playerBullet.posX++;
	if((playerBullet.posX == 22 && (playerBullet.posY == enemies[currEnemyIndex].enemyPos-1 || playerBullet.posY == enemies[currEnemyIndex].enemyPos+1))
	|| (playerBullet.posX == 21 &&
	(playerBullet.posY == enemies[currEnemyIndex].enemyPos || playerBullet.posY == enemies[currEnemyIndex].enemyPos-2 || playerBullet.posY == enemies[currEnemyIndex].enemyPos+2)))
	{
		playerBullet.hasHit = 1;
		currScore += enemies[currEnemyIndex].hitPoint;
		int count = (enemies[currEnemyIndex].hitPoint-5)/5;
		for (int i = 0; i < count; i++) {
			PORTC |= 0b00000001;
			_delay_ms(10);
			PORTC &= 0b11111110;
			_delay_ms(10);
		}
		
		updateLCD();
		
		enemies[currEnemyIndex].health -=50;
		
		if (enemies[currEnemyIndex].health <= 0)
		{
			eraseEnemy();
		}
	}
	
	if(playerBullet.posX >22 || playerBullet.hasHit)
	{
		playerBullet.isActive = 0;
		playerBullet.hasHit = 0;
		playerBullet.timeout = 5;
		long eraseBulletMask = ~(0b110000000000000000000000 >> (playerBullet.posX-1));
		binImage[playerBullet.posY] &= eraseBulletMask;
	}
	else
	{
		long eraseBulletMask = ~(0b110000000000000000000000 >> (playerBullet.posX-1));
		binImage[playerBullet.posY] &= eraseBulletMask;
		binImage[playerBullet.posY] |= (0b110000000000000000000000 >> (playerBullet.posX));
	}
	
}


void updateBullet()
{
	if (bulletPos)
	{
		bulletPosCount++;
		if (bulletPosCount)
		{
			long eraseBulletMask = ~(0b110000000000000000000000 >> (bulletPos-1));
			binImage[playerPos] &= eraseBulletMask;
			bulletPos++;
			binImage[playerPos] |= (0b110000000000000000000000 >> (bulletPos-1));
		}
	}
}
void drawBullet()
{
	if (!bulletPos)
	{
		binImage[playerPos] |= 0b000110000000000000000000;
		bulletPos = 4;
	}
}



void updatePlayer(int direction){
	
	playerPosCount++;
	if (playerPosCount % 5 != 0)
		return;
	
	playerPosCount = 0;	
	erasePlayer();
	
	if (direction == 1){
		// go up
		playerPos --;
		if (playerPos <= 1){
			playerPos = 1;
		}
	}
	
	if (direction == 0){
		// go down
		playerPos ++;
		if (playerPos >= 12){
			playerPos = 12;
		}
	}
	
	
}

/*
void drawEnvironment(){
	if (shiftCou == 6) {
		binImage[15]=0b001000111000011000001001101;
		
	}
	if (shiftCou == 12) {
		binImage[15]=0b001000111000011001101001101;
		
	}
	if (shiftCou == 18) {
		binImage[15]=0b001000111000011011001001101;
		
	}
	if (shiftCou == 24) {
		binImage[15]=0b001000111000011010101001101;
		shiftCou = 0;
	}
	binImage[15]	=	binImage[15] << 1;
	shiftCou++;
	
	
}


ISR(INT0_vect)
{
	drawBullet();
	while (bulletPos < 25)
	{
		updateBullet();
		binToImage();
		for (int i = 0 ; i < 6; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if (i == 0 && j == 0)
				PORTA = PORTA & 0b11111110;
				else
				PORTA = PORTA | 0b00000001;
				PORTA = PORTA | 0b00000010;
				PORTA = PORTA & 0b11111101;  // shift done
				PORTA = PORTA | 0b00000100;
				PORTA = PORTA & 0b11111011;  // store done
				PORTB = images[i][j];
				_delay_us(1000);
			}
		}
	}
	bulletPos = 0;
}
*/

void showGameOver(){
	binImage[0] =	0b011100011001100110111100;
	binImage[1] =	0b100000100101011010100000;
	binImage[2] =	0b100000100101011010100000;
	binImage[3] =	0b100000100101000010111100;
	binImage[4] =	0b100110111101000010100000;
	binImage[5] =	0b100010100101000010100000;
	binImage[6] =	0b011110100101000010111100;
	binImage[7] =	0b000000000000000010000000;
	binImage[8] =	0b011100100010111101111000;
	binImage[9] =	0b100010100010100001000100;
	binImage[10] =	0b100010100010100001000100;
	binImage[11] =	0b100010100010111101111000;
	binImage[12] =	0b100010100010100001100000;
	binImage[13] =	0b100010100010100001010000;
	binImage[14] =	0b100010010100100001001000;
	binImage[15] =	0b011100001000111101000100;
}

void showNext(){
	binImage[0] =	0b000000000000000000000000;
	binImage[1] =	0b000011000000000000000000;
	binImage[2] =	0b000001100000000000000000;
	binImage[3] =	0b000000110000000000000000;
	binImage[4] =	0b000000011000000000000000;
	binImage[5] =	0b000000001100000000000000;
	binImage[6] =	0b000000000110000000000000;
	binImage[7] =	0b000000000011000000000000;
	binImage[8] =	0b000000000110000000000000;
	binImage[9] =	0b000000001100000000000000;
	binImage[10] =	0b000000011000000000000000;
	binImage[11] =	0b000000110000000000000000;
	binImage[12] =	0b000001100000000000000000;
	binImage[13] =	0b000011000000000000000000;
	binImage[14] =	0b000110000000000000000000;
	binImage[15] =	0b000000000000000000000000;
}

void showCongrats(){
	binImage[0] =	0b111111111111111111111111;
	binImage[1] =	0b111111111111111111111111;
	binImage[2] =	0b111111111111111111111111;
	binImage[3] =	0b111111111111111111111111;
	binImage[4] =	0b111111111111111111111111;
	binImage[5] =	0b111111111111111111111111;
	binImage[6] =	0b111111111111111111111111;
	binImage[7] =	0b111111111111111111111111;
	binImage[8] =	0b111111111111111111111111;
	binImage[9] =	0b111111111111111111111111;
	binImage[10] =	0b111111111111111111111111;
	binImage[11] =	0b111111111111111111111111;
	binImage[12] =	0b111111111111111111111111;
	binImage[13] =	0b111111111111111111111111;
	binImage[14] =	0b111111111111111111111111;
	binImage[15] =	0b111111111111111111111111;
}

void eraseScreen() {
	binImage[0] =	0b000000000000000000000000;
	binImage[1] =	0b000000000000000000000000;
	binImage[2] =	0b000000000000000000000000;
	binImage[3] =	0b000000000000000000000000;
	binImage[4] =	0b000000000000000000000000;
	binImage[5] =	0b000000000000000000000000;
	binImage[6] =	0b000000000000000000000000;
	binImage[7] =	0b000000000000000000000000;
	binImage[8] =	0b000000000000000000000000;
	binImage[9] =	0b000000000000000000000000;
	binImage[10] =	0b000000000000000000000000;
	binImage[11] =	0b000000000000000000000000;
	binImage[12] =	0b000000000000000000000000;
	binImage[13] =	0b000000000000000000000000;
	binImage[14] =	0b000000000000000000000000;
	binImage[15] =	0b000000000000000000000000;
	
}

int main(void)
{
	DDRC = 0xFF;
	DDRA = 0b00000111;
	DDRB = 0x0;
	DDRD = 0x0;
	PORTA = 0x1;
	
	//PORTB = 0b11111111;
	//_delay_ms(5000);
	
	// set up an interrupt to catch rising edge
	/*
	GICR = (1 << INT0);
	MCUCR = MCUCR | (1 << ISC01);
	MCUCR = MCUCR & (~(1 << ISC00));
	sei();
	*/
	
	
	Lcd4_Init();
	updateLCD();
	
	
	
	for (int i = 0; i < 48; i++)
	{
		PORTA = PORTA | 0b00000010;
		PORTA = PORTA & 0b11111101;  // shift done
		PORTA = PORTA | 0b00000100;
		PORTA = PORTA & 0b11111011;  // store done
		_delay_ms(1);
	}
	
	initEnemyBullets();
	initEnemy();
	while (1)
	{
		if(playerAlive) {
			drawPlayer();
				
			// drawEnvironment();
				
			if (PINA & 0b00001000) {
				// button up pressed
				updatePlayer(1);
			}
			if (PINA & 0b00010000) {
				// button down pressed
				updatePlayer(0);
			}
			if (!(PINA & 0b00100000)) {
				// button bullet pressed
				initPlayerBullet();
				
				
			}
			if(playerBullet.isActive){
				handlePlayerBullet();
			}
			
			if (enemies[currEnemyIndex].health > 0){
				handleEnemyMovement();
				handleEnemyBullet();
			} else {
				showNext();
				if(nextLevelWait == 40){
					eraseScreen();
					nextLevelWait = 0;
					currEnemyIndex++;
					if (currEnemyIndex == 9)
					{
						// all levels finished
						showCongrats();
						return 0;
					}
					
				}
				nextLevelWait++;
			}
			
		}
		else {
			erasePlayer();
			eraseEnemy();
			
			showGameOver();
		}
		
		binToImage();
		
		for (int i = 0 ; i < 6; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if (i == 0 && j == 0)
				PORTA = PORTA & 0b11111110;
				else
				PORTA = PORTA | 0b00000001;
				PORTA = PORTA | 0b00000010;
				PORTA = PORTA & 0b11111101;  // shift done
				PORTA = PORTA | 0b00000100;
				PORTA = PORTA & 0b11111011;  // store done
				PORTB = images[i][j];
				_delay_ms(1);
			}
		}
	}
}