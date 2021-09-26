#ifndef grid_h
#define grid_h

#include <fstream>

class Grid {
private:
    sf::Vector2f pos;
    sf::Vector2f size;
    sf::Color gridColor;
    sf::Color obstacleColor;
    vector< vector<sf::Vector2f> > gridLines;
    float tileSize;
    
public:
    vector<Obstacle> obstacles;
    
    Grid(sf::Vector2f Pos, sf::Vector2f Size, float TileSize, sf::Color GridColor = sf::Color::White, sf::Color ObstacleColor = sf::Color::White) {
        pos = Pos;
        size = Size;
        tileSize = TileSize;
        gridColor = GridColor;
        obstacleColor = ObstacleColor;
        
        // Create vertical lines
        for (int i = 1; i < size.x; i++) {
            sf::Vector2f point1(tileSize * i, pos.y);
            sf::Vector2f point2(tileSize * i, pos.y + size.y * tileSize);
            
            vector<sf::Vector2f> temp;
            temp.push_back(point1);
            temp.push_back(point2);
            
            gridLines.push_back(temp);
        }
        
        // Create horizontal lines
        for (int i = 1; i < size.y; i++) {
            sf::Vector2f point1(pos.x, tileSize * i);
            sf::Vector2f point2(pos.x + size.x * tileSize, tileSize * i);
            
            vector<sf::Vector2f> temp;
            temp.push_back(point1);
            temp.push_back(point2);
            
            gridLines.push_back(temp);
        }
    }
    
    void draw(sf::RenderWindow& window) {
        for (vector<sf::Vector2f> vec : gridLines) {
            sf::Vertex line[] = {
                {vec[0], gridColor},
                {vec[1], gridColor}
            };
            
            window.draw(line, 2, sf::Lines);
        }
    }
    
    sf::Vector2f indexToCoord(int idx1, int idx2) {
        sf::Vector2f result;
        result.x = pos.x + idx1 * tileSize;
        result.y = pos.y + idx2 * tileSize;
        
        return result;
    }
    
    void createObstacle(int idx1, int idx2) {
        if (0 <= idx1 && idx1 < size.x && 0 <= idx2 && idx2 < size.y) {
            Obstacle obstacle(indexToCoord(idx1, idx2), sf::Vector2f(tileSize, tileSize));
            obstacles.push_back(obstacle);
        }
    }
    
    void readMapFile(string filePath) {
        ifstream file(filePath);
        string text;
        
        for (int i = 0; getline(file, text); i++) {
            for (int j = 0; j < text.length(); j++) {
                if (text[j] == '1' && j < size.x && i < size.y) {
                    createObstacle(j, i);
                }
            }
        }
    }
};

#endif
