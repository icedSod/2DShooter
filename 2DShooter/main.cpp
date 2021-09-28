#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <cmath>
#include "player.h"
#include "grid.h"
#include "button.h"
#include "menu.h"
#include "ResourcePath.hpp"
#include <json/value.h>
#include <json/json.h>

sf::Packet& operator <<(sf::Packet& packet, Player& p) {
    sf::Vector2f playerPos = p.getPos();
    sf::Vector2f rayPoint = p.getRayPoint();
    int bulletNumber = p.newBullets.size();
    
    return packet << playerPos.x << playerPos.y << rayPoint.x << rayPoint.y << bulletNumber;
}

sf::Vector2f centerRect(sf::Vector2f rectSize, sf::Vector2f windowSize) {
    return sf::Vector2f(windowSize.x / 2 - rectSize.x / 2, windowSize.y / 2 - rectSize.y / 2);
}

bool checkJsonErrors(Json::Value config) {
    vector<bool> checker;
    
    checker.push_back(config["window"]["size"][0].isUInt());
    checker.push_back(config["window"]["size"][1].isUInt());
    checker.push_back(config["window"]["fps"].isUInt());
    checker.push_back(config["window"]["title"].isString());
    checker.push_back(config["player"]["pos"][0][0].isDouble());
    checker.push_back(config["player"]["pos"][0][1].isDouble());
    checker.push_back(config["player"]["pos"][1][0].isDouble());
    checker.push_back(config["player"]["pos"][1][1].isDouble());
    checker.push_back(config["player"]["size"].isDouble());
    checker.push_back(config["player"]["speed"].isDouble());
    checker.push_back(config["player"]["color"][0][0].isInt());
    checker.push_back(config["player"]["color"][0][1].isInt());
    checker.push_back(config["player"]["color"][0][2].isInt());
    checker.push_back(config["player"]["color"][1][0].isInt());
    checker.push_back(config["player"]["color"][1][1].isInt());
    checker.push_back(config["player"]["color"][1][2].isInt());
    checker.push_back(config["player"]["spawnpoint"][0].isDouble());
    checker.push_back(config["player"]["spawnpoint"][1].isDouble());
    checker.push_back(config["player"]["ray_precision"].isDouble());
    checker.push_back(config["grid"]["pos"][0].isDouble());
    checker.push_back(config["grid"]["pos"][1].isDouble());
    checker.push_back(config["grid"]["size"][0].isDouble());
    checker.push_back(config["grid"]["size"][1].isDouble());
    checker.push_back(config["grid"]["tile_size"].isDouble());
    checker.push_back(config["grid"]["map"].isString());
    checker.push_back(config["text"]["font"].isString());
    checker.push_back(config["text"]["t1_pos"][0].isDouble());
    checker.push_back(config["text"]["t1_pos"][1].isDouble());
    checker.push_back(config["text"]["t2_pos"][0].isDouble());
    checker.push_back(config["text"]["t2_pos"][1].isDouble());
    checker.push_back(config["text"]["t1_font_size"].isInt());
    checker.push_back(config["text"]["t2_font_size"].isInt());
    checker.push_back(config["text"]["t1_color"][0].isInt());
    checker.push_back(config["text"]["t1_color"][1].isInt());
    checker.push_back(config["text"]["t1_color"][2].isInt());
    checker.push_back(config["text"]["t2_color"][0].isInt());
    checker.push_back(config["text"]["t2_color"][1].isInt());
    checker.push_back(config["text"]["t2_color"][2].isInt());
    checker.push_back(config["cooldown_bar"]["pos"][0].isDouble());
    checker.push_back(config["cooldown_bar"]["pos"][1].isDouble());
    checker.push_back(config["cooldown_bar"]["size"][0].isDouble());
    checker.push_back(config["cooldown_bar"]["size"][1].isDouble());
    checker.push_back(config["cooldown_bar"]["color"][0].isInt());
    checker.push_back(config["cooldown_bar"]["color"][1].isInt());
    checker.push_back(config["cooldown_bar"]["color"][2].isInt());
    checker.push_back(config["cooldown_bar"]["color"][3].isInt());
    
    for (bool b : checker) {
        if (!b) {
            return false;
        }
    }
    
    return true;
}

int runMainMenu(sf::RenderWindow& window, Menu& mainMenu, sf::TcpSocket& socket) {
    sf::Vector2i mousePos;
    sf::Vector2f buttonIdx;
    
    // MARK: - Menu loop
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
    // MARK: - Config file
    ifstream configFile(resourcePath() + "config.json");
    Json::Value config;
    Json::Reader reader;
    reader.parse(configFile, config);
    
    if (!checkJsonErrors(config)) {
        return -1;
    }
    
    // window
    Json::Value cfgWindowSize = config["window"]["size"];
    int cfgFps = config["window"]["fps"].asInt();
    string cfgTitle = config["window"]["title"].asString();
    // player
    float playerSize = config["player"]["size"].asFloat();
    float playerSpeed = config["player"]["speed"].asFloat();
    float rayPrecision = config["player"]["ray_precision"].asFloat();
    Json::Value cfgP1Pos = config["player"]["pos"][0];
    Json::Value cfgP2Pos = config["player"]["pos"][1];
    Json::Value cfgP1Color = config["player"]["color"][0];
    Json::Value cfgP2Color = config["player"]["color"][1];
    Json::Value cfgSpawnpoint = config["player"]["spawnpoint"];
    // grid
    Json::Value cfgGridPos = config["grid"]["pos"];
    Json::Value cfgGridSize = config["grid"]["size"];
    float gridTileSize = config["grid"]["tile_size"].asFloat();
    string mapName = config["grid"]["map"].asString();
    // text
    string fontName = config["text"]["font"].asString();
    Json::Value cfgT1Pos = config["text"]["t1_pos"];
    Json::Value cfgT2Pos = config["text"]["t2_pos"];
    int t1FontSize = config["text"]["t1_font_size"].asInt();
    int t2FontSize = config["text"]["t1_font_size"].asInt();
    Json::Value cfgT1Color = config["text"]["t1_color"];
    Json::Value cfgT2Color = config["text"]["t2_color"];
    // cooldown_bar
    Json::Value cfgBarPos = config["cooldown_bar"]["pos"];
    Json::Value cfgBarSize = config["cooldown_bar"]["size"];
    Json::Value cfgBarColor = config["cooldown_bar"]["color"];
    
    // MARK: - Multiplayer
    sf::TcpSocket socket;
    bool multiplayer = false;
    int plr = 0;
    
    socket.setBlocking(false);
    
    // MARK: - Window settings
    const int fps = cfgFps;
    sf::Vector2u windowSize(cfgWindowSize[0].asUInt(), cfgWindowSize[1].asUInt());
    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), cfgTitle);
    window.setFramerateLimit(fps);
    window.setKeyRepeatEnabled(false);
    
    sf::Image icon;
    if (!icon.loadFromFile(resourcePath() + "icon.png")) {
        return -1;
    }
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    
    // MARK: - Player settings
    sf::Vector2f player1Pos(cfgP1Pos[0].asFloat(), cfgP1Pos[1].asFloat());
    sf::Vector2f player2Pos(cfgP2Pos[0].asFloat(), cfgP2Pos[1].asFloat());
    sf::Vector2f spawnpoint(cfgSpawnpoint[0].asFloat(), cfgSpawnpoint[1].asFloat());
    sf::Color player1Color = sf::Color(cfgP1Color[0].asInt(), cfgP1Color[1].asInt(), cfgP1Color[2].asInt());
    sf::Color player2Color = sf::Color(cfgP2Color[0].asInt(), cfgP2Color[1].asInt(), cfgP2Color[2].asInt());
    Player player1(player1Pos, playerSize, playerSpeed, fps, windowSize, player1Color);
    
    vector<Player> players;
    players.push_back(player1);
    
    // MARK: - Grid settings
    sf::Vector2f gridPos(cfgGridPos[0].asFloat(), cfgGridPos[1].asFloat());
    sf::Vector2f gridSize(cfgGridSize[0].asFloat(), cfgGridSize[0].asFloat());
    Grid grid(gridPos, gridSize, gridTileSize);
    grid.readMapFile(resourcePath() + mapName);
    
    // MARK: - Variables
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
    
    // MARK: - Text
    sf::Font font;
    sf::Text playerPosText;
    sf::Text weaponText;
    
    if (!font.loadFromFile(resourcePath() + fontName)) {
        return -1;
    }
    
    // Player coordinate text
    playerPosText.setFont(font);
    playerPosText.setCharacterSize(t1FontSize);
    playerPosText.setFillColor(sf::Color(cfgT1Color[0].asInt(), cfgT1Color[1].asInt(), cfgT1Color[2].asInt()));
    playerPosText.setString('(' + to_string((int) players[plr].centerPoint().x) + ", " + to_string((int) players[plr].centerPoint().y) + ')');
    weaponText.setPosition(sf::Vector2f(cfgT1Pos[0].asDouble(), cfgT1Pos[1].asDouble()));
    
    // Selected weapon text
    weaponText.setFont(font);
    weaponText.setCharacterSize(t2FontSize);
    weaponText.setFillColor(sf::Color(cfgT2Color[0].asInt(), cfgT2Color[1].asInt(), cfgT2Color[2].asInt()));
    weaponText.setString(players[plr].getWeaponString());
    weaponText.setPosition(sf::Vector2f(cfgT2Pos[0].asDouble(), cfgT2Pos[1].asDouble()));
    
    // Create a key for each character
    for (char c : keybinds) {
        keyStates.insert(pair<char, bool>(c, false));
    }
    
    // For looping over pressed keys
    keys.push_back(sf::Keyboard::W);
    keys.push_back(sf::Keyboard::A);
    keys.push_back(sf::Keyboard::S);
    keys.push_back(sf::Keyboard::D);
    
    // MARK: - Cooldown bar
    sf::Vector2f cooldownBarSize(cfgBarSize[0].asDouble(), cfgBarSize[1].asDouble());
    sf::Vector2f cooldownBarPos(cfgBarPos[0].asDouble(), cfgBarPos[1].asDouble());
    sf::RectangleShape cooldownBar(cooldownBarSize);
    cooldownBar.setPosition(cooldownBarPos);
    cooldownBar.setFillColor(sf::Color(cfgBarColor[0].asInt(), cfgBarColor[1].asInt(), cfgBarColor[2].asInt(), cfgBarColor[3].asInt()));
    
    // MARK: - Main menu
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
    
    // MARK: - Button data
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
        
        Player player2(player2Pos, playerSize, playerSpeed, fps, windowSize, player2Color);
        players.push_back(player2);
    }
    
    // MARK: - Main loop
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
        
        // MARK: - Move player
        if (updated) {
            for (auto& [cKey, bVal] : keyStates) {
                if (keyStates[cKey]) {
                    players[plr].move(cKey);
                }
            }
        }
        
        if (multiplayer) {
            // MARK: - Send player data
            sf::Packet packet;
            if (players[plr].moved() || mousePos != oldMousePos || mousePressed || playerShot) {
                // Send data to other client
                packet << players[plr];
                players[plr].sendNewBullets(packet);
                mousePressed = false;
                playerShot = false;
                
                socket.send(packet);
            }
            
            // MARK: - Receive player data
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
        
        // MARK: - Draw ray
        players[plr].updateRay(mousePos, rayPrecision, grid.obstacles);
        for (Player p : players) {
            p.drawRay(window);
        }
        
        // MARK: - Draw grid
        grid.draw(window);
        
        for (int i = 0; i < players.size(); i++) {
            for (int j = 0; j < players[i].bullets.size(); j++) {
                Player& p = players[i];
                
                // MARK: - Draw bullets
                p.bullets[j].update();
                p.bullets[j].draw(window);
                
                // MARK: - Destroy bullet
                if (p.bullets[j].checkBoundaries()) {
                    p.bullets.erase(p.bullets.begin() + j);
                    j--;
                }
                
                // MARK: - Player bullet collision
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
                        // MARK: - Bullet obstacle collision
                        p.bullets.erase(p.bullets.begin() + i);
                        i--;
                    }
                }
            }
            
            // MARK: - Player obstacle collision
            float playerSize = players[plr].getSize();
            sf::Vector2f playerPos = players[plr].getPos();
            
            if (obstacle.circleCollision(playerPos, playerSize)) {
                players[plr].setPos(players[plr].getOldPos());
            }
            
            // MARK: - Draw obstacles
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
        
        // MARK: - Draw cooldown bar
        window.draw(cooldownBar);
        
        // MARK: - Draw text
        window.draw(playerPosText);
        window.draw(weaponText);
        
        // MARK: - Draw players
        for (Player p : players) {
            p.draw(window);
        }
        
        oldMousePos = mousePos;
        
        window.display();
    }
    
    return 0;
}
