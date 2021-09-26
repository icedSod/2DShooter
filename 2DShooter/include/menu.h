#ifndef menu_h
#define menu_h

class Menu {
private:
    vector< vector<Button> > buttons;
    bool runMenu;
    int layer;
    
public:
    Menu(vector< vector<Button> > Buttons) {
        buttons = Buttons;
        runMenu = true;
        layer = 0;
    }
    
    void draw(sf::RenderWindow& window) {
        for (Button button : buttons[layer]) {
            button.draw(window);
        }
    }
    
    sf::Vector2f containsPoint(sf::Vector2f point) {
        for (int i = 0; i < buttons[layer].size(); i++) {
            if (buttons[layer][i].containsPoint(point)) {
                return sf::Vector2f(layer, i);
            }
        }
        
        return sf::Vector2f(-1, -1);
    }
    
    bool run(int b = -1) {
        if (b == 0 || b == 1) {
            runMenu = b;
        }
        
        return runMenu;
    }
    
    void setLayer(int val) {
        layer = val;
    }
};

#endif
