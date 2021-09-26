#ifndef button_h
#define button_h

#include "ResourcePath.hpp"

// Button textures from https://www.clickminded.com/button-generator/
// Font: Calibri, Size: 26, Width: 192, Height: 67, Corner Radius: 30, Border Size: 4
class Button {
private:
    sf::Vector2f pos;
    sf::Vector2f scale;
    sf::Vector2f size;
    sf::Texture texture;
    sf::Sprite sprite;
    
public:
    Button(sf::Vector2f Pos, string TextureFileName, sf::Vector2f Scale = sf::Vector2f(1, 1)) {
        pos = Pos;
        scale = Scale;
        
        texture.loadFromFile(resourcePath() + TextureFileName);
        
        sprite.setTexture(texture);
        sprite.setScale(scale);
        sprite.setPosition(pos);
        size = sf::Vector2f(sprite.getLocalBounds().width, sprite.getLocalBounds().height);
    }
    
    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
    
    void setPos(sf::Vector2f newPos) {
        pos = newPos;
        sprite.setPosition(pos);
    }
    
    bool containsPoint(sf::Vector2f point) {
        if (pos.x < point.x && point.x < pos.x + size.x && pos.y < point.y && point.y < pos.y + size.y) {
            return true;
        }
        
        return false;
    }
};

#endif
