#pragma once
#include <vector>
#include "tmpl8/template.h"
#include "tmpl8/surface.h"
namespace Tmpl8 {

	class Surface;

	const int WORLD_BORDER = 2300;

	const uint32_t gray = 0x303030;
	const uint32_t blue = 0x5a8ecd;
	const uint32_t red = 0xCC2020;
	const uint32_t purple = 0x303030;
	const uint32_t white = 0xFFFFFF;
	const uint32_t lightPurple = 0xead7ff;
	const uint32_t green = 0x30FF50;

	const int BAR_X = 20;
	const int BAR_WIDTH = 120;
	const int BAR_HEIGHT = 18;
	const int BAR_SPACING = 28;
	const int BAR_Y_START = 20;

	const int PLAYER_BAR_X = 1700;
	const int PLAYER_BAR_X_END = 1900;
	const int PLAYER_BAR_WIDTH = PLAYER_BAR_X_END - PLAYER_BAR_X;
	const int HP_BAR_Y = 1025;
	const int HP_BAR_Y_END = 1050;
	const int ENERGY_BAR_Y = 975;
	const int ENERGY_BAR_Y_END = 1000;

	const int BOSS_BAR_X = 1340;
	const int BOSS_BAR_X_END = 1640;
	const int BOSS_BAR_Y = 25;
	const int BOSS_BAR_Y_END = 50;

	const float PLAYER_START_X = 1000.0f;
	const float PLAYER_START_Y = 1000.0f;
	const float PLAYER_SMALL_SPEED = 1.2f;
	const int BOSSFORM_PLAYER_WIDTH = 230;
	const int BOSSFROM_PLAYER_HEIGHT = 250;
	const int PLAYER_MAX_HP = 200;
	const float PLAYER_ACCELERATION = 0.009f;
	const float PLAYER_FRICTION = 0.97f;
	const float BOSS_START_X = 990.0f;
	const float BOSS_START_Y = 540.0f;
	const int BOSS_MAX_HP = 300;

	const float ENERGY_TO_TRANSFORM = 200.0;
	const float ENERGY_DRAIN_RATE = 0.03;
	const float PASSIVE_ENERGY_GAIN = 0.0001;
	const float GRAZE_ENERGY_GAIN = 0.4;

	const float PLAYER_HIT_FLASH_DURATION = 300.0f;
	const int BULLET_HIT_DAMAGE = 5;
	const int CONTACT_DAMAGE = 10;
	const float CONTACT_DAMAGE_INTERVAL = 300.0f;
	const int BOSS_DAMAGE_NORMAL = 1;
	const int BOSS_DAMAGE_TRANSFORMED = 10;

	const float BIGBULLETSPEEDTIMER = 754.72f;
	const float VORTEX_DURATION = 5000.0f;
	const float VORTEX_COOLDOWN = 10000.0f;
	const float VORTEX_FIRE_RATE = 100.0f;
	const int VORTEX_BULLET_COUNT = 12;
	const float VORTEX_BULLET_SPEED = 1.0f;
	const float VORTEX_ANGLE_SHIFT = 0.9f;

	const float PATTERN_SWITCH_TIME = 6037.74f;
	const float PATTERN_START_DELAY = 4700.0f;
	const float GAME_END_DELAY = 1500.0f;

	const float GRAZE_EXTRA_RADIUS = 50.0f;

	const float FLASH_DURATION = 100.0f;
	const float SCREEN_SHAKE_DURATION = 500.0f;
	const int   SCREEN_SHAKE_RANGE = 15;

	const float BOSS_SEEK_RANGE = 500.0f;
	const float BOSS_RETREAT_RANGE = 200.0f;
	const float BOSS_SEEK_SPEED = 0.4f;
	const float BOSS_RETREAT_SPEED = 0.3f;
	const float BOSS_ORBIT_SPEED = 0.3f;
	const float PUSH_FORCE = 2.0f;

	const float FLEE_CLOSE_RANGE = 200.0f;
	const float FLEE_FAR_RANGE = 500.0f;
	const float BULLET_DODGE_RANGE = 300.0f;
	const float WALL_MARGIN = 300.0f;

	const float COMBO_TIMEOUT = 800.0f;

	enum collisionState
	{
		colliding,
		grazing,
		avoiding
	};
	enum GameState {
		menu,
		playing,
		gameOver,
		gameWon,
	};
	enum PlayerState {
		playerForm,
		bossForm,
	};
	class Player
	{
	public:
		float x = 1500;
		float y = 1500;
		float velocityX;
		float velocityY;
		float diameter = 20;
		float width = 61;
		float height = 71;
		float speed = 0.9f;
		float fireTimer = 0;
		float fireCooldown = 150.0f;
		int hp = PLAYER_MAX_HP;
		float energy = 0;
		float bulletDiameter = 3.47f;
		float bulletWidth = 50.0f;
		float aimDirX = 0.0f;
		float aimDirY = -1.0f;
	};
	class Boss
	{
	public:
		float x = 1500;
		float y = 1300;
		float width = 250;
		float height = 311;
		float diameter = 250;
		float speed = 0.4f;
		int hp = BOSS_MAX_HP;
		float bulletDiameter = 22.0f;
		float shieldHitTimer = 50.0f;
		float fireTimer = 0.0f;
		int pattern = 0;
	};
	class Bullet
	{
	public:
		float x = 0.0f;
		float y = 0.0f;
		float speed = 2.0f;
		float directionX = 0.0f;
		float directionY = -1.0f;
		float bossSpeed = 1.0f;
		float bigBulletLifeTimer = 0.0f;
		float explodeTime = 0.0f;
		int size = 5;
		bool isBig = false;
		bool isVortex = false;
	};
	class Game
	{
	public:
		void SetTarget(Surface* surface) { screen = surface; }
		void Init();
		void Shutdown();
		void Menu();
		void Playing(float deltaTime);
		void gameEnding(float deltaTime);
		void GameOver();
		void ResetGame();
		void Win();
		void Tick(float deltaTime);

		void MouseUp(int button) {};
		void MouseDown(int button) {};
		void MouseMove(int x, int y);
		void KeyUp(int key);
		void KeyDown(int key);

		void UpdatePlayerMovement(float deltaTime);
		void RestrictPlayerPosition();
		void UpdatePlayerBullets(float deltaTime);
		void UpdateTransformationState(float deltaTime);
		void DrawUI();
		void DrawVortexCooldown();
		void DrawGameObjects(float deltaTime);
		void DrawingBossBullets(float deltaTime, float bossBullet_speed);

		void CameraCoordinates();

		void Transformation();
		void RedoTransformation();

		void RandomizeBossPatternValues();
		void BossWhipAttack(float bossBullet_speed, float bossFireCooldown, float deltaTime, float swingSpeed, float rotationSpeed);
		void BossSniperAttack(float bossBullet_speed, float bossFireCooldown, float deltaTime, float distanceBetweenBullets, int bulletQuantity);
		void BossBigBulletAttack(float bossBullet_speed, float bossFireCooldown, float deltaTime);
		void BossCircleAttack(int bulletquantity, float bossBullet_speed, float bossFireCooldown, float deltaTime, float angleShiftvalue);

		void PlayerVortexAttack(float deltaTime);
		void PlayerAimDirection();

		void BossPlayerCollision(float deltaTime);
		collisionState AreColliding(float centerX1, float centerX2, float centerY1, float centerY2, float diameter1, float diameter2);
		float RandomFloat(float min, float max);
		void SetMusicVolume(float volume);

	private:

		Player playerShip;
		Boss bossShip;
		std::vector<Bullet> playerBullets;
		std::vector<Bullet> bossBullets;

		void BossSeekPlayer(float deltaTime);
		void BossFlee(float deltaTime);

		float fleeOrbitDirection = 1.0f;
		float fleeDirectionInterval = 2000.0f;
		float fleeDirectionTimer = 0.0f;

		bool key1Pressed = false;
		bool moveUp = false;
		bool moveDown = false;
		bool moveRight = false;
		bool moveLeft = false;
		bool enterPressed = false;
		bool shiftPressed = false;
		bool isFiring = true;
		bool gameEndingStarted = false;

		Surface* screen;
		Sprite* playerSprite = nullptr;
		Sprite* bossSprite = nullptr;
		Sprite* energyIconSprite = nullptr;
		Sprite* playerBulletSprite = nullptr;
		Sprite* bossBulletSprite = nullptr;
		Sprite* bossShieldSprite = nullptr;
		Surface* backgroundSurface = nullptr;
		Font* uiFont = nullptr;

		float cameraX = 0.0f;
		float cameraY = 0.0f;

		int mouseX = 0;
		int mouseY = 0;
		bool mouseClicked = false;

		float changeBossPatternTimer = 0.0f;
		float angleShift = 0.0f;
		float contactDamageTimer = 0.0f;
		float flashTimer = 0.0f;
		float screenShakeTimer = 0.0f;
		float whipTimer = 0.0f;
		float gameEndTimer = 0.0f;
		float vortexCooldownTimer = VORTEX_COOLDOWN;
		float vortexActiveTimer = 0.0f;
		float vortexFireTimer = 0.0f;
		int comboCount = 0;
		float comboTimer = 0.0f;
		float introTimer = 0.0f;

		float vortexAngle = 0.0f;
		bool vortexActive = false;

		float currentBulletSpeed = 1.0f;
		float currentFireCooldown = 100.0f;
		float currentAngleShiftValue = 1.0f;
		float currentSwingSpeed = 0.007f;
		float currentRotationSpeed = 0.001f;
		float currentDistanceBetweenBullets = 0.0f;
		int currentBulletQuantity = 5;

		GameState gameEndResult = menu;
		GameState currentGameState = menu;
		PlayerState currentPlayerState = playerForm;
	};
}; // namespace Tmpl8