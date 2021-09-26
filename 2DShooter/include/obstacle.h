#ifndef obstacle_h
#define obstacle_h

class Obstacle {
private:
    sf::Vector2f pos;
    sf::Vector2f size;
    sf::RectangleShape shape;
    sf::Color color;
    
public:
    Obstacle(sf::Vector2f Pos, sf::Vector2f Size, sf::Color Color = sf::Color::White) {
        pos = Pos;
        size = Size;
        color = Color;
        
        shape = sf::RectangleShape(size);
        shape.setFillColor(color);
        shape.setPosition(pos);
    }
    
    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }
    
    bool containsPoint(sf::Vector2f point) {
        if (pos.x < point.x && point.x < pos.x + size.x) {
            if (pos.y < point.y && point.y < pos.y + size.y) {
                return true;
            }
        }
        
        return false;
    }
    
    bool circleCollision(sf::Vector2f objPos, float objSize) {
        if (containsPoint(objPos) || containsPoint(sf::Vector2f(objPos.x + objSize * 2, objPos.y)) || containsPoint(sf::Vector2f(objPos.x, objPos.y + objSize * 2)) || containsPoint(sf::Vector2f(objPos.x + objSize * 2, objPos.y + objSize * 2))) {
            return true;
        }
        
        return false;
    }
};

#endif
