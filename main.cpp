#include "raylib.h"
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

struct Bullet {
    Vector2 position;
    float speed = 10.0f;
    float radius = 5.0f;
    Color color = RED;

    Bullet(Vector2 pos) : position(pos) {}

    void update()
    {
        position.y -= speed;
    }

    void draw() const
    {
        DrawCircleV(position, radius, color);
    }

    bool isOffScreen() const
    {
        return position.y < 0;
    }
};

struct Lethal_Bullet {
    Vector2 position;
    float speed;
	float angle;
    float radius = 8.0f;
    Color color = GRAY;

    Lethal_Bullet(Vector2 pos, float spd, float ang) : position(pos), speed(spd), angle(ang) {}

    void update()
	{
      position.x += cos(angle) * speed;
      position.y += sin(angle) * speed;

    }

    void draw() const
	{
		DrawCircleV(position, radius, color);
    }

};

struct Dummy_enemy {
    Vector2 position = {400, 70};
    float radius = 25.0;
    float health = 125000.0;
    bool death = false;
    Color color = GREEN;

    std::vector<Lethal_Bullet> Lethal_Bullets;
	float bulletCooldown = 0.0f;

    Dummy_enemy() {}

    void draw() const
    {
        if (!death) {
            DrawCircleV(position, radius, color);
        }
    }

    void checkCollisionWithBullets(const std::vector<Bullet>& bullets)
    {
        for (const Bullet& b : bullets) {
            if (CheckCollisionCircles(position, radius, b.position, b.radius)) {
                health -= 10; // Arbitrary damage per hit
                if (health <= 0) {
                    death = true;
                    break;
                }
            }
        }
    }

    void drawHealtEnemey()
    {
        if (!death) {
            DrawText(std::to_string(health).c_str(), 50, 50, 50, GREEN);
        }
    }

    void updateBullets()
	{
        for (auto& b : Lethal_Bullets) {
            b.update();
        }
        // Remove Lethal_Bullets off-screen (optional)
        Lethal_Bullets.erase(std::remove_if(Lethal_Bullets.begin(), Lethal_Bullets.end(), [](const Lethal_Bullet& b) {
            return b.position.x < 0 || b.position.x > GetScreenWidth() ||
                   b.position.y < 0 || b.position.y > GetScreenHeight();
        }), Lethal_Bullets.end());
    }

    void shootRadialPattern(float deltaTime)
	{
        bulletCooldown -= deltaTime;
        if (bulletCooldown <= 0.0f) {
            const int numBullets = 12;
            const float angleStep = 2 * PI / numBullets + GetRandomValue(10, 30);

            for (int i = 0; i < numBullets; ++i) {
                float angle = i * angleStep + GetRandomValue(200, 300);
                Lethal_Bullets.emplace_back(position, 3.0f, angle);
            }

            bulletCooldown = 0.1f; // Fire every 1 second
        }
    }

    void drawBullets() const {
        for (const auto& b : Lethal_Bullets) {
            b.draw();
        }
    }

    const std::vector<Lethal_Bullet>& getLethalBullets() const 
    {
        return Lethal_Bullets;
    }

};

class Player {
   private:
        float radius;
        Vector2 position;
		Color color;
        std::vector<Bullet> bullets;
        std::vector<int> weapons;
		bool isDeath = false;
        std::vector<Lethal_Bullet> l_bullets;

        int powerLevel = 1;
        bool isFocus = false;


        // Linear interpolation helper
        float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        // In your class, add:
        float horiTwo = 0.0f, vertTwo = 0.0f;
        float horiThree = 0.0f, vertThree = 0.0f;
        float transitionSpeed = 15.0f; // tweak for speed

        void updateWeaponOffsets(float deltaTime)
        {
            float targetHoriTwo = isFocus ? 0.0f : 10.0f;
            float targetVertTwo = isFocus ? 0.0f : 10.0f;
            float targetHoriThree = isFocus ? 0.0f : 8.0f;
            float targetVertThree = isFocus ? 0.0f : 8.0f;

            horiTwo = Lerp(horiTwo, targetHoriTwo, deltaTime * transitionSpeed);
            vertTwo = Lerp(vertTwo, targetVertTwo, deltaTime * transitionSpeed);
            horiThree = Lerp(horiThree, targetHoriThree, deltaTime * transitionSpeed);
            vertThree = Lerp(vertThree, targetVertThree, deltaTime * transitionSpeed);
        }

        void drawWeapons() const
        {
            if (powerLevel >= 2) {
                DrawCircleV({ position.x - 25 - horiTwo, position.y + 15 - vertTwo }, 5, DARKGRAY);
                DrawCircleV({ position.x + 25 + horiTwo, position.y + 15 - vertTwo }, 5, DARKGRAY);
            }

            if (powerLevel >= 3) {
                DrawCircleV({ position.x - 15 - horiThree, position.y - 20 + vertThree }, 5, DARKGRAY);
                DrawCircleV({ position.x + 15 + horiThree, position.y - 20 + vertThree }, 5, DARKGRAY);
            }
        }



    public:
        Player(float rad, Vector2 pos, Color col): radius(rad), position(pos), color(col) {}

        void move()
        {
            Vector2 direction = { 0.0f, 0.0f };

            if (IsKeyDown(KEY_RIGHT)) direction.x += 1.0f;
            else if (IsKeyDown(KEY_LEFT))  direction.x -= 1.0f;
            if (IsKeyDown(KEY_UP))    direction.y -= 1.0f;
            else if (IsKeyDown(KEY_DOWN))  direction.y += 1.0f;

            float magnitude = sqrt(direction.x * direction.x + direction.y * direction.y);
            if (magnitude != 0) {
                direction.x /= magnitude;
                direction.y /= magnitude;
            }

            isFocus = IsKeyDown(KEY_LEFT_SHIFT);

            float speed = isFocus? 4.5f : 8.0f;
            position.x += direction.x * speed;
            position.y += direction.y * speed;
        }

        void clamp()
        {
            if (position.x - radius < 0) {
                position.x = radius;
            } else if (position.x + radius > GetScreenWidth()) {
                position.x = GetScreenWidth() - radius;
            }

            if (position.y - radius < 0) {
                position.y = radius;
            } else if (position.y + radius > GetScreenHeight()) {
                position.y = GetScreenHeight() - radius;
            }
        }

        void spawn_player()
        {
            DrawCircleV(position, radius, color);
            updateWeaponOffsets(GetFrameTime());
            drawWeapons();
        }

        void player_shooting()
        {
            static int shootCooldown = 0; // Add cooldown to prevent bullet spam
            shootCooldown++;

            // int uniqe_pos_shot_hori = 10;
            // int uniqe_pos_shot_vert = 10;



            if (IsKeyDown(KEY_Z) && shootCooldown > 3) {
                shootCooldown = 0;

                switch (powerLevel) {
                    case 1:
                        bullets.push_back(Bullet(position));
                        break;

                    case 2: {
                        bullets.push_back(Bullet(position));
                        bullets.push_back(Bullet({ position.x - 25 - horiTwo, position.y + 15 - vertTwo }));
                        bullets.push_back(Bullet({ position.x + 25 + horiTwo, position.y + 15 - vertTwo }));
                        break;
                    }

                    case 3: {
                        bullets.push_back(Bullet(position));
                        bullets.push_back(Bullet({ position.x - 15 - horiThree, position.y - 20 + vertThree  }));
                        bullets.push_back(Bullet({ position.x + 15 + horiThree, position.y - 20 + vertThree  }));
                        bullets.push_back(Bullet({ position.x - 25 - horiTwo, position.y + 15 - vertThree  }));
                        bullets.push_back(Bullet({ position.x + 25 + horiTwo, position.y + 15 - vertThree  }));
                        break;
                    }
                }
            }

            // Update and draw bullets
            for (auto& bullet : bullets) {
                bullet.update();
                bullet.draw();
            }

            bullets.erase(
                std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) {
                    return b.isOffScreen();
                }),
                bullets.end()
            );
        }

        void updatePowerLevel()
        {
            if (IsKeyPressed(KEY_ONE)) powerLevel = 1;
            if (IsKeyPressed(KEY_TWO)) powerLevel = 2;
            if (IsKeyPressed(KEY_THREE)) powerLevel = 3;
        }

        const std::vector<Bullet>& getBullets() const
        {
            return bullets;
        }

        void detectPlayerCollision(const std::vector<Lethal_Bullet> &l_bullets) 
        {
            for (const Lethal_Bullet& b : l_bullets) {
                if (CheckCollisionCircles(position, radius, b.position, b.radius)) {
                    isDeath = true;
                    break;
                }
            }
        }

        bool getDeathStatus() 
        {
            return isDeath;
        }

};

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    bool shouldGameOver;

    Player player(5, {400, 300}, MAROON);


    Dummy_enemy enemy;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);

    while (!WindowShouldClose()) 
    {
      BeginDrawing();

      ClearBackground(RAYWHITE);

      player.detectPlayerCollision(
          enemy.getLethalBullets());            // Check for collisions first
      shouldGameOver = player.getDeathStatus(); // Update Game Over state here

      if (shouldGameOver) {
        ClearBackground(RED);
        DrawText("GAME OVER", GetScreenWidth() / 2 - 100, GetScreenHeight() / 2,
                 25, GRAY);
      } else {
        player.spawn_player();
        player.updatePowerLevel();
        player.move();
        player.clamp();
        player.player_shooting();

        enemy.updateBullets();
        enemy.shootRadialPattern(GetFrameTime());

        enemy.draw();
        enemy.drawHealtEnemey();
        enemy.checkCollisionWithBullets(player.getBullets());
        enemy.drawBullets();
      }

      EndDrawing();
    }

    CloseWindow();
    return 0;
}
