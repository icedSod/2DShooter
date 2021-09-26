#ifndef weapons_h
#define weapons_h

using namespace std;

class Weapon {
public:
    sf::Color bulletColor;
    sf::Vector2u windowSize;
    int fps;
    float bulletSize;
    float bulletSpeed;
    float cooldown;
};

class Pistol : public Weapon {
public:
    Pistol(sf::Color BulletColor, int Fps, sf::Vector2u WindowSize) {
        bulletColor = BulletColor;
        fps = Fps;
        windowSize = WindowSize;
        
        bulletSize = 5;
        bulletSpeed = 200;
        cooldown = 0.1;
    }
    
    Bullet shoot(sf::Vector2f pos, sf::Vector2f direction) {
        return Bullet(pos, direction, bulletSize, bulletSpeed, fps, windowSize, bulletColor);
    }
};

class Sniper : public Weapon {
public:
    Sniper(sf::Color BulletColor, int Fps, sf::Vector2u WindowSize) {
        bulletColor = BulletColor;
        fps = Fps;
        windowSize = WindowSize;
        
        bulletSize = 7.5;
        bulletSpeed = 750;
        cooldown = 1;
    }
    
    Bullet shoot(sf::Vector2f pos, sf::Vector2f direction) {
        return Bullet(pos, direction, bulletSize, bulletSpeed, fps, windowSize, bulletColor);
    }
};

class Shotgun : public Weapon {
public:
    Shotgun(sf::Color BulletColor, int Fps, sf::Vector2u WindowSize) {
        bulletColor = BulletColor;
        fps = Fps;
        windowSize = WindowSize;
        
        bulletSize = 2.5;
        bulletSpeed = 150;
        cooldown = 0.5;
    }
    
    vector<Bullet> shoot(sf::Vector2f pos, sf::Vector2f direction) {
        float angle = atan2(direction.y, direction.x);
        vector<float> angles;
        vector<Bullet> bullets;
        
        float minOffset = -0.5;
        float maxOffset = 0.5;
        float increase = 0.25;
        
        for (float i = minOffset; i <= maxOffset; i += increase) {
            angles.push_back(angle + i);
        }
        
        for (float a : angles) {
            sf::Vector2f angleDirection(cos(a), sin(a));
            Bullet bullet(pos, angleDirection, bulletSize, bulletSpeed, fps, windowSize, bulletColor);
            bullets.push_back(bullet);
        }
        
        return bullets;
    }
};

#endif
