#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <cmath>
#include "player.h"
#include "grid.h"
#include "button.h"
#include "menu.h"
#include "ResourcePath.hpp"

sf::Packet& operator <<(sf::Packet& packet, Player& p) {
    sf::Vector2f playerPos = p.getPos();
    sf::Vector2f rayPoint = p.getRayPoint();
    int bulletNumber = p.newBullets.size();
    
    return packet << playerPos.x << playerPos.y << rayPoint.x << rayPoint.y << bulletNumber;
}

sf::Vector2f centerRect(sf::Vector2f rectSize, sf::Vector2f windowSize) {
    return sf::Vector2f(windowSize.x / 2 - rectSize.x / 2, windowSize.y / 2 - rectSize.y / 2);
}

int runMainMenu(sf::RenderWindow& window, Menu& mainMenu, sf::TcpSocket& socket) {
    sf::Vector2i mousePos;
    sf::Vector2f buttonIdx;
    
    while (mainMenu.run()) {
        sf::Event event;
        
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed: {
                    window.close();
                    return 0;
                } case sf::Event::MouseButtonPressed: {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        mousePos = sf::Mouse::getPosition(window);
                        buttonIdx = mainMenu.containsPoint((sf::Vector2f) mousePos);
                        
                        if (buttonIdx.x == 0) {
                            if (buttonIdx.y == 0) {
                                // Play
                                mainMenu.setLayer(1);
                            } else if (buttonIdx.y == 1) {
                                // Settings
                            } else {
                                // Exit
                                window.close();
                                return 0;
                            }
                        } else if (buttonIdx.x == 1) {
                            if (buttonIdx.y == 0) {
                                // Singleplayer
                                return 0;
                            } else if (buttonIdx.y == 1) {
                                // Multiplayer
                                mainMenu.setLayer(2);
                            } else {
                                // Back
                                mainMenu.setLayer(buttonIdx.x - 1);
                            }
                        } else if (buttonIdx.x == 2) {
                            if (buttonIdx.y == 0) {
                                // Create server
                                sf::TcpListener listener;
                                listener.listen(2000);
                                listener.accept(socket);
                                
                                return 1;
                            } else if (buttonIdx.y == 1) {
                                // Join server
                                sf::IpAddress ip = sf::IpAddress::getLocalAddress();
                                if (!socket.connect(ip, 2000)) {
                                    return 0;
                                }
                                
                                return 2;
                            } else {
                                // Back
                                mainMenu.setLayer(buttonIdx.x - 1);
                            }
                        }
                    }
                } default: {
                    break;
                }
            }
        }
            
        window.clear(sf::Color::Black);
            
        mainMenu.draw(window);
            
        window.display();
    }
    
    return 0;
}

int main() {
    // Multiplayer
    sf::TcpSocket socket;
    bool multiplayer = false;
    int plr = 0;
    
    socket.setBlocking(false);
    
    // Window settings
    const int fps = 24;
    sf::Vector2u windowSize(800, 800);
    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "2D Shooter");
    window.setFramerateLimit(fps);
    window.setKeyRepeatEnabled(false);
    
    // Player settings
    float playerSize = 20;
    float playerSpeed = 150;
    float rayPrecision = 10;
    sf::Vector2f player1Pos(240 - playerSize, 240 - playerSize);
    sf::Vector2f player2Pos(560 - playerSize, 560 - playerSize);
    sf::Vector2f spawnpoint(0, 0);
    sf::Color player1Color = sf::Color::Red;
    sf::Color player2Color = sf::Color::Blue;
    
    // Grid settings
    sf::Vector2f gridPos(0, 0);
    sf::Vector2f gridSize(10, 10);
    float gridTileSize = 80;
    string mapFileName = "map.txt";
    srand(time(NULL));
    
    // Class objects
    vector<Player> players;
    Grid grid(gridPos, gridSize, gridTileSize);
    Player player1(player1Pos, playerSize, playerSpeed, fps, windowSize, player1Color);
    
    players.push_back(player1);
    grid.readMapFile(resourcePath() + mapFileName);
    
    // Variables
    float reloadTime = 0;
    bool updated = true;
    bool mousePressed = false;
    bool playerShot = false;
    bool bVal;
    char cKey;
    sf::Vector2i mousePos;
    sf::Vector2i oldMousePos;
    sf::Clock clock;
    string keybinds = "wasd";
    vector<sf::Keyboard::Key> keys;
    map<char, bool> keyStates;
    
    // Text
    sf::Font font;
    sf::Text playerPosText;
    sf::Text weaponText;
    string fontFileName = "SourceCodePro-Regular.ttf";
    
    if (!font.loadFromFile(resourcePath() + fontFileName)) {
        return -1;
    }
    
    // Player coordinate text
    playerPosText.setFont(font);
    playerPosText.setCharacterSize(24);
    playerPosText.setString('(' + to_string((int) players[plr].centerPoint().x) + ", " + to_string((int) players[plr].centerPoint().y) + ')');
    
    // Selected weapon text
    weaponText.setFont(font);
    weaponText.setCharacterSize(24);
    weaponText.setFillColor(sf::Color::Black);
    weaponText.setString(players[plr].getWeaponString());
    weaponText.setPosition(0, windowSize.y - weaponText.getLocalBounds().height * 2);
    
    // Create a key for each character
    for (char c : keybinds) {
        keyStates.insert(pair<char, bool>(c, false));
    }
    
    // For looping over pressed keys
    keys.push_back(sf::Keyboard::W);
    keys.push_back(sf::Keyboard::A);
    keys.push_back(sf::Keyboard::S);
    keys.push_back(sf::Keyboard::D);
    
    // Cooldown bar
    sf::Vector2f cooldownBarSize(240, 40);
    sf::Vector2f cooldownBarPos(0, 800 - cooldownBarSize.y);
    sf::RectangleShape cooldownBar(cooldownBarSize);
    cooldownBar.setPosition(cooldownBarPos);
    cooldownBar.setFillColor(sf::Color(214, 204, 197, 200));
    
    // Main menu
    int layerNumber = 3;
    vector< vector<Button> > btnLayers(layerNumber, vector<Button>());
    vector< vector<string> > btnFileNames(layerNumber, vector<string>());
    sf::Texture btnTexture;
    
    btnFileNames[0].push_back("btn-play.png");
    btnFileNames[0].push_back("btn-settings.png");
    btnFileNames[0].push_back("btn-exit.png");
    
    btnFileNames[1].push_back("btn-singleplayer.png");
    btnFileNames[1].push_back("btn-multiplayer.png");
    btnFileNames[1].push_back("btn-back.png");
    
    btnFileNames[2].push_back("btn-createserver.png");
    btnFileNames[2].push_back("btn-joinserver.png");
    
    // Button data
    sf::Vector2f btnSize(200, 75);
    sf::Vector2f centerPos = centerRect(btnSize, (sf::Vector2f) windowSize);
    
    // Layer 1
    Button playBtn(sf::Vector2f(centerPos.x, centerPos.y - (btnSize.y / 2 + 50)), btnFileNames[0][0]);
    Button settingsBtn(sf::Vector2f(centerPos.x, centerPos.y), btnFileNames[0][1]);
    Button exitBtn(sf::Vector2f(centerPos.x, centerPos.y + (btnSize.y / 2 + 50)), btnFileNames[0][2]);
    btnLayers[0].push_back(playBtn);
    btnLayers[0].push_back(settingsBtn);
    btnLayers[0].push_back(exitBtn);
    
    // Layer 2
    Button singleplayerBtn(sf::Vector2f(centerPos.x, centerPos.y - (btnSize.y / 2 + 50)), btnFileNames[1][0]);
    Button multiplayerBtn(sf::Vector2f(centerPos.x, centerPos.y), btnFileNames[1][1]);
    Button backBtn(sf::Vector2f(centerPos.x, centerPos.y + (btnSize.y / 2 + 50)), btnFileNames[1][2]);
    btnLayers[1].push_back(singleplayerBtn);
    btnLayers[1].push_back(multiplayerBtn);
    btnLayers[1].push_back(backBtn);
    
    // Layer 3
    Button createServerBtn(sf::Vector2f(centerPos.x, centerPos.y - (btnSize.y / 2 + 50)), btnFileNames[2][0]);
    Button joinServerBtn(sf::Vector2f(centerPos.x, centerPos.y), btnFileNames[2][1]);
    btnLayers[2].push_back(createServerBtn);
    btnLayers[2].push_back(joinServerBtn);
    btnLayers[2].push_back(backBtn);
    
    Menu mainMenu(btnLayers);
    int menuResult = runMainMenu(window, mainMenu, socket);
    
    if (menuResult == 1 || menuResult == 2) {
        plr = menuResult - 1;
        multiplayer = true;
    }
    
    if (multiplayer) {
        Player player2(player2Pos, playerSize, playerSpeed, fps, windowSize, player2Color);
        players.push_back(player2);
    }
    
    while (window.isOpen()) {
        sf::Event event;
        float time = clock.getElapsedTime().asSeconds();
        float cooldown = time - reloadTime;
        
        mousePos = sf::Mouse::getPosition(window);
        
        for (Player p : players) {
            players[plr].updateOldPos();
        }
        
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                } case sf::Event::GainedFocus: {
                    updated = true;
                    break;
                } case sf::Event::LostFocus: {
                    updated = false;
                    break;
                } case sf::Event::KeyPressed: {
                    for (int i = 0; i < keybinds.length(); i++) {
                        if (event.key.code == keys[i]) {
                            keyStates[keybinds[i]] = true;
                        }
                    }
                    break;
                } case sf::Event::KeyReleased: {
                    for (int i = 0; i < keybinds.length(); i++) {
                        if (event.key.code == keys[i]) {
                            keyStates[keybinds[i]] = false;
                        }
                    }
                    break;
                } case sf::Event::MouseButtonPressed: {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        mousePressed = true;
                        
                        if (players[plr].shoot(mousePos, cooldown)) {
                            reloadTime = time;
                            
                            if (!multiplayer) {
                                players[plr].updateNewBullets();
                            }
                        }
                    } else if (event.mouseButton.button == sf::Mouse::Right) {
                        players[plr].selectWeapon(1);
                        weaponText.setString(players[plr].getWeaponString());
                    }
                    break;
                } default: {
                    break;
                }
            }
        }
        
        // If a key is held down move the player
        if (updated) {
            for (auto& [cKey, bVal] : keyStates) {
                if (keyStates[cKey]) {
                    players[plr].move(cKey);
                }
            }
        }
        
        if (multiplayer) {
            // Send player data
            sf::Packet packet;
            if (players[plr].moved() || mousePos != oldMousePos || mousePressed || playerShot) {
                // Send data to other client
                packet << players[plr];
                players[plr].sendNewBullets(packet);
                mousePressed = false;
                playerShot = false;
                
                socket.send(packet);
            }
            
            // Receive player data
            socket.receive(packet);
            sf::Vector2f p2Pos;
            sf::Vector2f p2RayPoint;
            int bulletNumber;
            
            if (packet >> p2Pos.x >> p2Pos.y >> p2RayPoint.x >> p2RayPoint.y >> bulletNumber) {
                sf::Vector2f bPos;
                sf::Vector2f bDirection;
                sf::Color bColor;
                float bSize;
                float bSpeed;
                
                // Receive data from other client and set it to the other client's player
                for (int i = 0; i < bulletNumber; i++) {
                    if (packet >> bPos.x >> bPos.y >> bDirection.x >> bDirection.y >> bSize >> bSpeed >> bColor.r >> bColor.g >> bColor.b) {
                        Bullet bullet(bPos, bDirection, bSize, bSpeed, fps, windowSize, sf::Color(bColor.r, bColor.g, bColor.b));
                        players[!plr].bullets.push_back(bullet);
                    }
                }
                
                players[!plr].setPos(p2Pos);
                players[!plr].setRayPoint(p2RayPoint);
            }
        }
        
        window.clear(sf::Color::Black);
        
        // Draw ray
        players[plr].updateRay(mousePos, rayPrecision, grid.obstacles);
        for (Player p : players) {
            p.drawRay(window);
        }
        
        // Draw grid
        grid.draw(window);
        
        for (int i = 0; i < players.size(); i++) {
            for (int j = 0; j < players[i].bullets.size(); j++) {
                Player& p = players[i];
                
                // Draw and update bullets
                p.bullets[j].update();
                p.bullets[j].draw(window);
                
                // Destroy bullet when it is off screen
                if (p.bullets[j].checkBoundaries()) {
                    p.bullets.erase(p.bullets.begin() + j);
                    j--;
                }
                
                // Player and bullet collision
                if (multiplayer && i != plr && players[plr].containsPoint(p.bullets[j].centerPoint())) {
                    players[plr].setPos(spawnpoint);
                    playerShot = true;
                }
            }
        }
        
        for (Obstacle obstacle : grid.obstacles) {
            // Check for bullet and obstacle collision
            for (Player& p : players) {
                for (int i = 0; i < p.bullets.size(); i++) {
                    if (obstacle.containsPoint(p.bullets[i].centerPoint())) {
                        // Destroy bullet on obstacle collision
                        p.bullets.erase(p.bullets.begin() + i);
                        i--;
                    }
                }
            }
            
            // Check for player and obstacle collision
            float playerSize = players[plr].getSize();
            sf::Vector2f playerPos = players[plr].getPos();
            
            if (obstacle.circleCollision(playerPos, playerSize)) {
                players[plr].setPos(players[plr].getOldPos());
            }
            
            // Draw obstacles
            obstacle.draw(window);
        }
        
        if (players[plr].moved()) {
            sf::Vector2f playerPos = players[plr].centerPoint();
            playerPosText.setString('(' + to_string((int) playerPos.x) + ", " + to_string((int) playerPos.y) + ')');
        }
        
        sf::Vector2f newCooldownBarSize = sf::Vector2f(cooldown / players[plr].getWeaponCooldown() * cooldownBarSize.x, cooldownBarSize.y);
        
        if (newCooldownBarSize.x > cooldownBarSize.x) {
            newCooldownBarSize = cooldownBarSize;
        }
        
        cooldownBar.setSize(newCooldownBarSize);
        
        // Draw cooldown bar
        window.draw(cooldownBar);
        
        // Draw text
        window.draw(playerPosText);
        window.draw(weaponText);
        
        // Draw players
        for (Player p : players) {
            p.draw(window);
        }
        
        oldMousePos = mousePos;
        
        window.display();
    }
    
    return 0;
}
