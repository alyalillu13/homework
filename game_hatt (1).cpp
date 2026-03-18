#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>

using namespace std;

class Item
{
    string name;
    string type;
    string description;
    int value;
public:
    Item() : name(""), type(""), description(""), value(0) {}
    Item(const string& name, const string& type, const string& desc, int val) : name(name), type(type), description(desc), value(val) {}
    string getName() const { return name; }
    string getType() const { return type; }
    int getValue() const { return value; }
};

class Player
{
    int x, y;
    int health;
    bool hasKey;
    Item weapon;
public:
    Player(int startX, int startY) : x(startX), y(startY), health(50), hasKey(false), weapon("Fist", "weapon", "Your hands", 5) {}
    bool move(int dx, int dy) { x += dx; y += dy; return true; }
    int getX() const { return x; }
    int getY() const { return y; }
    void setPosition(int newX, int newY) { x = newX; y = newY; }
    void takeDamage(int amount) { health -= amount; if (health < 0) health = 0; }
    int getHealth() const { return health; }
    bool getHasKey() const { return hasKey; }
    void setHasKey(bool value) { hasKey = value; }
    void setWeapon(const Item& newWeapon) { weapon = newWeapon; }
    const Item& getWeapon() const { return weapon; }
    bool hasWeapon(const string& name) const { return weapon.getName() == name; }
};

class NPC
{
    int x, y;
    string dialogue[5];
    int dialogueCount;
    string givesWeapon;
    bool alive;
public:
    NPC(int x, int y, const string dlg[], int cnt, const string& weapon) : x(x), y(y), dialogueCount(cnt), givesWeapon(weapon), alive(true)
    {
        for (int i = 0; i < cnt; i++) dialogue[i] = dlg[i];
    }
    const string* getDialogue() const { return dialogue; }
    int getDialogueCount() const { return dialogueCount; }
    string getWeapon() const { return givesWeapon; }
    int getX() const { return x; }
    int getY() const { return y; }
    bool isAlive() const { return alive; }
    void kill() { alive = false; }
};

class Enemy
{
    int x, y;
    int health;
    int damage;
    bool boss;
public:
    Enemy(int x, int y, int hp, int dmg, bool isBoss = false) : x(x), y(y), health(hp), damage(dmg), boss(isBoss) {}
    int getX() const { return x; }
    int getY() const { return y; }
    int getHealth() const { return health; }
    void takeDamage(int amount) { health -= amount; if (health < 0) health = 0; }
    int getDamage() const { return damage; }
    bool isAlive() const { return health > 0; }
    bool isBoss() const { return boss; }
};

class Maze
{
    enum { W = 10, H = 10 };
    char grid[H][W];
    int exitX, exitY;
public:
    Maze(int ex = 8, int ey = 8) : exitX(ex), exitY(ey)
    {
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                grid[y][x] = '.';
        for (int x = 0; x < W; x++)
        {
            grid[0][x] = '#';
            grid[H-1][x] = '#';
        }
        for (int y = 0; y < H; y++)
        {
            grid[y][0] = '#';
            grid[y][W-1] = '#';
        }
        grid[2][2] = '#'; grid[2][3] = '#'; grid[2][4] = '#';
        grid[3][4] = '#';
        grid[4][2] = '#'; grid[4][3] = '#';
        grid[5][2] = '#'; grid[5][3] = '#'; grid[5][4] = '#';
        grid[ey][ex] = 'G';
    }
    bool isWall(int x, int y) const
    {
        if (x < 0 || x >= W || y < 0 || y >= H) return true;
        return grid[y][x] == '#';
    }
    bool isExit(int x, int y) const { return x == exitX && y == exitY; }
    char getCell(int x, int y) const
    {
        if (x < 0 || x >= W || y < 0 || y >= H) return '#';
        return grid[y][x];
    }
    void setCell(int x, int y, char c) { if (x >= 0 && x < W && y >= 0 && y < H) grid[y][x] = c; }
    int getWidth() const { return W; }
    int getHeight() const { return H; }
    int getExitX() const { return exitX; }
    int getExitY() const { return exitY; }
};

class Maze2 : public Maze
{
public:
    Maze2() : Maze(7, 7) {}
};

class Game;
class Presenter
{
    Game* game;
public:
    Presenter(Game* g) : game(g) {}
    void drawGame();
    void drawMaze(const Maze* maze);
    void showDialogue(const string lines[], int cnt);
    bool askYesNo(const string& question);
    char getInput();
    void showTimer(int t) { cout << "   Time left: " << t << endl; }
    void clearScreen() { system("clear"); }
    void showCombatStatus(const Enemy* e, const Player* p);
};

class Game
{
    static const int MAX_LOCS = 2;
    Maze* locs[MAX_LOCS];
    int curLoc;
    Player* player;
    NPC* oldMan;
    Enemy* guard;
    Enemy* oldManBoss;
    Presenter* presenter;
    int timeLeft;
    string state;
    bool helpedOldMan;
    bool oldManDead;
    bool visitedSecondLocation;
    bool returnedForOldMan;
    string lastMsg;
    int wallHits;

public:
    Game() : curLoc(0), player(nullptr), oldMan(nullptr), guard(nullptr), oldManBoss(nullptr),
             presenter(nullptr), timeLeft(0), state(""), helpedOldMan(false), oldManDead(false),
             visitedSecondLocation(false), returnedForOldMan(false), lastMsg(""), wallHits(0)
    {
        locs[0] = locs[1] = nullptr;
    }

    ~Game()
    {
        delete locs[0]; delete locs[1];
        delete player; delete oldMan; delete guard; delete oldManBoss; delete presenter;
    }

    void start()
    {
        locs[0] = new Maze();
        locs[1] = new Maze2();
        player = new Player(1, 1);
        string dlg[] =
        {
            "Hey, they imprisoned you too...",
            "I've been here 33 years...",
            "All I found is this dagger...",
            "I'm too old to escape...",
            "Will you help me?"
        };
        oldMan = new NPC(4, 4, dlg, 5, "old_dagger");
        guard = oldManBoss = nullptr;
        presenter = new Presenter(this);
        timeLeft = 50;
        state = "play";
        lastMsg = "Welcome to the Citadel!";
    }

    void initSecond()
    {
        guard = new Enemy(5, 5, 50, 15, false);
        locs[1]->setCell(1, 1, 'R');
    }

    void checkAutoCombat()
    {
        if (state != "play") return;
        int px = player->getX(), py = player->getY();
        if (guard && guard->isAlive())
        {
            int gx = guard->getX(), gy = guard->getY();
            if (abs(px - gx) <= 1 && abs(py - gy) <= 1)
            {
                state = "combat";
                lastMsg = "You meet the guard! Fight!";
                return;
            }
        }
        if (oldManBoss && oldManBoss->isAlive())
        {
            int bx = oldManBoss->getX(), by = oldManBoss->getY();
            if (abs(px - bx) <= 1 && abs(py - by) <= 1)
            {
                state = "combat";
                lastMsg = "The old man attacks you! Fight!";
                return;
            }
        }
    }

    void setMsg(const string& m) { lastMsg = m; }

    bool tryAttack()
    {
        int px = player->getX(), py = player->getY();
        if (guard && guard->isAlive())
        {
            int gx = guard->getX(), gy = guard->getY();
            if (abs(px - gx) <= 1 && abs(py - gy) <= 1)
            {
                state = "combat";
                lastMsg = "You attack the guard!";
                return true;
            }
        }
        if (oldManBoss && oldManBoss->isAlive())
        {
            int bx = oldManBoss->getX(), by = oldManBoss->getY();
            if (abs(px - bx) <= 1 && abs(py - by) <= 1)
            {
                state = "combat";
                lastMsg = "You attack the old man!";
                return true;
            }
        }
        if (curLoc == 0 && oldMan && oldMan->isAlive())
        {
            int nx = oldMan->getX(), ny = oldMan->getY();
            if (abs(px - nx) <= 1 && abs(py - ny) <= 1)
            {
                oldMan->kill();
                setMsg("You killed the old man.");
                if (!player->hasWeapon("Old Dagger") && !player->hasWeapon("Dual Daggers"))
                    locs[curLoc]->setCell(nx, ny, 'K');
                oldManDead = true;
                return true;
            }
        }
        return false;
    }

    void update(char input)
    {
        if (state == "combat")
        {
            handleCombat(input);
            return;
        }
        if (state != "play") return;
        Maze* maze = locs[curLoc];
        int dx = 0, dy = 0;
        if (input == 'f')
        {
            if (tryAttack()) timeLeft--;
            else setMsg("No enemy nearby.");
            return;
        }
        if (input == 'q')
        {
            cout << "   You quit the game. Goodbye!\n";
            exit(0);
        }
        if (input == 'w') dy = -1;
        else if (input == 's') dy = 1;
        else if (input == 'a') dx = -1;
        else if (input == 'd') dx = 1;
        else
        {
            setMsg("Unknown command.");
            timeLeft--;
            return;
        }
        if (dx != 0 || dy != 0)
        {
            int nx = player->getX() + dx, ny = player->getY() + dy;
            if (curLoc == 0 && oldMan && oldMan->isAlive() && nx == oldMan->getX() && ny == oldMan->getY())
            {
                if (helpedOldMan && visitedSecondLocation && !returnedForOldMan)
                {
                    string finalDlg[] = {"You came back for me! Thank you!"};
                    presenter->showDialogue(finalDlg, 1);
                    setMsg("Together you go through the gate and escape!");
                    returnedForOldMan = true;
                    state = "win";
                    return;
                }
                presenter->showDialogue(oldMan->getDialogue(), oldMan->getDialogueCount());
                Item dagger("Old Dagger", "weapon", "Old but sharp dagger", 20);
                player->setWeapon(dagger);
                setMsg("The old man gives you his dagger.");
                helpedOldMan = presenter->askYesNo("Help the old man? (y/n)");
                if (helpedOldMan) setMsg("You promised to help him.");
                else setMsg("You took the dagger and left.");
                timeLeft--;
                checkAutoCombat();
                return;
            }
            if (curLoc == 1 && maze->getCell(nx, ny) == 'R')
            {
                curLoc = 0;
                player->setPosition(7, 8);
                setMsg("You return to the first level.");
                timeLeft--;
                checkAutoCombat();
                return;
            }
            if (maze->isWall(nx, ny))
            {
                setMsg("Ouch! Wall!");
                player->takeDamage(5);
                wallHits++;
                if (wallHits >= 10)
                {
                    state = "win";
                    setMsg("You smashed the wall with your head! You escaped!");
                    return;
                }
                timeLeft--;
                if (player->getHealth() <= 0)
                {
                    state = "gameover";
                    setMsg("You died...");
                }
                return;
            }
            if (maze->isExit(nx, ny))
            {
                if (curLoc == 0)
                {
                    curLoc = 1;
                    player->setPosition(1, 1);
                    visitedSecondLocation = true;
                    initSecond();
                    setMsg("You enter the second level. A guard awaits!");
                    timeLeft--;
                    checkAutoCombat();
                    return;
                }
                else
                {
                    if (!player->getHasKey())
                    {
                        setMsg("Need a key!");
                        timeLeft--;
                        return;
                    }
                    if ((!helpedOldMan || (helpedOldMan && !returnedForOldMan)) && !oldManDead && oldManBoss == nullptr)
                    {
                        oldManBoss = new Enemy(maze->getExitX(), maze->getExitY(), 30, 20, true);
                        setMsg("The old man blocks your way! Fight!");
                        state = "combat";
                        timeLeft--;
                        return;
                    }
                    if (oldManBoss && oldManBoss->isAlive())
                    {
                        if (abs(player->getX() - oldManBoss->getX()) <= 1 && abs(player->getY() - oldManBoss->getY()) <= 1)
                        {
                            state = "combat";
                            setMsg("The old man attacks!");
                            timeLeft--;
                        }
                        else
                        {
                            setMsg("The old man is nearby but not here.");
                            timeLeft--;
                        }
                        return;
                    }
                    state = "win";
                    setMsg("Congratulations! You escaped!");
                    return;
                }
            }
            player->move(dx, dy);
            timeLeft--;
            char cell = maze->getCell(player->getX(), player->getY());
            if (cell == 'D')
            {
                Item gDagger("Guard Dagger", "weapon", "Guard's dagger", 15);
                if (player->hasWeapon("Old Dagger"))
                {
                    Item dual("Dual Daggers", "weapon", "Two daggers", 35);
                    player->setWeapon(dual);
                    setMsg("You pick up the second dagger! Now you have dual daggers (35 dmg).");
                }
                else if (player->getWeapon().getValue() < 15)
                {
                    player->setWeapon(gDagger);
                    setMsg("You pick up the guard's dagger.");
                }
                else
                {
                    setMsg("You already have a better weapon.");
                }
                maze->setCell(player->getX(), player->getY(), '.');
            }
            else if (cell == 'K')
            {
                Item oDagger("Old Dagger", "weapon", "Old dagger", 20);
                if (player->getWeapon().getValue() < 20)
                {
                    player->setWeapon(oDagger);
                    setMsg("You pick up the old man's dagger.");
                }
                else
                {
                    setMsg("You already have a better weapon.");
                }
                maze->setCell(player->getX(), player->getY(), '.');
            }
            checkAutoCombat();
        }
        if (player->getHealth() <= 0 || timeLeft <= 0)
        {
            state = "gameover";
            setMsg("Game over. You lost.");
        }
    }

    void handleCombat(char input)
    {
        Enemy* enemy = nullptr;
        int px = player->getX(), py = player->getY();
        if (guard && guard->isAlive() && abs(px - guard->getX()) <= 1 && abs(py - guard->getY()) <= 1)
            enemy = guard;
        else if (oldManBoss && oldManBoss->isAlive() && abs(px - oldManBoss->getX()) <= 1 && abs(py - oldManBoss->getY()) <= 1)
            enemy = oldManBoss;
        if (!enemy)
        {
            state = "play";
            return;
        }
        if (input == 'e')
        {
            int dmg = player->getWeapon().getValue();
            if (enemy->isBoss() && !player->hasWeapon("Dual Daggers"))
            {
                setMsg("Your weapon cannot harm the old man! Only dual daggers work.");
                dmg = 0;
            }
            if (dmg > 0)
            {
                enemy->takeDamage(dmg);
                setMsg("You dealt " + to_string(dmg) + " damage!");
            }
            if (!enemy->isAlive())
            {
                setMsg("You defeated the enemy!");
                if (enemy == guard)
                {
                    player->setHasKey(true);
                    setMsg("You got the key.");
                    locs[1]->setCell(guard->getX(), guard->getY(), 'D');
                    delete guard;
                    guard = nullptr;
                }
                else
                {
                    oldManDead = true;
                    delete oldManBoss;
                    oldManBoss = nullptr;
                    setMsg("The old man is dead.");
                }
                state = "play";
                checkAutoCombat();
                return;
            }
            player->takeDamage(enemy->getDamage());
            setMsg("Enemy attacks! You take " + to_string(enemy->getDamage()) + " damage.");
            if (player->getHealth() <= 0)
            {
                state = "gameover";
                setMsg("You died in battle...");
            }
        }
        else if (input == 'r')
        {
            setMsg("You retreat.");
            state = "play";
        }
        else
        {
            setMsg("In combat use 'e' to attack, 'r' to retreat.");
        }
    }

    bool isGameOver() const { return state == "gameover" || state == "win"; }
    bool checkWin() const { return state == "win"; }
    Maze* getMaze() const { return locs[curLoc]; }
    Player* getPlayer() const { return player; }
    NPC* getOldMan() const { return oldMan; }
    Enemy* getGuard() const { return guard; }
    Enemy* getBoss() const { return oldManBoss; }
    Presenter* getPresenter() const { return presenter; }
    int getTime() const { return timeLeft; }
    int getCurLoc() const { return curLoc; }
    const string& getState() const { return state; }
    const string& getLastMsg() const { return lastMsg; }
};

void Presenter::drawGame()
{
    clearScreen();
    cout << "   " << game->getLastMsg() << endl << endl;
    drawMaze(game->getMaze());
    showTimer(game->getTime());
    cout << "   Health: " << game->getPlayer()->getHealth() << "/50" << endl;
    cout << "   Weapon: " << game->getPlayer()->getWeapon().getName() << " (damage " << game->getPlayer()->getWeapon().getValue() << ")" << endl;
    cout << "   Key: " << (game->getPlayer()->getHasKey() ? "yes" : "no") << endl;
}

void Presenter::drawMaze(const Maze* maze)
{
    int w = maze->getWidth(), h = maze->getHeight();
    int px = game->getPlayer()->getX(), py = game->getPlayer()->getY();
    int nx = (game->getCurLoc() == 0 && game->getOldMan() && game->getOldMan()->isAlive()) ? game->getOldMan()->getX() : -1;
    int ny = (game->getCurLoc() == 0 && game->getOldMan() && game->getOldMan()->isAlive()) ? game->getOldMan()->getY() : -1;
    int gx = -1, gy = -1, bx = -1, by = -1;
    if (game->getCurLoc() == 1 && game->getGuard() && game->getGuard()->isAlive())
    {
        gx = game->getGuard()->getX(); gy = game->getGuard()->getY();
    }
    if (game->getCurLoc() == 1 && game->getBoss() && game->getBoss()->isAlive())
    {
        bx = game->getBoss()->getX(); by = game->getBoss()->getY();
    }
    for (int y = 0; y < h; y++)
    {
        cout << "   ";
        for (int x = 0; x < w; x++)
        {
            if (x == px && y == py) cout << '@';
            else if (x == nx && y == ny) cout << 'N';
            else if ((x == gx && y == gy) || (x == bx && y == by)) cout << 'E';
            else cout << maze->getCell(x, y);
        }
        cout << endl;
    }
}

void Presenter::showDialogue(const string lines[], int cnt)
{
    cout << "=== Old man says ===" << endl;
    for (int i = 0; i < cnt; i++) cout << lines[i] << endl;
    cout << "====================" << endl;
}

bool Presenter::askYesNo(const string& question)
{
    char a;
    do
    {
        cout << question << " (y/n): ";
        cin >> a;
        a = tolower(a);
    } while (a != 'y' && a != 'n');
    return a == 'y';
}

void Presenter::showCombatStatus(const Enemy* e, const Player* p)
{
    cout << "=== Combat ===" << endl;
    cout << "   Enemy health: " << e->getHealth() << "/50" << endl;
    cout << "   Your health: " << p->getHealth() << "/50" << endl;
    cout << "   Actions: e - attack, r - retreat" << endl;
}

char Presenter::getInput()
{
    char c;
    if (game->getState() == "combat")
    {
        if (game->getGuard() && game->getGuard()->isAlive())
            showCombatStatus(game->getGuard(), game->getPlayer());
        else if (game->getBoss() && game->getBoss()->isAlive())
            showCombatStatus(game->getBoss(), game->getPlayer());
        cout << "   Combat! Choose action (e - attack, r - retreat): ";
    }
    else
    {
        cout << "   Command (w/a/s/d move, f attack, q quit): ";
    }
    cin >> c;
    return c;
}

int main()
{
    Game game;
    game.start();
    bool running = true;
    while (running)
    {
        game.getPresenter()->drawGame();
        if (game.isGameOver() || game.checkWin())
        {
            cout << "   " << game.getLastMsg() << endl;
            running = false;
        }
        else
        {
            char cmd = game.getPresenter()->getInput();
            game.update(cmd);
            if (game.isGameOver() || game.checkWin())
            {
                cout << "   " << game.getLastMsg() << endl;
                running = false;
            }
        }
    }
    return 0;
}