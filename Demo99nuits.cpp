#pragma warning(disable : 4996)

#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <sstream>

#include "GX.h"
#include "GX TexturedFace3D.h"
#include "GX Objects3D.h"

struct Bullet
{
    V3 pos;
    V3 vel;
    float life;
    float radius;
};

struct Asteroid
{
    V3 pos;
    V3 vel;
    float radius;
    int size; // 3 = gros, 2 = moyen, 1 = petit
};

struct GameData99nuits
{
    GX::Camera Cam;

    Object3D AsteroidMesh;
    Object3D BulletMesh;

    V3 shipPos;
    V3 shipVel;
    float shipYawDeg;
    float shipRadius;

    std::vector<Bullet> bullets;
    std::vector<Asteroid> asteroids;

    float fireCooldown;
    float invulnerabilityTimer;
    int score;
    int lives;
    bool gameOver;

    float arenaHalfWidth;
    float arenaHalfDepth;
};
GameData99nuits G;

static float frand01()
{
    return float(rand()) / float(RAND_MAX);
}

static float frandRange(float a, float b)
{
    return a + (b - a) * frand01();
}

static V3 HeadingFromDeg(float deg)
{
    float rad = deg * PI / 180.0f;  // Conversion en radians
    return V3(std::sin(rad), 0.0f, -std::cos(rad));
}

static float DistanceXZ(const V3& a, const V3& b)
{
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return std::sqrt(dx * dx + dz * dz);
}

static void WrapPosition(V3& p)
{
    if (p.x < -G.arenaHalfWidth)  p.x += 2.0f * G.arenaHalfWidth;
    if (p.x > G.arenaHalfWidth)  p.x -= 2.0f * G.arenaHalfWidth;
    if (p.z < -G.arenaHalfDepth)  p.z += 2.0f * G.arenaHalfDepth;
    if (p.z > G.arenaHalfDepth)  p.z -= 2.0f * G.arenaHalfDepth;
}

static Object3D BuildShipMesh(const V3& center, float yawDeg, bool thrustOn)
{
    Object3D ship;

    V3 f = HeadingFromDeg(yawDeg);
    V3 r(-f.z, 0.0f, f.x);
    V3 top(0.0f, 2.5f, 0.0f);

    V3 nose = center + f * 5.5f;
    V3 rearLeft = center - f * 3.5f - r * 2.8f;
    V3 rearRight = center - f * 3.5f + r * 2.8f;
    V3 roof = center + top;
    V3 belly = center - top * 0.35f;

    Color body = Color::White;
    Color wing = Color::Cyan;

    V3 n1 = prodVect(rearLeft - nose, roof - nose);      n1.normalize();
    V3 n2 = prodVect(roof - nose, rearRight - nose);     n2.normalize();
    V3 n3 = prodVect(rearRight - nose, belly - nose);    n3.normalize();
    V3 n4 = prodVect(belly - nose, rearLeft - nose);     n4.normalize();
    V3 n5 = prodVect(rearRight - rearLeft, roof - rearLeft); n5.normalize();
    V3 n6 = prodVect(belly - rearLeft, rearRight - rearLeft); n6.normalize();

    ship.Triangles.push_back(Triangle(nose, rearLeft, roof, n1, body));
    ship.Triangles.push_back(Triangle(nose, roof, rearRight, n2, body));
    ship.Triangles.push_back(Triangle(nose, rearRight, belly, n3, wing));
    ship.Triangles.push_back(Triangle(nose, belly, rearLeft, n4, wing));
    ship.Triangles.push_back(Triangle(rearLeft, rearRight, roof, n5, body));
    ship.Triangles.push_back(Triangle(rearLeft, belly, rearRight, n6, body));

    if (thrustOn)
    {
        V3 flameTip = center - f * 7.5f;
        V3 flameLeft = center - f * 3.8f - r * 1.0f;
        V3 flameRight = center - f * 3.8f + r * 1.0f;
        V3 flameUp = center - f * 3.8f + V3(0.0f, 1.0f, 0.0f);
        Color fire1 = Color::Yellow;
        Color fire2 = Color::Red;

        V3 nf1 = prodVect(flameLeft - flameTip, flameUp - flameTip); nf1.normalize();
        V3 nf2 = prodVect(flameUp - flameTip, flameRight - flameTip); nf2.normalize();
        V3 nf3 = prodVect(flameRight - flameTip, flameLeft - flameTip); nf3.normalize();

        ship.Triangles.push_back(Triangle(flameTip, flameLeft, flameUp, nf1, fire1));
        ship.Triangles.push_back(Triangle(flameTip, flameUp, flameRight, nf2, fire1));
        ship.Triangles.push_back(Triangle(flameTip, flameRight, flameLeft, nf3, fire2));
    }

    return ship;
}

static void ResetShip()
{
    G.shipPos = V3(0.0f, 0.0f, 0.0f);
    G.shipVel = V3(0.0f, 0.0f, 0.0f);
    G.shipYawDeg = 0.0f;
    G.shipRadius = 4.0f;
    G.invulnerabilityTimer = 2.0f;
}

static void SpawnAsteroid(int size, const V3* forcedPos = nullptr)
{
    Asteroid a;
    a.size = size;
    a.radius = (size == 3) ? 8.0f : (size == 2 ? 5.0f : 3.0f);

    if (forcedPos)
    {
        a.pos = *forcedPos;
    }
    else
    {
        do
        {
            a.pos = V3(frandRange(-G.arenaHalfWidth, G.arenaHalfWidth), 0.0f,
                frandRange(-G.arenaHalfDepth, G.arenaHalfDepth));
        } while (DistanceXZ(a.pos, G.shipPos) < 30.0f);
    }

    float angle = frandRange(0.0f, 360.0f) * PI / 180.0f;
    float speed = (size == 3) ? frandRange(8.0f, 12.0f) : (size == 2 ? frandRange(12.0f, 18.0f) : frandRange(18.0f, 24.0f));
    a.vel = V3(std::cos(angle) * speed, 0.0f, std::sin(angle) * speed);

    G.asteroids.push_back(a);
}

static void SplitAsteroid(const Asteroid& a)        // Division des astéoïdes en 2 plus petits lorsqu'ils sont détruits
{
    if (a.size <= 1) return;

    int newSize = a.size - 1;
    for (int i = 0; i < 2; ++i)
    {
        Asteroid child;
        child.size = newSize;
        child.radius = (newSize == 2) ? 5.0f : 3.0f;
        child.pos = a.pos;

        float angle = frandRange(0.0f, 360.0f) * PI / 180.0f;
        float speed = (newSize == 2) ? frandRange(16.0f, 22.0f) : frandRange(22.0f, 28.0f);
        child.vel = a.vel * 0.35f + V3(std::cos(angle) * speed, 0.0f, std::sin(angle) * speed);
        G.asteroids.push_back(child);
    }
}

static std::string ToString(int v)
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

void demoForestRender(bool isInPause)       // Affichage de tous les éléments du jeu
{
    GX::select3DMode(Color::Black, G.Cam, 1200, 800);

    // Bordure de l'arène
    Object3D wall = Object3D::CreateCube(2.0f, Color::Gray);
    wall.setPivot(2, 1, 2);

    for (int i = -int(G.arenaHalfWidth); i <= int(G.arenaHalfWidth); i += 20)
    {
        GX::BeginLocalFrame(); GX::Translate(V3((float)i, 0.0f, -G.arenaHalfDepth)); wall.forceSize(2.0f); wall.Draw(true, false, false); GX::EndLocalFrame();
        GX::BeginLocalFrame(); GX::Translate(V3((float)i, 0.0f, G.arenaHalfDepth)); wall.Draw(true, false, false); GX::EndLocalFrame();
    }
    for (int i = -int(G.arenaHalfDepth); i <= int(G.arenaHalfDepth); i += 20)
    {
        GX::BeginLocalFrame(); GX::Translate(V3(-G.arenaHalfWidth, 0.0f, (float)i)); wall.Draw(true, false, false); GX::EndLocalFrame();
        GX::BeginLocalFrame(); GX::Translate(V3(G.arenaHalfWidth, 0.0f, (float)i)); wall.Draw(true, false, false); GX::EndLocalFrame();
    }

    // Astéroïdes
    for (const Asteroid& a : G.asteroids)
    {
        GX::BeginLocalFrame();
        GX::Translate(a.pos);
        Object3D rock = G.AsteroidMesh;
        rock.forceSize(a.radius * 2.2f);
        rock.Draw(true, false, false);
        GX::EndLocalFrame();
    }

    // Projectiles
    for (const Bullet& b : G.bullets)
    {
        GX::BeginLocalFrame();
        GX::Translate(b.pos + V3(0.0f, 1.2f, 0.0f));
        Object3D shot = G.BulletMesh;
        shot.forceSize(b.radius * 2.0f);
        shot.Draw(true, false, false);
        GX::EndLocalFrame();
    }

    // Vaisseau
    bool thrustOn = GX::isKeyPressed(Key::Z) && !G.gameOver;
    if (G.invulnerabilityTimer <= 0.0f || (int)(G.invulnerabilityTimer * 10.0f) % 2 == 0)
    {
        Object3D ship = BuildShipMesh(G.shipPos + V3(0.0f, 1.0f, 0.0f), G.shipYawDeg, thrustOn);
        ship.Draw(true, false, false);
    }

    GX::ActivateUImode();
    GX::drawStringFontMono(V3(20, 10, 0), "ASTEROIDS 3D", 45, 2, Color::White);
    GX::drawStringFontMono(V3(20, 55, 0), "Z: thrust   Q/D: rotate   X: fire", 28, 2, Color::White);
    GX::drawStringFontMono(V3(20, 90, 0), ("Score: " + ToString(G.score)).c_str(), 28, 2, Color::Yellow);
    GX::drawStringFontMono(V3(20, 125, 0), ("Lives: " + ToString(G.lives)).c_str(), 28, 2, Color::Cyan);
    GX::drawStringFontMono(V3(20, 160, 0), ("Rocks: " + ToString((int)G.asteroids.size())).c_str(), 28, 2, Color::Gray);

    if (G.gameOver)
    {
        GX::drawStringFontMono(V3(280, 320, 0), "GAME OVER", 60, 3, Color::Red);
        GX::drawStringFontMono(V3(180, 385, 0), "Press W to restart", 34, 2, Color::White);
    }
    else if (isInPause)
    {
        GX::drawStringFontMono(V3(360, 320, 0), "PAUSE", 60, 3, Color::White);
    }
}

void demoForestLogic(float dT)
{
    if (G.gameOver)
    {
        if (GX::isKeyPressed(Key::W))
        {
            G.bullets.clear();
            G.asteroids.clear();
            G.score = 0;
            G.lives = 3;
            G.gameOver = false;
            ResetShip();
            for (int i = 0; i < 5; ++i) SpawnAsteroid(3);
        }
        return;
    }

    // Rotation du vaisseau
    const float turnSpeed = 180.0f;
    if (GX::isKeyPressed(Key::Q)) G.shipYawDeg -= turnSpeed * dT;
    if (GX::isKeyPressed(Key::D)) G.shipYawDeg += turnSpeed * dT;

    // Poussée
    V3 forward = HeadingFromDeg(G.shipYawDeg);
    if (GX::isKeyPressed(Key::Z))
    {
        G.shipVel = G.shipVel + forward * (36.0f * dT);
    }

    // Frein léger pour rester arcade
    G.shipVel = G.shipVel * (1.0f - std::min(0.7f * dT, 0.7f));

    // Limitation vitesse
    float shipSpeed = G.shipVel.norm();
    if (shipSpeed > 32.0f)
        G.shipVel = G.shipVel * (32.0f / shipSpeed);

    G.shipPos = G.shipPos + G.shipVel * dT;
    WrapPosition(G.shipPos);

    // Tir
    G.fireCooldown -= dT;
    if (GX::isMouseLeftButtonPressed() && G.fireCooldown <= 0.0f)
    {
        Bullet b;
        b.radius = 0.8f;
        b.life = 1.4f;
        b.pos = G.shipPos + forward * 6.5f + V3(0.0f, 1.2f, 0.0f);
        b.vel = forward * 70.0f + G.shipVel;
        G.bullets.push_back(b);
        G.fireCooldown = 0.22f;
    }

    // Bullets
    for (Bullet& b : G.bullets)
    {
        b.pos = b.pos + b.vel * dT;
        b.life -= dT;
        WrapPosition(b.pos);
    }
    G.bullets.erase(std::remove_if(G.bullets.begin(), G.bullets.end(),
        [](const Bullet& b) { return b.life <= 0.0f; }), G.bullets.end());

    // Astéroïdes
    for (Asteroid& a : G.asteroids)
    {
        a.pos = a.pos + a.vel * dT;
        WrapPosition(a.pos);
    }

    // Collisions bullet / asteroid
    for (int ib = (int)G.bullets.size() - 1; ib >= 0; --ib)
    {
        bool bulletRemoved = false;
        for (int ia = (int)G.asteroids.size() - 1; ia >= 0; --ia)
        {
            if (DistanceXZ(G.bullets[ib].pos, G.asteroids[ia].pos) <= G.bullets[ib].radius + G.asteroids[ia].radius)
            {
                Asteroid hit = G.asteroids[ia];
                G.asteroids.erase(G.asteroids.begin() + ia);
                G.bullets.erase(G.bullets.begin() + ib);
                G.score += (hit.size == 3) ? 20 : (hit.size == 2 ? 50 : 100);
                SplitAsteroid(hit);
                bulletRemoved = true;
                break;
            }
        }
        if (bulletRemoved) continue;
    }

    // Collisions ship / asteroid
    G.invulnerabilityTimer -= dT;
    if (G.invulnerabilityTimer <= 0.0f)
    {
        for (const Asteroid& a : G.asteroids)
        {
            if (DistanceXZ(G.shipPos, a.pos) <= G.shipRadius + a.radius)
            {
                G.lives--;
                if (G.lives <= 0)
                {
                    G.gameOver = true;
                }
                else
                {
                    ResetShip();
                }
                break;
            }
        }
    }

    // Nouvelle vague
    if (G.asteroids.empty())
    {
        for (int i = 0; i < 6; ++i)
            SpawnAsteroid(3);
    }
}

void demoForestInit()
{
    std::srand((unsigned int)std::time(nullptr));

    G.arenaHalfWidth = 90.0f;
    G.arenaHalfDepth = 60.0f;

    G.Cam.pos = V3(0.0f, 115.0f, 35.0f);
    G.Cam.pitch = -72.0f;
    G.Cam.yaw = 0.0f;

    G.AsteroidMesh = Object3D::CreateSphere(1.0f, 20.0f, Color::Gray);
    G.BulletMesh = Object3D::CreateSphere(1.0f, 30.0f, Color::Yellow);

    G.score = 0;
    G.lives = 3;
    G.fireCooldown = 0.0f;
    G.gameOver = false;

    ResetShip();

    G.asteroids.clear();
    G.bullets.clear();
    for (int i = 0; i < 5; ++i)
        SpawnAsteroid(3);
}
