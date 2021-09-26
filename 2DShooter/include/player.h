#ifndef player_h
#define player_h

#include "bullet.h"
#include "obstacle.h"
#include "weapons.h"

sf::Packet& operator <<(sf::Packet& packet, Bullet& b)
{
    sf::Vector2f bPos = b.getPos();
    sf::Vector2f bDirection = b.getDirection();
    float bSize = b.getSize();
    float bSpeed = b.getSpeed();
    sf::Color bColor = b.getColor();
    
    return packet << bPos.x << bPos.y << bDirection.x << bDirection.y << bSize << bSpeed << bColor.r << bColor.g << bColor.b;
}

class Player {
private:
    sf::Vector2f pos;
    sf::Vector2f oldPos;
    sf::Vector2f rayPoint;
    sf::Vector2u windowSize;
    sf::CircleShape shape;
    sf::Color color;
    Pistol* pistol;
    Sniper* sniper;
    Shotgun* shotgun;
    
    float size;
    float speed;
    int selectedWeapon;
    int weaponNumber;
    int fps;
    
public:
    vector<Bullet> bullets;
    vector<Bullet> newBullets;
    
    Player(sf::Vector2f Pos, float Size, float Speed, int Fps, sf::Vector2u WindowSize, sf::Color Color = sf::Color::White) {
        pos = Pos;
        size = Size;
        speed = Speed;
        fps = Fps;
        windowSize = WindowSize;
        color = Color;
        oldPos = Pos;
        
        pistol = new Pistol(color, fps, windowSize);
        sniper = new Sniper(color, fps, windowSize);
        shotgun = new Shotgun(color, fps, windowSize);
        selectedWeapon = 0;
        weaponNumber = 3;
        
        shape = sf::CircleShape(size);
        shape.setPosition(pos);
        shape.setFillColor(color);
    }
    
    sf::Vector2f centerPoint() {
        sf::Vector2f result = pos;
        result.x += size;
        result.y += size;
        
        return result;
    }
    
    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
    
    void drawRay(sf::RenderWindow& window) {
        sf::Vertex ray[] = {
            {centerPoint(), color},
            {rayPoint, color}
        };
        
        window.draw(ray, 2, sf::Lines);
    }
    
    void setPos(sf::Vector2f newPos) {
        pos = newPos;
        shape.setPosition(pos);
    }
    
    void setRayPoint(sf::Vector2f newPoint) {
        rayPoint = newPoint;
    }
    
    void setWeapon(int number) {
        if (0 <= number && number <= 0) {
            selectedWeapon = number;
        }
    }
    
    void selectWeapon(int number) {
        if (selectedWeapon + number < 0) {
            selectedWeapon = weaponNumber - 1;
        } else if (selectedWeapon + number > weaponNumber - 1) {
            selectedWeapon = 0;
        } else {
            selectedWeapon += number;
        }
    }
    
    bool onScreen(sf::Vector2f point) {
        if (0 <= point.x && point.x <= windowSize.x && 0 <= point.y && point.y <= windowSize.y) {
            return true;
        }
        
        return false;
    }
    
    float squaredHypot(float a, float b) {
        return a * a + b * b;
    }
    
    bool containsPoint(sf::Vector2f point) {
        sf::Vector2f center = centerPoint();
        sf::Vector2f distance(abs(center.x - point.x), abs(center.y - point.y));
        if (squaredHypot(distance.x, distance.y) < size * size) {
            return true;
        }
        
        return false;
    }
    
    bool moved() {
        if (pos != oldPos) {
            return true;
        }
        
        return false;
    }
    
    void checkBoundaries() {
        sf::Vector2f newPos = pos;
        
        if (!(0 < pos.x)) {
            newPos.x = 0;
        } else if (!(pos.x < windowSize.x - size * 2)) {
            newPos.x = windowSize.x - size * 2;
        }
        
        if (!(0 < pos.y)) {
            newPos.y = 0;
        } else if (!(pos.y < windowSize.y - size * 2)) {
            newPos.y = windowSize.y - size * 2;
        }
        
        setPos(newPos);
    }
    
    void move(char direction) {
        switch (direction) {
            case 'w':
                setPos(sf::Vector2f(pos.x, pos.y - speed / fps));
                break;
            case 'a':
                setPos(sf::Vector2f(pos.x - speed / fps, pos.y));
                break;
            case 's':
                setPos(sf::Vector2f(pos.x, pos.y + speed / fps));
                break;
            case 'd':
                setPos(sf::Vector2f(pos.x + speed / fps, pos.y));
                break;
        }
        
        checkBoundaries();
    }
    
    sf::Vector2f getDirection(sf::Vector2f vec) {
        sf::Vector2f center = centerPoint();
        vec.x -= center.x;
        vec.y -= center.y;
        
        return vec;
    }
    
    bool shoot(sf::Vector2i mousePos, float cooldown) {
        sf::Vector2f direction = getDirection((sf::Vector2f) mousePos);
        sf::Vector2f center = centerPoint();
        
        switch (selectedWeapon) {
            case 0: {
                if (cooldown > pistol->cooldown) {
                    sf::Vector2f bulletPos(center.x - pistol->bulletSize, center.y - pistol->bulletSize);
                    Bullet bullet = pistol->shoot(bulletPos, direction);
                    newBullets.push_back(bullet);
                    
                    return true;
                }
                break;
            } case 1: {
                if (cooldown > sniper->cooldown) {
                    sf::Vector2f bulletPos(center.x - sniper->bulletSize, center.y - sniper->bulletSize);
                    Bullet bullet = sniper->shoot(bulletPos, direction);
                    newBullets.push_back(bullet);
                            
                    return true;
                }
                break;
            } case 2: {
                if (cooldown > shotgun->cooldown) {
                    sf::Vector2f bulletPos(center.x - shotgun->bulletSize, center.y - shotgun->bulletSize);
                        
                    vector<Bullet> bulletVec = shotgun->shoot(bulletPos, direction);
                        
                    for (Bullet bullet : bulletVec) {
                        newBullets.push_back(bullet);
                    }
                    
                    return true;
                }
                break;
            }
        }
        
        return false;
    }
    
    string getWeaponString(int number = -1) {
        if (number == -1) {
            number = selectedWeapon;
        }
        
        switch (number) {
            case 0:
                return "Pistol";
            case 1:
                return "Sniper";
            case 2:
                return "Shotgun";
        }
        
        return "";
    }
    
    // From: https://en.sfml-dev.org/forums/index.php?topic=1488.0
    sf::Vector2f normalize(sf::Vector2f source)
    {
        float length = sqrt((source.x * source.x) + (source.y * source.y));
        
        if (length != 0) {
            return sf::Vector2f(source.x / length, source.y / length);
        } else {
            return source;
        }
    }
    
    void sendNewBullets(sf::Packet& packet) {
        for (int i = 0; i < newBullets.size(); i++) {
            packet << newBullets[i];
            bullets.push_back(newBullets[i]);
            newBullets.erase(newBullets.begin() + i);
            i--;
        }
    }
    
    void updateNewBullets() {
        for (int i = 0; i < newBullets.size(); i++) {
            bullets.push_back(newBullets[i]);
            newBullets.erase(newBullets.begin() + i);
            i--;
        }
    }
    
    void updateRay(sf::Vector2i mousePos, float precision, vector<Obstacle> obstacles) {
        sf::Vector2f center = centerPoint();
        sf::Vector2f direction = normalize(getDirection((sf::Vector2f) mousePos));
        
        for (float i = 1; true; i += precision) {
            rayPoint = sf::Vector2f(center.x + direction.x * i, center.y + direction.y * i);
            
            if (!onScreen(rayPoint)) {
                break;
            }
            
            for (Obstacle obstacle : obstacles) {
                if (obstacle.containsPoint(rayPoint)) {
                    return;
                }
            }
        }
    }
    
    void updateOldPos() {
        oldPos = pos;
    }
    
    // Get private members
    sf::Vector2f getPos() {
        return pos;
    }
    
    sf::Vector2f getOldPos() {
        return oldPos;
    }
    
    sf::Vector2f getRayPoint() {
        return rayPoint;
    }
    
    float getSize() {
        return size;
    }
    
    float getWeaponCooldown() {
        switch (selectedWeapon) {
            case 0:
                return pistol->cooldown;
            case 1:
                return sniper->cooldown;
            case 2:
                return shotgun->cooldown;
        }
        
        return 0;
    }
    
    int getSelectedWeapon() {
        return selectedWeapon;
    }
};

#endif
