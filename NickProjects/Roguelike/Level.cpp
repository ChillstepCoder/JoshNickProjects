#include "Level.h"
#include "Player.h"
#include "Gamesystem.h"
#include <fstream>
#include <iostream>
#include <cstdio>

Level::Level()
{

}

void Level::load(std::string fileName, Player& player, const vector<Shop>& shops)
{

    _shops = shops;

    //Loads the level

    _levelData.clear();
    _enemies.clear();

    std::ifstream file;

    file.open(fileName);
    if (file.fail()) {
        perror(fileName.c_str());
        system("PAUSE");
        exit(1);
    }

    std::string line;

    while (getline(file, line)) {
        _levelData.push_back(line);
    }

    file.close();

    //Process the level
    char tile;

    for (int i = 0; i < _levelData.size(); i++)
    {
        for (int j = 0; j < _levelData[i].size(); j++)
        {
            tile = _levelData[i][j];

            switch (tile)
            {
            case '@': //Player
                player.setPosition(j, i);
                break;
            case 'S': //Skeleton
                _enemies.push_back(Enemy("Skeleton", tile, 1, 4, 1, 10, 20)); //level, attack, defense, health, xp)
                _enemies.back().setPosition(j, i);
                break;
            case 'W': //Skeleton Warrior
                _enemies.push_back(Enemy("Skeleton Warrior", tile, 5, 8, 10, 40, 50)); //level, attack, defense, health, xp)
                _enemies.back().setPosition(j, i);
                break;
            case 'K': //Skeleton Knight
                _enemies.push_back(Enemy("Skeleton Knight", tile, 10, 18, 20, 80, 250)); //level, attack, defense, health, xp)
                _enemies.back().setPosition(j, i);
                break;
            case 'L': //Lich
                _enemies.push_back(Enemy("Lich", tile, 25, 40, 40, 300, 1000)); //level, attack, defense, health, xp)
                _enemies.back().setPosition(j, i);
                break;
            case 'D': //Demon
                _enemies.push_back(Enemy("Demon", tile, 45, 150, 150, 400, 1500)); //level, attack, defense, health, xp)
                _enemies.back().setPosition(j, i);
                break;
            case 'A': //Archdevil
                _enemies.push_back(Enemy("Archdevil", tile, 75, 600, 600, 800, 20000)); //level, attack, defense, health, xp)
                _enemies.back().setPosition(j, i);
                break;
            case 'P': //Primeval Demon King
                _enemies.push_back(Enemy("Primeval Demon King", tile, 100, 1800, 1400, 5000, 1000000)); //level, attack, defense, health, xp)
                _enemies.back().setPosition(j, i);
                break;
            }
        }
    }
}
void Level::printvoid()
{
    std::cout << std::string(100, '\n');
}

void Level::print()
{
    printf("\n");

    for (int i = 0; i < _levelData.size(); i++)
    {
        printf("%s\n", _levelData[i].c_str());
    }
    printf("\n");
}

void Level::movePlayer(char input, Player& player)
{
    int playerX;
    int playerY;

    player.getPosition(playerX, playerY);

    char moveTile;

    switch (input)
    {
    case 'w': //up
    case 'W':
        processPlayerMove(player, playerX, playerY - 1);
        break;
    case 's': //down
    case 'S':
        processPlayerMove(player, playerX, playerY + 1);
        break;
    case 'a': //left
    case 'A':
        processPlayerMove(player, playerX - 1, playerY);
        break;
    case 'd': //right
    case 'D':
        processPlayerMove(player, playerX + 1, playerY);
        break;
    case 'i': //inventory
    case 'I':

        break;
    default:
        printf("INVALID INPUT!\n");
        system("PAUSE");
        break;
    }
}

void Level::updateEnemies(Player& player)
{

    char aiMove;
    int playerX;
    int playerY;
    int enemyX;
    int enemyY;

    player.getPosition(playerX, playerY);

    for (int i = 0; i < _enemies.size(); i++)
    {
        aiMove = _enemies[i].getMove(playerX, playerY);
        _enemies[i].getPosition(enemyX, enemyY);

        switch (aiMove)
        {
        case 'w': //up
            processEnemyMove(player, i, enemyX, enemyY - 1);
            break;
        case 's': //down
            processEnemyMove(player, i, enemyX, enemyY + 1);
            break;
        case 'a': //left
            processEnemyMove(player, i, enemyX - 1, enemyY);
            break;
        case 'd': //right
            processEnemyMove(player, i, enemyX + 1, enemyY);
            break;
        }
    }
}

char Level::getTile(int x, int y)
{
    return _levelData[y][x];
}

void Level::setTile(int x, int y, char tile)
{
    _levelData[y][x] = tile;
}

void Level::processPlayerMove(Player& player, int targetX, int targetY)
{
    int playerX;
    int playerY;
    player.getPosition(playerX, playerY);
    std::string nextlevel = "Level2.txt";

    char moveTile = getTile(targetX, targetY);

    switch (moveTile)
    {
    case '.':
        player.setPosition(targetX, targetY);
        setTile(playerX, playerY, '.');
        setTile(targetX, targetY, '@');
        break;
    case '-':
        
        levelnumber++;
        load("Level" + std::to_string(levelnumber) + ".txt", player, _shops);

        break;
    case '_':

        levelnumber--;
        load("Level" + std::to_string(levelnumber) + ".txt", player, _shops);

        break;
    case '$':
    {



        bool isShopping = true;

        // OPEN SHOP
        while (isShopping == true)
        {
            cout << "Shops:\n";
            int i = 1;
            char input;


            for (auto lit = _shops.begin(); lit != _shops.end(); lit++)
            {
                cout << i << ". " << (*lit).getName() << endl;
                i++;
            }

            cout << "\nWhat shop would you like to enter? Q to quit: ";


            cin >> input;



            if (input == 'Q' || input == 'q')
            {

                isShopping = false;
            }
            else
            {
                //works until we have 10 shops
                int shopIndex = input - '1';

                cout << endl;

                bool validShop = false;
                int counter = 0;
                for (auto lit = _shops.begin(); lit != _shops.end(); lit++, counter++)
                {
                    if (counter == shopIndex)
                    {

                        lit->enterShop(player);
                        validShop = true;

                    }
                }
                if (validShop == false)
                {
                    cout << "Invalid Shop!\n";
                    system("PAUSE");
                }
            }
        }
        system("PAUSE");
    }
        break;

    case '#':
        break;
    default:
        battleMonster(player, targetX, targetY);
        break;

    }
}

void Level::processEnemyMove(Player& player, int enemyIndex, int targetX, int targetY)
{
    int playerX;
    int playerY;
    int enemyX;
    int enemyY;

    _enemies[enemyIndex].getPosition(enemyX, enemyY);

    player.getPosition(playerX, playerY);


    char moveTile = getTile(targetX, targetY);

    switch (moveTile)
    {
    case '.':
        _enemies[enemyIndex].setPosition(targetX, targetY);
        setTile(enemyX, enemyY, '.');
        setTile(targetX, targetY, _enemies[enemyIndex].getTile());
        break;
    case '@':
        battleMonster(player, enemyX, enemyY);
        break;
    default:
        break;

    }
}

void Level::battleMonster(Player& player, int targetX, int targetY)
{
    int enemyX;
    int enemyY;
    int playerX;
    int playerY;
    int attackRoll;
    int attackResult;
    std::string enemyName;
    int enemyLevel;
    int enemyAttack;
    int enemyDefense;
    int enemyHealth;
    int enemyExp;
    int enemyGold;

    player.getPosition(playerX, playerY);


    for (int i = 0; i < _enemies.size(); i++)
    {

        _enemies[i].getPosition(enemyX, enemyY);
        enemyName = _enemies[i].getName();
        enemyLevel = _enemies[i].getLevel();
        enemyAttack = _enemies[i].getAttack();
        enemyDefense = _enemies[i].getDefense();
        enemyHealth = _enemies[i].getHealth();
        enemyExp = _enemies[i].getExperience();
        enemyGold = _enemies[i].getGold();

        if (targetX == enemyX && targetY == enemyY)
        {
            //Battle!

            printvoid(); // used to reprint level and make attacks above

            printf("\nLevel %i %s \nAtk: %i   Def: %i   Hp: %i\n\n", enemyLevel, enemyName.c_str(), enemyAttack, enemyDefense, enemyHealth);

            attackRoll = player.attack();
            printf("\nYou attacked %s with a roll of %d\n\n", enemyName.c_str(), attackRoll);
            attackResult = _enemies[i].takeDamage(attackRoll);

            if (attackResult != 0)
            {
                setTile(targetX, targetY, '.');
                printf("%s died!\n\nYou have earned %i gold and %i experience points!\n\n", enemyName.c_str(), enemyGold, enemyExp);
                player.printstats();
                print();

                //remove the enemy
                _enemies[i] = _enemies.back();
                _enemies.pop_back();
                i--;

                system("PAUSE");
                player.addExperience(attackResult);

                return;
            }
            //Monster turn!
            attackRoll = _enemies[i].attack();
            printf("%s attacked you with a roll of %d\n\n", enemyName.c_str(), attackRoll);

            attackResult = player.takeDamage(attackRoll);

            if (attackResult != 0)
            {
                setTile(playerX, playerY, 'x');
                printvoid();
                printf("You died!\n\n");
                player.printstats();
                print();
                system("PAUSE");

                exit(0);
            }

            // used to make the attack rolls be above the stats/level
            player.printstats();
            print();


            system("PAUSE");
            return;
        }
    }
}


