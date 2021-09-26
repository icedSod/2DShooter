#ifndef bullet_h
#define bullet_h

class Bullet {
private:
    sf::Vector2f pos;
    sf::Vector2f direction;
    sf::Vector2u windowSize;
    sf::Color color;
    sf::CircleShape shape;
    
    float size;
    float speed;
    int fps;
    
public:
    Bullet(sf::Vector2f Pos, sf::Vector2f Direction, float Size, float Speed, int Fps, sf::Vector2u WindowSize, sf::Color Color = sf::Color::White) {
        pos = Pos;
        direction = Direction;
        size = Size;
        speed = Speed;
        fps = Fps;
        windowSize = WindowSize;
        color = Color;
        
        shape = sf::CircleShape(size);
        shape.setPosition(pos);
        shape.setFillColor(color);
    }
    
    void setPos(sf::Vector2f newPos) {
        pos = newPos;
        shape.setPosition(pos);
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
    
    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
    
    void update() {
        sf::Vector2f newDirection = normalize(direction);
        float x = pos.x + newDirection.x * speed / fps;
        float y = pos.y + newDirection.y * speed / fps;
        
        setPos(sf::Vector2f(x, y));
    }
    
    bool checkBoundaries() {
        if (!(0 < pos.x + size * 2) || !(pos.x < windowSize.x)) {
            return true;
        } else if (!(0 < pos.y + size * 2) || !(pos.y < windowSize.y)) {
            return true;
        }
        
        return false;
    }
    
    sf::Vector2f centerPoint() {
        sf::Vector2f result = pos;
        result.x += size;
        result.y += size;
        
        return result;
    }
    
    // Get private members
    sf::Vector2f getPos() {
        return pos;
    }
    
    sf::Vector2f getDirection() {
        return direction;
    }
    
    sf::Color getColor() {
        return color;
    }
    
    float getSize() {
        return size;
    }
    
    float getSpeed() {
        return speed;
    }
};

#endif
