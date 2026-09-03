#include "game.h"
#include "tmpl8/surface.h"
#include <SDL_scancode.h>
#include "tmpl8/template.h"
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <cmath>
#include <ctime>
#include <cstdio>
#include <vector>

namespace Tmpl8
{
	void Game::Init()
	{
		SetMusicVolume(0.1f);
		PlaySoundA("assets/menuMusic.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		srand((unsigned int)time(0));
		bossShip.pattern = rand() % 6;
		bossShieldSprite = new Sprite(new Surface("assets/BossShield.png"), 1);
		playerSprite = new Sprite(new Surface("assets/playership.png"), 5);
		bossSprite = new Sprite(new Surface("assets/BossShip.png"), 5);
		backgroundSurface = new Surface("assets/SpaceBackground.png");
		playerBulletSprite = new Sprite(new Surface("assets/playerBullet.png"), 1);
		bossBulletSprite = new Sprite(new Surface("assets/bossBullet.png"), 1);
		uiFont = new Font(Font("assets/lumosCaps.png", "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890"));
		ResetGame();
	}
	void Game::MouseMove(int x, int y) {
		mouseX = x;
		mouseY = y;
	}
	// shakes the screen depending on the ending and changes music
	void Game::gameEnding(float deltaTime) {
		if (gameEndingStarted) {
			gameEndTimer -= deltaTime;
			Game::DrawGameObjects(deltaTime);
			Game::DrawUI();

			if (screenShakeTimer > 0.0f) {
				screenShakeTimer -= deltaTime;
				cameraX += (rand() % (SCREEN_SHAKE_RANGE * 2) - SCREEN_SHAKE_RANGE);
				cameraY += (rand() % (SCREEN_SHAKE_RANGE * 2) - SCREEN_SHAKE_RANGE);
				cameraX = Clamp(cameraX, 0.0f, (float)WORLD_BORDER - ScreenWidth);
				cameraY = Clamp(cameraY, 0.0f, (float)WORLD_BORDER - ScreenHeight);
			}
			if (flashTimer > 0.0f) {
				flashTimer -= deltaTime;
				if (gameEndResult == gameOver) {
					screen->Bar(0, 0, ScreenWidth - 1, ScreenHeight - 1, red);
				}
				else {
					screen->Bar(0, 0, ScreenWidth - 1, ScreenHeight - 1, white);
				}
			}

			if (gameEndTimer <= 0) {
				currentGameState = gameEndResult;
				gameEndingStarted = false;
				PlaySoundA("assets/endMusic.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
				return;
			}
			return;
		}
	}
	void Game::DrawUI()
	{
		int hpFill = Clamp(PLAYER_BAR_X + playerShip.hp - 1, PLAYER_BAR_X, PLAYER_BAR_X_END);
		int energyFill = Clamp(PLAYER_BAR_X + (int)playerShip.energy - 1, PLAYER_BAR_X, PLAYER_BAR_X_END);
		int bossHpFill = Clamp(BOSS_BAR_X + bossShip.hp - 1, BOSS_BAR_X, BOSS_BAR_X_END);

		screen->Bar(PLAYER_BAR_X, HP_BAR_Y, PLAYER_BAR_X_END, HP_BAR_Y_END, gray);
		screen->Bar(PLAYER_BAR_X, HP_BAR_Y, hpFill, HP_BAR_Y_END, blue);

		screen->Bar(BOSS_BAR_X, BOSS_BAR_Y, BOSS_BAR_X_END, BOSS_BAR_Y_END, gray);
		screen->Bar(BOSS_BAR_X, BOSS_BAR_Y, bossHpFill, BOSS_BAR_Y_END, red);

		screen->Bar(PLAYER_BAR_X, ENERGY_BAR_Y, PLAYER_BAR_X_END, ENERGY_BAR_Y_END, gray);

		if (playerShip.energy >= ENERGY_TO_TRANSFORM) { // flickers if energy is full
			bool flashOn = sin(playerShip.fireTimer + 200) > 0;
			int energyColor;
			if (flashOn) {
				energyColor = lightPurple;
			}
			else {
				energyColor = white;
			}
			screen->Bar(PLAYER_BAR_X, ENERGY_BAR_Y, energyFill, ENERGY_BAR_Y_END, energyColor);
		}
		else {
			screen->Bar(PLAYER_BAR_X, ENERGY_BAR_Y, energyFill, ENERGY_BAR_Y_END, lightPurple);
		}

		if (currentPlayerState == bossForm) {
			DrawVortexCooldown();
			// displays hits combo
			if (comboCount > 0 && !vortexActive) {
				char comboText[32];
				int displayCombo = comboCount;

				if (displayCombo > 5) {
					displayCombo = 5;
				}

				sprintf(comboText, "COMBO  X%d", displayCombo);

				int bounceY = (int)(sin(comboTimer * 0.02f) * 5.0f);
				int textY = 80 + bounceY;

				bool show = true;
				if (comboCount >= 5) {
					show = sin(comboTimer * 0.04f) > 0;
				}

				if (show) {
					uiFont->Centre(screen, comboText, textY);
				}

				float timeRatio = Clamp(comboTimer / COMBO_TIMEOUT, 0.0f, 1.0f);
				int barWidth = 200;
				int barX = (ScreenWidth / 2) - (barWidth / 2);
				int barY = textY + 100;

				screen->Bar(barX, barY, barX + barWidth, barY + 8, gray);

				int timerColor;
				if (timeRatio > 0.3f) {
					timerColor = lightPurple;
				}
				else {
					timerColor = red;
				}

				screen->Bar(barX, barY, barX + (int)(barWidth * timeRatio), barY + 8, timerColor);
			}
		}
		screen->Print("PLAYER HP", PLAYER_BAR_X, HP_BAR_Y - 15, white);
		screen->Print("ENERGY", PLAYER_BAR_X, ENERGY_BAR_Y - 15, white);
		screen->Print("BOSS HP", BOSS_BAR_X, BOSS_BAR_Y - 15, white);
	}

	void Game::DrawVortexCooldown()
	{
		int vortexBarY = BAR_Y_START + BAR_SPACING;
		screen->Bar(BAR_X, vortexBarY, BAR_X + BAR_WIDTH, vortexBarY + BAR_HEIGHT, gray);

		if (vortexActive) {
			float durationProgress = Clamp(vortexActiveTimer / VORTEX_DURATION, 0.0f, 1.0f);
			int durationFill = (int)(durationProgress * BAR_WIDTH);
			screen->Bar(BAR_X, vortexBarY, BAR_X + durationFill, vortexBarY + BAR_HEIGHT, blue);
		}
		else {
			float vortexProgress = Clamp(vortexCooldownTimer / VORTEX_COOLDOWN, 0.0f, 1.0f);
			int vortexFill = (int)(vortexProgress * BAR_WIDTH);
			int vortexColor;
			if (vortexProgress >= 1.0f) {
				vortexColor = green;
			}
			else {
				vortexColor = red;
			}
			screen->Bar(BAR_X, vortexBarY, BAR_X + vortexFill, vortexBarY + BAR_HEIGHT, vortexColor);
		}
		screen->Print("1", BAR_X + BAR_WIDTH + 8, vortexBarY + 3, white);
	}
	void Game::DrawGameObjects(float deltaTime)
	{
		playerSprite->DrawScaled((int)(playerShip.x - cameraX + 0.5f), (int)(playerShip.y - cameraY + 0.5f), (int)playerShip.width, (int)playerShip.height, screen);
		bossSprite->DrawScaled((int)(bossShip.x - cameraX + 0.5f), (int)(bossShip.y - cameraY + 0.5f), (int)bossShip.width, (int)bossShip.height, screen);
		if (currentPlayerState == playerForm)
		{
			bool shieldFlicker = true;
			if (bossShip.shieldHitTimer > 0) {
				shieldFlicker = sin(bossShip.shieldHitTimer * 0.05f) > 0;
			}
			if (shieldFlicker) {
				bossShieldSprite->DrawScaled((int)(bossShip.x + bossShip.width / 2.0f - cameraX - 160), (int)(bossShip.y + bossShip.height / 2.0f - cameraY - 140), 320, 320, screen);
			}
		}
	}
	void Game::Shutdown()
	{
		delete playerSprite;
		delete bossSprite;
		delete backgroundSurface;
		delete bossShieldSprite;
		delete playerBulletSprite;
		delete bossBulletSprite;
		delete uiFont;
	}
	void Game::KeyDown(int key) {
		if (key == SDL_SCANCODE_W) moveUp = true;
		if (key == SDL_SCANCODE_S) moveDown = true;
		if (key == SDL_SCANCODE_D) moveRight = true;
		if (key == SDL_SCANCODE_A) moveLeft = true;
		if (key == SDL_SCANCODE_RETURN) enterPressed = true;
		if (key == SDL_SCANCODE_LSHIFT) shiftPressed = true;
		if (key == SDL_SCANCODE_1) key1Pressed = true;
	}
	void Game::KeyUp(int key) {
		if (key == SDL_SCANCODE_W) moveUp = false;
		if (key == SDL_SCANCODE_S) moveDown = false;
		if (key == SDL_SCANCODE_D) moveRight = false;
		if (key == SDL_SCANCODE_A) moveLeft = false;
		if (key == SDL_SCANCODE_RETURN) enterPressed = false;
		if (key == SDL_SCANCODE_LSHIFT) shiftPressed = false;
		if (key == SDL_SCANCODE_1) key1Pressed = false;
	}
	// adds acceleration, limits the speed and imitates friction so that player stops smoothly
	void Game::UpdatePlayerMovement(float deltaTime)
	{
		if (moveUp) {
			playerShip.velocityY -= PLAYER_ACCELERATION * deltaTime;
		}
		if (moveDown) {
			playerShip.velocityY += PLAYER_ACCELERATION * deltaTime;
		}
		if (moveRight) {
			playerSprite->SetFrame(4);
			playerShip.velocityX += PLAYER_ACCELERATION * deltaTime;
		}
		if (moveLeft) {
			playerSprite->SetFrame(0);
			playerShip.velocityX -= PLAYER_ACCELERATION * deltaTime;
		}
		float currentSpeed = sqrtf(playerShip.velocityX * playerShip.velocityX + playerShip.velocityY * playerShip.velocityY);
		if (currentSpeed > playerShip.speed) {
			playerShip.velocityX = (playerShip.velocityX / currentSpeed) * playerShip.speed;
			playerShip.velocityY = (playerShip.velocityY / currentSpeed) * playerShip.speed;
		}
		playerShip.velocityX *= PLAYER_FRICTION;
		playerShip.velocityY *= PLAYER_FRICTION;

		playerShip.x += playerShip.velocityX * deltaTime;
		playerShip.y += playerShip.velocityY * deltaTime;
	}

	void Game::PlayerAimDirection() {

		if (moveUp || moveDown || moveLeft || moveRight) {
			playerShip.aimDirX = 0.0f;
			playerShip.aimDirY = 0.0f;
			if (moveUp)    playerShip.aimDirY -= 1.0f;
			if (moveDown)  playerShip.aimDirY += 1.0f;
			if (moveLeft)  playerShip.aimDirX -= 1.0f;
			if (moveRight) playerShip.aimDirX += 1.0f;
			float len = sqrtf(playerShip.aimDirX * playerShip.aimDirX
				+ playerShip.aimDirY * playerShip.aimDirY);
			if (len > 0.0f) {
				playerShip.aimDirX /= len;
				playerShip.aimDirY /= len;
			}
		}
	}
	// keeps player inside world borders
	void Game::RestrictPlayerPosition() {
		if (currentPlayerState == playerForm) {
			playerShip.y = Clamp(playerShip.y, 0.0f, (float)WORLD_BORDER - playerShip.diameter * 3);
			playerShip.x = Clamp(playerShip.x, 0.0f, (float)WORLD_BORDER - playerShip.diameter * 3);
		}
		else {
			playerShip.y = Clamp(playerShip.y, 0.0f, (float)WORLD_BORDER - BOSSFROM_PLAYER_HEIGHT);
			playerShip.x = Clamp(playerShip.x, 0.0f, (float)WORLD_BORDER - BOSSFORM_PLAYER_WIDTH);
		}
	}
	// centers the camera on the player, restricted to world borders
	void Game::CameraCoordinates()
	{
		cameraX = playerShip.x - (ScreenWidth) / 2;
		cameraY = playerShip.y - (ScreenHeight) / 2;
		cameraX = Clamp(cameraX, 0.0f, (float)WORLD_BORDER - ScreenWidth);
		cameraY = Clamp(cameraY, 0.0f, (float)WORLD_BORDER - ScreenHeight);
	}
	void Game::UpdateTransformationState(float deltaTime) {
		if (playerShip.energy >= ENERGY_TO_TRANSFORM && shiftPressed && currentPlayerState == playerForm)
		{
			Game::Transformation();
		}
		if (currentPlayerState == bossForm) {
			playerShip.energy -= ENERGY_DRAIN_RATE * deltaTime;
		}
		if (playerShip.energy <= 0 && currentPlayerState == bossForm)
		{
			Game::RedoTransformation();
		}
	}

	// reference: https://dooglz.github.io/set09121/lab8_1.html
	// acts depending on the distance to the player:
	// moves towards the player when far away, inceases the distance when too close and constantly circles around the target using perpendicular directions X and Y
	void Game::BossSeekPlayer(float deltaTime) {
		float xdist = (playerShip.x + playerShip.width / 2.0f) - (bossShip.x + bossShip.width / 2.0f);
		float ydist = (playerShip.y + playerShip.height / 2.0f) - (bossShip.y + bossShip.height / 2.0f);
		float distance = sqrtf(xdist * xdist + ydist * ydist);

		if (distance > 0.0f) {
			float directionX = xdist / distance;
			float directionY = ydist / distance;
			if (distance > BOSS_SEEK_RANGE) {
				bossShip.x += directionX * BOSS_SEEK_SPEED * deltaTime;
				bossShip.y += directionY * BOSS_SEEK_SPEED * deltaTime;
			}
			else if (distance < BOSS_RETREAT_RANGE) {
				bossShip.x -= directionX * BOSS_RETREAT_SPEED * deltaTime;
				bossShip.y -= directionY * BOSS_RETREAT_SPEED * deltaTime;
			}
			else {
				bossShip.x += -directionY * BOSS_ORBIT_SPEED * deltaTime;
				bossShip.y += directionX * BOSS_ORBIT_SPEED * deltaTime;
			}
		}
		bossShip.x = Clamp(bossShip.x, 0.0f, (float)WORLD_BORDER - bossShip.width);
		bossShip.y = Clamp(bossShip.y, 0.0f, WORLD_BORDER * 0.9f);
	}
	// activated after player transformation
	// circles around and flees from the player, tries to dodge bullets near by and pushes off the walls
	void Game::BossFlee(float deltaTime) {
		float bossCenterX = bossShip.x + bossShip.width / 2.0f;
		float bossCenterY = bossShip.y + bossShip.height / 2.0f;
		float playerCenterX = playerShip.x + playerShip.width / 2.0f;
		float playerCenterY = playerShip.y + playerShip.height / 2.0f;

		float xdist = bossCenterX - playerCenterX;
		float ydist = bossCenterY - playerCenterY;
		float distance = sqrtf(xdist * xdist + ydist * ydist);

		// flees away from player
		float awayX = 0.0f;
		float awayY = 0.0f;
		if (distance > 0.0f) {
			awayX = xdist / distance;
			awayY = ydist / distance;
		}
		// perpendicular to flee
		float orbitX = -awayY * fleeOrbitDirection;
		float orbitY = awayX * fleeOrbitDirection;

		// flips the direction randomly to feel unpredictable
		fleeDirectionTimer += deltaTime;
		if (fleeDirectionTimer > fleeDirectionInterval) {
			fleeOrbitDirection = (rand() % 2 == 0) ? 1.0f : -1.0f;
			fleeDirectionInterval = RandomFloat(500.0f, 1500.0f);
			fleeDirectionTimer = 0;
		}

		float fleeWeight = 0.4f;
		float orbitWeight = 0.6f;

		if (distance < FLEE_CLOSE_RANGE) {
			fleeWeight = 0.8f;
			orbitWeight = 0.3f;
		}
		else if (distance > FLEE_FAR_RANGE) {
			fleeWeight = 0.0f;
			orbitWeight = 0.8f;
		}
		// find closest bullet and flee from it
		float closestDistance = 999999.0f;
		float bulletFleeX = 0.0f;
		float bulletFleeY = 0.0f;
		for (int i = 0; i < (int)playerBullets.size(); i++) {
			float bulletX = playerBullets[i].x - bossCenterX;
			float bulletY = playerBullets[i].y - bossCenterY;
			float bulletDist = sqrtf(bulletX * bulletX + bulletY * bulletY);
			if (bulletDist < closestDistance) {
				closestDistance = bulletDist;
				if (bulletDist > 0.0f) {
					bulletFleeX = -bulletX / bulletDist;
					bulletFleeY = -bulletY / bulletDist;
				}
			}
		}
		float bulletWeight = (closestDistance < BULLET_DODGE_RANGE) ? 0.7f : 0.0f;

		// push from walls so that it's hard to corner the boss
		float wallPushX = 0.0f;
		float wallPushY = 0.0f;

		if (bossCenterX < WALL_MARGIN)
			wallPushX = (WALL_MARGIN - bossCenterX) / WALL_MARGIN;
		else if (bossCenterX > WORLD_BORDER - WALL_MARGIN)
			wallPushX = -(bossCenterX - (WORLD_BORDER - WALL_MARGIN)) / WALL_MARGIN;

		if (bossCenterY < WALL_MARGIN)
			wallPushY = (WALL_MARGIN - bossCenterY) / WALL_MARGIN;
		else if (bossCenterY > WORLD_BORDER - WALL_MARGIN)
			wallPushY = -(bossCenterY - (WORLD_BORDER - WALL_MARGIN)) / WALL_MARGIN;

		if (wallPushX != 0.0f && (orbitX * wallPushX) < 0) fleeOrbitDirection *= -1.0f;
		if (wallPushY != 0.0f && (orbitY * wallPushY) < 0) fleeOrbitDirection *= -1.0f;

		// combine all the forces from above
		float finalDirX = awayX * fleeWeight + orbitX * orbitWeight + bulletFleeX * bulletWeight + wallPushX * 2.5f;
		float finalDirY = awayY * fleeWeight + orbitY * orbitWeight + bulletFleeY * bulletWeight + wallPushY * 2.5f;

		float finalDistance = sqrtf(finalDirX * finalDirX + finalDirY * finalDirY);
		if (finalDistance > 0.0f) {
			finalDirX /= finalDistance;
			finalDirY /= finalDistance;
		}

		// move faster near walls or bullets
		float dynamicSpeedMultiplier = 2.0f;

		if (wallPushX != 0.0f || wallPushY != 0.0f) {
			dynamicSpeedMultiplier = 2.5f;
		}

		if (closestDistance < 150.0f) {
			dynamicSpeedMultiplier = 2.0f;
		}

		bossShip.x += finalDirX * bossShip.speed * dynamicSpeedMultiplier * deltaTime;
		bossShip.y += finalDirY * bossShip.speed * dynamicSpeedMultiplier * deltaTime;

		bossShip.x = Clamp(bossShip.x, 0.0f, (float)WORLD_BORDER - bossShip.width);
		bossShip.y = Clamp(bossShip.y, 0.0f, WORLD_BORDER * 0.9f);
	}
	// makes pattern values be different every time, controls how often certain ones appear
	void Game::RandomizeBossPatternValues() {
		if (changeBossPatternTimer > PATTERN_SWITCH_TIME) {
			int newPattern;
			do {
				int roll = rand() % 10; // chances 
				if (roll < 2)      newPattern = 1;  // 20% 
				else if (roll < 4) newPattern = 4; // 20%
				else if (roll < 6) newPattern = 0;  // 20%
				else if (roll < 8) newPattern = 3; // 10%
				else if (roll < 9) newPattern = 5; // 10%
				else               newPattern = 2;  // 10%
			} while (newPattern == bossShip.pattern);

			bossShip.pattern = newPattern;
			changeBossPatternTimer = 0;

			switch (bossShip.pattern)
			{
			case 0:
				currentBulletSpeed = RandomFloat(0.9f, 1.3f);
				currentFireCooldown = RandomFloat(30.0f, 40.0f);
				currentSwingSpeed = 0.007f;
				currentRotationSpeed = RandomFloat(0.001f, 0.002f);
				break;
			case 1:
				currentBulletSpeed = RandomFloat(1.0f, 1.8f);
				currentFireCooldown = RandomFloat(377.36f, 400.0f);
				currentBulletQuantity = RandomFloat(12, 20);
				currentAngleShiftValue = RandomFloat(0.5f, 1.0f);
				break;
			case 2:
				currentBulletSpeed = 0.7f;
				currentFireCooldown = 1200.0f;
				break;
			case 3:
				currentBulletSpeed = RandomFloat(1.5f, 2.0f);
				currentFireCooldown = 377.36f;
				currentDistanceBetweenBullets = RandomFloat(0.1f, 0.4f);
				currentBulletQuantity = RandomFloat(3, 9);
				break;
			case 4:
				currentBulletSpeed = 1.0f;
				currentFireCooldown = 94.34f;
				break;
			case 5:
				currentBulletQuantity = 6;
				currentBulletSpeed = 1.5f;
				currentFireCooldown = 60.0f;
				currentAngleShiftValue = 0.2f;
				break;
			}
		}
	}
	// creates circle of bullets around the boss with the use of sin and cos for calculating directions
	void Game::BossCircleAttack(int bulletquantity, float bossBullet_speed, float bossFireCooldown, float deltaTime, float angleShiftvalue) {
		if (bossShip.fireTimer > bossFireCooldown)
		{
			for (int p = 0; p < bulletquantity; p++)
			{
				float stepsBetweenBullets = 2 * PI / bulletquantity;
				Bullet newBullet;
				newBullet.bossSpeed = bossBullet_speed;
				newBullet.x = bossShip.x + bossShip.width / 2.0f;
				newBullet.y = bossShip.y + bossShip.height / 2.0f;
				newBullet.directionX = cos((stepsBetweenBullets * p) + angleShift);
				newBullet.directionY = sin((stepsBetweenBullets * p) + angleShift);
				bossBullets.push_back(newBullet);
			}
			bossShip.fireTimer = 0;
			angleShift += angleShiftvalue;
		}
	}
	// creates a whip-like pattern, bullets fluctuate using sin and rotation
	void Game::BossWhipAttack(float bossBullet_speed, float bossFireCooldown, float deltaTime, float swingSpeed, float rotationSpeed) {
		if (bossShip.fireTimer > bossFireCooldown)
		{
			{
				Bullet newBullet;
				newBullet.bossSpeed = bossBullet_speed;
				newBullet.x = bossShip.x + bossShip.width / 2.0f;
				newBullet.y = bossShip.y + bossShip.height / 2.0f;
				float angle = whipTimer * rotationSpeed + sin(whipTimer * swingSpeed);
				newBullet.directionX = cos(angle);
				newBullet.directionY = sin(angle);
				bossBullets.push_back(newBullet);
			}
			bossShip.fireTimer = 0;
		}
	}
	// fires bullets towards the player using atan2 to find the angle and spread bullets around that angle
	void Game::BossSniperAttack(float bossBullet_speed, float bossFireCooldown, float deltaTime, float distanceBetweenBullets, int bulletQuantity) {
		if (bossShip.fireTimer > bossFireCooldown)
		{
			float xdist = (playerShip.x + playerShip.width / 2.0f) - (bossShip.x + bossShip.width / 2.0f);
			float ydist = (playerShip.y + playerShip.height / 2.0f) - (bossShip.y + bossShip.height / 2.0f);
			float distance = sqrtf(xdist * xdist + ydist * ydist);
			float directionX = xdist / distance;
			float directionY = ydist / distance;
			float angle = atan2f(directionY, directionX);

			float totalSpread = distanceBetweenBullets * (bulletQuantity - 1);
			float startAngle = angle - totalSpread / 2.0f;

			for (int p = 0; p < bulletQuantity; p++) {
				Bullet newBullet;
				newBullet.bossSpeed = bossBullet_speed;
				float bulletAngle = startAngle + p * distanceBetweenBullets;
				newBullet.x = bossShip.x + bossShip.width / 2.0f;
				newBullet.y = bossShip.y + bossShip.height / 2.0f;
				newBullet.directionX = cos(bulletAngle);
				newBullet.directionY = sin(bulletAngle);
				bossBullets.push_back(newBullet);
			}
			bossShip.fireTimer = 0;
		}
	}
	// fires bigger bullet towards player and explodes over time
	void Game::BossBigBulletAttack(float bossBullet_speed, float bossFireCooldown, float deltaTime) {
		if (bossShip.fireTimer > bossFireCooldown) {
			Bullet newBullet;
			newBullet.isBig = true;
			newBullet.bossSpeed = bossBullet_speed;
			newBullet.x = bossShip.x + bossShip.width / 2.0f;
			newBullet.y = bossShip.y + bossShip.height / 2.0f;
			float xdist = (playerShip.x + playerShip.width / 2.0f) - (bossShip.x + bossShip.width / 2.0f);
			float ydist = (playerShip.y + playerShip.height / 2.0f) - (bossShip.y + bossShip.height / 2.0f);
			float distance = sqrtf(xdist * xdist + ydist * ydist);
			float directionX = xdist / distance;
			float directionY = ydist / distance;
			newBullet.directionX = directionX;
			newBullet.directionY = directionY;
			newBullet.explodeTime = BIGBULLETSPEEDTIMER;
			bossBullets.push_back(newBullet);
			bossShip.fireTimer = 0;
		}
	}
	// updates and draws the bullets, draws the big bullet explosion, checks collision
	void Game::DrawingBossBullets(float deltaTime, float bossBullet_speed) {
		for (int p = (int)bossBullets.size() - 1; p >= 0; p--) {
			bossBullets[p].bigBulletLifeTimer += deltaTime;
			bossBullets[p].y += (bossBullets[p].directionY * deltaTime * bossBullets[p].bossSpeed);
			bossBullets[p].x += (bossBullets[p].directionX * deltaTime * bossBullets[p].bossSpeed);

			if (bossBullets[p].bigBulletLifeTimer > bossBullets[p].explodeTime && bossBullets[p].isBig) {
				float explodeX = bossBullets[p].x;
				float explodeY = bossBullets[p].y;
				for (int k = 0; k < 12; k++)
				{
					float stepsBetweenBullets = 2 * PI / 12;
					Bullet newBullet;
					newBullet.bossSpeed = bossBullet_speed;
					newBullet.x = explodeX;
					newBullet.y = explodeY;
					newBullet.directionX = cos((stepsBetweenBullets * k) + angleShift);
					newBullet.directionY = sin((stepsBetweenBullets * k) + angleShift);
					bossBullets.push_back(newBullet);
				}
				std::swap(bossBullets[p], bossBullets.back());
				bossBullets.pop_back();
				continue;
			}
			if ((bossBullets[p].y - cameraY > 0 && bossBullets[p].y - cameraY < ScreenHeight - bossBullets[p].size) &&
				(bossBullets[p].x - cameraX > 0 && bossBullets[p].x - cameraX < ScreenWidth - bossBullets[p].size))
			{
				if (bossBullets[p].isBig)
				{
					bossBulletSprite->DrawScaled((int)(bossBullets[p].x - cameraX - 150), (int)(bossBullets[p].y - cameraY - 150), 200, 200, screen);
				}
				else bossBulletSprite->Draw(screen, (int)(bossBullets[p].x - cameraX), (int)(bossBullets[p].y - cameraY));
			}
			// Remove bullets that leave the world
			if ((bossBullets[p].y <= bossBullets[p].size || bossBullets[p].y >= WORLD_BORDER - bossBullets[p].size) ||
				(bossBullets[p].x <= 0 || bossBullets[p].x >= WORLD_BORDER - bossBullets[p].size))
			{
				std::swap(bossBullets[p], bossBullets.back());
				bossBullets.pop_back();
				continue;
			}
			float bossBulletCenterX = bossBullets[p].x + bossShip.bulletDiameter / 2.0f;
			float bossBulletCenterY = bossBullets[p].y + bossShip.bulletDiameter / 2.0f;
			float playerCenterX = playerShip.x + playerShip.width / 2.0f;
			float playerCenterY = playerShip.y + playerShip.height / 2.0f;

			collisionState currentState = Game::AreColliding(playerCenterX, bossBulletCenterX, playerCenterY, bossBulletCenterY, playerShip.diameter, bossShip.bulletDiameter);
			if (currentState == colliding)
			{
				playerShip.hp -= BULLET_HIT_DAMAGE;
				screenShakeTimer = 100.0f;
				std::swap(bossBullets[p], bossBullets.back());
				bossBullets.pop_back();
				continue;
			}
			else if (currentState == grazing)
			{
				if (playerShip.energy < ENERGY_TO_TRANSFORM) {
					playerShip.energy += GRAZE_ENERGY_GAIN;
				}
			}
			else continue;
		}
	}
	// creates and updates player bullets, handles combo and mouse movement in boss form
	void Game::UpdatePlayerBullets(float deltaTime) {
		bool isUsingAbility = false;
		if (currentPlayerState == bossForm) {
			if (vortexActive || key1Pressed) {
				isUsingAbility = true;
			}
		}
		if (playerShip.fireTimer > playerShip.fireCooldown && isFiring && changeBossPatternTimer >= 0.0f && !isUsingAbility)
		{
			Bullet newBullet;
			newBullet.x = playerShip.x + (playerShip.width / 2.0f) - (playerShip.bulletWidth / 2.0f);
			newBullet.y = playerShip.y;

			if (currentPlayerState == bossForm) {
				float targetX = mouseX + cameraX;
				float targetY = mouseY + cameraY;
				float playerCenterX = playerShip.x + playerShip.width / 2.0f;
				float playerCenterY = playerShip.y + playerShip.height / 2.0f;
				float dx = targetX - playerCenterX;
				float dy = targetY - playerCenterY;
				float len = sqrtf(dx * dx + dy * dy);
				if (len > 0.0f) {
					newBullet.directionX = dx / len;
					newBullet.directionY = dy / len;
				}
				newBullet.size = 0;
				newBullet.speed = 2.5f;
			}
			playerBullets.push_back(newBullet);
			playerShip.fireTimer = 0;
		}
		for (int q = (int)playerBullets.size() - 1; q >= 0; q--)
		{
			playerBullets[q].y += (playerBullets[q].directionY * deltaTime * playerBullets[q].speed);
			playerBullets[q].x += (playerBullets[q].directionX * deltaTime * playerBullets[q].speed);
			if ((playerBullets[q].y - cameraY > 0 && playerBullets[q].y - cameraY < ScreenHeight - 4) &&
				(playerBullets[q].x - cameraX > 0 && playerBullets[q].x - cameraX < ScreenWidth - 4))
			{
				if (playerBullets[q].size > 5) {
					playerBulletSprite->DrawScaled((int)(playerBullets[q].x - cameraX), (int)(playerBullets[q].y - cameraY), playerBullets[q].size, playerBullets[q].size, screen);
				}
				else {
					playerBulletSprite->Draw(screen, (int)(playerBullets[q].x - cameraX), (int)(playerBullets[q].y - cameraY));
				}
			}
			if ((playerBullets[q].y <= 4 || playerBullets[q].y >= WORLD_BORDER - 4) ||
				(playerBullets[q].x <= 0 || playerBullets[q].x >= WORLD_BORDER - 4))
			{
				std::swap(playerBullets[q], playerBullets.back());
				playerBullets.pop_back();
				continue;
			}
			float playerBulletCenterX = playerBullets[q].x + playerShip.bulletDiameter / 2.0f;
			float playerBulletCenterY = playerBullets[q].y + playerShip.bulletDiameter / 2.0f;
			float bossCenterX = bossShip.x + bossShip.width / 2.0f;
			float bossCenterY = bossShip.y + bossShip.height / 2.0f;

			collisionState currentState = Game::AreColliding(bossCenterX, playerBulletCenterX, bossCenterY, playerBulletCenterY, bossShip.diameter, playerShip.bulletDiameter);

			if (currentState == colliding)
			{
				if (introTimer > 0.0f) {
					bossShip.shieldHitTimer = 100.0f;
				}
				else {
					if (currentPlayerState == bossForm)
					{
						if (playerBullets[q].isVortex) {
							bossShip.hp -= BOSS_DAMAGE_TRANSFORMED;
						}
						else {
							if (comboTimer > 0.0f) comboCount++;
							else comboCount = 1;

							comboTimer = COMBO_TIMEOUT;
							float multiplier = 1.0f + ((comboCount - 1) * 0.2f);
							if (multiplier > 2.0f) multiplier = 2.0f;

							bossShip.hp -= (int)(BOSS_DAMAGE_TRANSFORMED * multiplier);
						}
					}
					else if (currentPlayerState == playerForm)
					{
						bossShip.shieldHitTimer = 250.0f;
						bossShip.hp -= BOSS_DAMAGE_NORMAL;
					}
				}
				std::swap(playerBullets[q], playerBullets.back());
				playerBullets.pop_back();
				continue;
			}
			else continue;
		}
	}
	// similar to bossCircleAttack but with fixed values and duration
	void Game::PlayerVortexAttack(float deltaTime) {

		if (!vortexActive) {
			vortexCooldownTimer += deltaTime;
			if (vortexCooldownTimer >= VORTEX_COOLDOWN && key1Pressed) {
				vortexActive = true;
				vortexActiveTimer = VORTEX_DURATION;
				vortexFireTimer = 0.0f;
				vortexAngle = 0.0f;
				vortexCooldownTimer = 0.0f;
			}
			return;
		}
		vortexActiveTimer -= deltaTime;
		vortexFireTimer += deltaTime;
		if (vortexFireTimer >= VORTEX_FIRE_RATE) {
			float centerX = playerShip.x + playerShip.width / 2.0f;
			float centerY = playerShip.y + playerShip.height / 2.0f;
			float step = 2.0f * PI / VORTEX_BULLET_COUNT;
			for (int p = 0; p < VORTEX_BULLET_COUNT; p++) {
				Bullet newBullet;
				newBullet.x = centerX;
				newBullet.y = centerY;
				newBullet.directionX = cos(step * p + vortexAngle);
				newBullet.directionY = sin(step * p + vortexAngle);
				newBullet.speed = VORTEX_BULLET_SPEED;
				newBullet.isVortex = true;
				playerBullets.push_back(newBullet);
			}
			vortexAngle += VORTEX_ANGLE_SHIFT;
			vortexFireTimer = 0.0f;
		}
		if (vortexActiveTimer <= 0.0f) {
			vortexActive = false;
		}
	}
	// detects how different objects interact using sphere-sphere collision: collide, graze or avoid
	// grazing is detected when objects are in close enough range and allows player to gain energy faster
	collisionState Game::AreColliding(float centerX1, float centerX2, float centerY1, float centerY2, float diameter1, float diameter2)
	{
		float xdist = centerX1 - centerX2;
		float ydist = centerY1 - centerY2;
		float radiusSum = (diameter1 + diameter2) / 2.0f;
		float distance = sqrtf(xdist * xdist + ydist * ydist);

		if (distance <= radiusSum)
		{
			return colliding;
		}
		else if (distance > radiusSum && distance < radiusSum + GRAZE_EXTRA_RADIUS)
		{
			return grazing;
		}
		else return avoiding;
	}
	// prevents player from touching the boss
	// makes player take damage but with a reasonable time frame
	void Game::BossPlayerCollision(float deltaTime) {
		float bossCenterX = bossShip.x + bossShip.width / 2.0f;
		float bossCenterY = bossShip.y + bossShip.height / 2.0f;
		float playerCenterX = playerShip.x + playerShip.width / 2.0f;
		float playerCenterY = playerShip.y + playerShip.height / 2.0f;

		collisionState currentState = Game::AreColliding(playerCenterX, bossCenterX, playerCenterY, bossCenterY, playerShip.diameter, bossShip.diameter);
		if (currentState == colliding) {
			float xdist = playerCenterX - bossCenterX;
			float ydist = playerCenterY - bossCenterY;
			float distance = sqrtf(xdist * xdist + ydist * ydist);
			if (distance > 0.0f) {
				float pushX = xdist / distance;
				float pushY = ydist / distance;
				float pushForce = PUSH_FORCE * deltaTime;
				playerShip.x += pushX * pushForce;
				playerShip.y += pushY * pushForce;
			}
			contactDamageTimer += deltaTime;
			if (contactDamageTimer > CONTACT_DAMAGE_INTERVAL) {
				playerShip.hp -= CONTACT_DAMAGE;
				screenShakeTimer = 70.0f;
				contactDamageTimer = 0;
			}
		}
		else {
			contactDamageTimer = 0;
		}
		Game::RestrictPlayerPosition();
	}
	// returns a random value from a range
	float Game::RandomFloat(float min, float max) {
		return min + (float)rand() / (float)RAND_MAX * (max - min);
	}
	// appears only once at launch
	void Game::Menu()
	{
		screen->Clear(0); 
		uiFont->Centre(screen, "MENU", 300);
		uiFont->Centre(screen, "WASD   for   movement", 400);
		uiFont->Centre(screen, "SHIFT   to   transform", 460);
		uiFont->Centre(screen, "ONE   for   special   transform   ability", 520);
		uiFont->Centre(screen, "When   in   a   BOSSFORM   use   your   MOUSE   to   AIM   towards   the   enemy", 580);
		uiFont->Centre(screen, "Press    ENTER   to   start", 680);
		if (enterPressed) {
			PlaySoundA("assets/battleMusic.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			currentGameState = playing;
		}
	};
	void Game::GameOver()
	{
		screen->Clear(0); 
		uiFont->Centre(screen, "GAME    OVER", 440);
		uiFont->Centre(screen, "Press    ENTER   to   retry", 500);
		if (enterPressed) {
			Game::ResetGame();
			PlaySoundA("assets/battleMusic.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			currentGameState = playing;
			return;
		}
	}
	void Game::Win() {
		screen->Clear(0); 
		uiFont->Centre(screen, "YOU     WON", 440);
		uiFont->Centre(screen, "Press    ENTER   to   start", 500);
		if (enterPressed) {
			Game::ResetGame();
			PlaySoundA("assets/battleMusic.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			currentGameState = playing;
			return;
		}
	}
	void Game::ResetGame() {
		playerShip.x = PLAYER_START_X;
		playerShip.y = PLAYER_START_Y;
		playerShip.speed = PLAYER_SMALL_SPEED;
		playerShip.hp = PLAYER_MAX_HP;
		playerShip.energy = 0;
		playerShip.fireCooldown = 150;
		playerShip.fireTimer = 0;
		playerShip.width = 61;
		playerShip.height = 71;
		playerShip.diameter = 20.0f;

		bossShip.x = BOSS_START_X;
		bossShip.y = -600.0f;
		bossShip.hp = BOSS_MAX_HP;
		bossShip.width = 250;
		bossShip.height = 311;
		bossShip.diameter = 250.0f;
		bossShip.pattern = rand() % 6;
		bossShip.fireTimer = 0.0f;
		bossShip.shieldHitTimer = 0.0f;
		gameEndTimer = 0.0f;

		playerBullets.clear();
		bossBullets.clear();

		bossShip.pattern = 1;
		changeBossPatternTimer = 0.0f;
		whipTimer = 0.0f;
		angleShift = 0.0f;
		contactDamageTimer = 0.0f;
		flashTimer = 0.0f;
		screenShakeTimer = 0.0f;

		vortexCooldownTimer = 0.0f;
		vortexActiveTimer = 0.0f;
		vortexFireTimer = 0.0f;
		vortexAngle = 0.0f;
		comboTimer = 0.0f;
		introTimer = 4700.0f;

		gameEndingStarted = false;
		vortexActive = false;

		currentPlayerState = playerForm;
		comboCount = 0;
	}
	// enlarges player, shrinks the boss, clears all the bullets
	void Game::Transformation() {
		bossBullets.clear();
		playerBullets.clear();

		flashTimer = FLASH_DURATION;
		screenShakeTimer = SCREEN_SHAKE_DURATION;

		// using these centers to make a smooth transition between small and big states
		float oldBossCenterX = bossShip.x + bossShip.width / 2.0f;
		float oldPlayerCenterX = playerShip.x + playerShip.width / 2.0f;
		float oldBossCenterY = bossShip.y + bossShip.height / 2.0f;
		float oldPlayerCenterY = playerShip.y + playerShip.height / 2.0f;

		playerShip.fireCooldown = 50.0f;
		playerShip.width = 230;
		playerShip.height = 270;
		playerShip.speed = 0.5;
		bossShip.width = 71;
		bossShip.height = 81;
		bossShip.diameter = 70.0f;
		playerShip.diameter = 70.0f;

		playerShip.x = oldPlayerCenterX - playerShip.width / 2.0f;
		bossShip.x = oldBossCenterX - bossShip.width / 2.0f;
		playerShip.y = oldPlayerCenterY - playerShip.height / 2.0f;
		bossShip.y = oldBossCenterY - bossShip.height / 2.0f;

		currentPlayerState = bossForm;

		vortexCooldownTimer = VORTEX_COOLDOWN;
		vortexActive = false;
	}
	// when energy wears out, reverts back to small player and big boss
	void Game::RedoTransformation()
	{
		float oldBossCenterX = bossShip.x + bossShip.width / 2.0f;
		float oldPlayerCenterX = playerShip.x + playerShip.width / 2.0f;
		float oldBossCenterY = bossShip.y + bossShip.height / 2.0f;
		float oldPlayerCenterY = playerShip.y + playerShip.height / 2.0f;

		playerShip.fireCooldown = 150.0f;
		playerShip.width = 71;
		playerShip.height = 81;
		bossShip.width = 230;
		bossShip.height = 270;
		playerShip.speed = PLAYER_SMALL_SPEED;
		bossShip.diameter = 267.0f;
		playerShip.diameter = 22.0f;

		playerShip.x = oldPlayerCenterX - playerShip.width / 2.0f;
		bossShip.x = oldBossCenterX - bossShip.width / 2.0f;
		playerShip.y = oldPlayerCenterY - playerShip.height / 2.0f;
		bossShip.y = oldBossCenterY - bossShip.height / 2.0f;

		currentPlayerState = playerForm;
		bossBullets.clear();
		bossShip.fireTimer = 0;
		changeBossPatternTimer = 0;
	}

	void Game::SetMusicVolume(float volume) {
		DWORD vol = (DWORD)(volume * 0xFFFF);
		waveOutSetVolume(NULL, vol | (vol << 16));
	}

	// main function
	// background, timers, ui, movement and game ending conditions are here
	void Game::Playing(float deltaTime) {
		backgroundSurface->CopyTo(screen, (int)-cameraX, (int)-cameraY);
		Game::gameEnding(deltaTime);
		if (gameEndingStarted || currentGameState != playing) return;

		playerShip.fireTimer += deltaTime;
		bossShip.shieldHitTimer -= deltaTime;
		whipTimer += deltaTime;

		if (comboTimer > 0.0f) {
			comboTimer -= deltaTime;
		}
		else {
			comboCount = 0;
		}

		if (playerShip.energy < ENERGY_TO_TRANSFORM) {
			playerShip.energy += (PASSIVE_ENERGY_GAIN * deltaTime);
		}

		if (introTimer > 0.0f) {
			introTimer -= deltaTime;

			float progress = 1.0f - (introTimer / PATTERN_START_DELAY);
			float smoothExit = 1.0f - powf(1.0f - progress, 3.0f);

			bossShip.y = -600.0f + (BOSS_START_Y - (-600.0f)) * smoothExit;
			bossShip.x = BOSS_START_X;
		}
		else {

			changeBossPatternTimer += deltaTime;

			if (currentPlayerState == playerForm)
			{
				bossShip.fireTimer += deltaTime;
				Game::BossSeekPlayer(deltaTime);
				Game::RandomizeBossPatternValues();

				switch (bossShip.pattern)
				{
				case 0: Game::BossWhipAttack(currentBulletSpeed, currentFireCooldown, deltaTime, currentSwingSpeed, currentRotationSpeed); break;
				case 1: Game::BossCircleAttack(currentBulletQuantity, currentBulletSpeed, currentFireCooldown, deltaTime, currentAngleShiftValue); break;
				case 2: Game::BossBigBulletAttack(currentBulletSpeed, currentFireCooldown, deltaTime); break;
				case 3: Game::BossSniperAttack(currentBulletSpeed, currentFireCooldown, deltaTime, currentDistanceBetweenBullets, currentBulletQuantity); break;
				case 4: Game::BossCircleAttack(4, currentBulletSpeed, currentFireCooldown, deltaTime, 0); break;
				case 5: Game::BossCircleAttack(currentBulletQuantity, currentBulletSpeed, currentFireCooldown, deltaTime, currentAngleShiftValue); break;
				}
			}
			else if (currentPlayerState == bossForm)
			{
				Game::BossFlee(deltaTime);
				Game::PlayerVortexAttack(deltaTime);
			}
		}

		bossSprite->SetFrame(2);
		playerSprite->SetFrame(2);

		Game::UpdatePlayerMovement(deltaTime);
		Game::RestrictPlayerPosition();
		Game::CameraCoordinates();

		if (screenShakeTimer > 0.0f) {
			screenShakeTimer -= deltaTime;
			cameraX += (rand() % (SCREEN_SHAKE_RANGE * 2) - SCREEN_SHAKE_RANGE);
			cameraY += (rand() % (SCREEN_SHAKE_RANGE * 2) - SCREEN_SHAKE_RANGE);
			cameraX = Clamp(cameraX, 0.0f, (float)WORLD_BORDER - ScreenWidth);
			cameraY = Clamp(cameraY, 0.0f, (float)WORLD_BORDER - ScreenHeight);
		}

		Game::BossPlayerCollision(deltaTime);
		Game::UpdatePlayerBullets(deltaTime);
		Game::DrawingBossBullets(deltaTime, 2);
		Game::UpdateTransformationState(deltaTime);

		if (bossShip.hp <= 0 && !gameEndingStarted) {
			gameEndingStarted = true;
			gameEndTimer = GAME_END_DELAY;
			flashTimer = 300.0f;
			screenShakeTimer = 1500.0f;
			PlaySoundA(NULL, NULL, 0);
			gameEndResult = gameWon;
		}

		if (playerShip.hp <= 0 && !gameEndingStarted) {
			gameEndingStarted = true;
			gameEndTimer = GAME_END_DELAY;
			screenShakeTimer = 500.0f;
			flashTimer = 300.0f;
			PlaySoundA(NULL, NULL, 0);
			gameEndResult = gameOver;
		}

		Game::DrawGameObjects(deltaTime);
		Game::DrawUI();

		if (flashTimer > 0.0f) {
			flashTimer -= deltaTime;
			screen->Bar(0, 0, ScreenWidth - 1, ScreenHeight - 1, white);
		}
	}


	void Game::Tick(float deltaTime)
	{
		if (currentGameState == menu) {
			Game::Menu();
			return;
		}
		else if (currentGameState == playing) {
			Game::Playing(deltaTime);
			return;
		}
		else if (currentGameState == gameOver) {
			Game::GameOver();
			return;
		}
		else if (currentGameState == gameWon) {
			Game::Win();
			return;
		}
	}
};