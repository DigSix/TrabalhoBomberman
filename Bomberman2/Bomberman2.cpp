#include <iostream>
using namespace std;

#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <time.h>
#include <fstream>

#define m_size 9 /// Please only use >= 5 odd numbers.

#define floor 0
#define wall 1
#define brk_wall 2
#define player 3
#define enemy 4
#define bomb 5
#define plr_and_bb 6
#define explosion 7

#define max_bombs 1

#define bomb_timer 1000
#define explosion_timer 250
#define enemy_timer 500
#define enemy_interval_timer enemy_timer / 3

struct coords {

    int x, y;

};

struct coords {

    int x, y;

};

coords directions[4]{
    {0,-1}, //Up = [0]
    {-1,0}, //Left = [1]
    {0,1}, //Down = [2]
    {1,0}, //Right = [3]
};


bool collision(int map[m_size][m_size], coords obj, int collider, coords direction) {
    if (map[obj.y + direction.y][obj.x + direction.x] == collider) {
        return true;
    }
    else {
        return false;
    }

}

bool timer_check(clock_t start_timer, clock_t end_timer, int timer_size) {

    if (((end_timer - start_timer) / (CLOCKS_PER_SEC / 1000)) >= timer_size) {
        return true;
    }
    else {
        return false;
    }

}

struct Player {

public:
    coords pcoords;
    void player_control(int map[m_size][m_size], char key) {

        static coords player_direction;
        switch (key) {
        case 72: case 'w':
            player_direction = directions[0];
            break;
        case 75: case 'a':
            player_direction = directions[1];
            break;
        case 80: case 's':
            player_direction = directions[2];
            break;
        case 77: case 'd':
            player_direction = directions[3];
            break;
        default:
            return;
        }

        if (!collision(map, pcoords, wall, player_direction) && !collision(map, pcoords, brk_wall, player_direction)) {
            pcoords.x += player_direction.x;
            pcoords.y += player_direction.y;
        }
    }
};

struct Enemy {

public:
    coords ecoords;

private:
    bool is_enemy_moving = false;
    coords sort_direction;
    int sort_range;
    clock_t start, interval;

    void move_enemy(int map[m_size][m_size], coords direction) {
        if (!collision(map, ecoords, wall, direction) && !collision(map, ecoords, brk_wall, direction) && !collision(map, ecoords, bomb, direction)) {
            ecoords.x += direction.x;
            ecoords.y += direction.y;
        }
        else {
            direction = directions[rand() % 4];
            move_enemy(map, direction);
        }
    }

public:
    void enemy_moving(int map[m_size][m_size], clock_t end) {
        if (!is_enemy_moving && timer_check(start, end, enemy_timer)) {
            sort_direction = directions[rand() % 4];
            sort_range = (rand() % 3) + 1;
            interval = clock();
            is_enemy_moving = true;
        }
        if (is_enemy_moving && timer_check(interval, end, enemy_timer)) {
            move_enemy(map, sort_direction);
            interval = clock();
            if (sort_range == 0) {
                start = clock();
                is_enemy_moving = false;
            }
            else {
                sort_range--;
            }
        }
    }
};

struct Bomb {
public:
    coords bcoords;
    int bombs_remaning = 1;
    int range = 1;

private:
    clock_t start, interval;
    bool is_bomb_exploded = false;

    void show_explosion(int map[m_size][m_size]) {
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= range; j++) {
                if (!collision(map, bcoords, wall, directions[i])) {
                    map[bcoords.y + (directions[i].y * j)][bcoords.x + (directions[i].x * j)] = explosion;
                }
            }
        }
        bombs_remaning++;
    }
    void clear_explosion(int map[m_size][m_size]) {
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= range; j++) {
                if (!collision(map, bcoords, wall, directions[i])) {
                    map[bcoords.y + (directions[i].y * j)][bcoords.x + (directions[i].x * j)] = floor;
                }
            }
        }
    }

public:
    void place_bomb(int map[m_size][m_size], coords Player, char key) {
        if (key == 32 || key == ' ') {
            if (bombs_remaning > 0) {
                bcoords.x = Player.x;
                bcoords.y = Player.y;
                bombs_remaning--;
                start = clock();
            }
        }
    }

    void destroy_bomb(int map[m_size][m_size], clock_t end) {
        if (!is_bomb_exploded && timer_check(start, end, bomb_timer)) {
            is_bomb_exploded = true;
            show_explosion(map);
            interval = clock();
        }
        if (is_bomb_exploded && timer_check(interval, end, explosion_timer)) {
            is_bomb_exploded = false;
            clear_explosion(map);
            start = clock();
        }
    }
};

void drawn_m(int map[m_size][m_size], coords pcoords, coords ecoords, coords bcoords, HANDLE color) {

    int i = 0, j = 0;
    for (i = 0; i < m_size; i++) {
        for (j = 0; j < m_size; j++) {

            if (((i == pcoords.y) && (j == pcoords.x)) && ((pcoords.x != bcoords.x) && (pcoords.y != bcoords.y))) { /// Player.
                SetConsoleTextAttribute(color, 12);
                cout << char(3);
            }

            if ((i == ecoords.y) && (j == ecoords.x)) { /// Enemy.
                SetConsoleTextAttribute(color, 15);
                cout << char(30);
            }

            if (((i == bcoords.y) && (j == bcoords.x)) && ((pcoords.x != bcoords.x) && (pcoords.y != bcoords.y))) { /// Bobm.
                SetConsoleTextAttribute(color, 15);
                cout << char(15);
            }

            if (((j == pcoords.x) && (j == bcoords.x)) && ((i == pcoords.y) && (i == bcoords.y))) { /// Player and bomb.
                SetConsoleTextAttribute(color, 15);
                cout << char(3);
            }

            switch (map[i][j]) {

            case floor: /// Floor.
                SetConsoleTextAttribute(color, 0);
                cout << char(219);
                break;

            case wall: /// Solid wall.
                SetConsoleTextAttribute(color, 8);
                cout << char(177);
                break;

            case brk_wall: /// Breakable wall.
                SetConsoleTextAttribute(color, 15);
                cout << char(177);
                break;

            case explosion: /// Kabum!
                SetConsoleTextAttribute(color, 15);
                cout << char(15);
                break;

            }
        }
        cout << "\n";
    }
}

/// Generating map.
void create_map(int map[m_size][m_size], coords& pcoords, coords& ecoords) {

    int i, j;

    for (i = 0; i < m_size; i++) {
        for (j = 0; j < m_size; j++) {

            /// Generating side walls and floors.
            if (i == 0 || i == m_size - 1 || j == 0 || j == m_size - 1) {
                map[i][j] = wall;
            }
            else {
                map[i][j] = floor;
            }

            /// Generating midle walls and breakable walls.
            if (i > 0 && i < m_size - 1 && j > 0 && j < m_size - 1) {

                if (i % 2 == 0 || j % 2 == 0) {
                    map[i][j] = brk_wall;
                }

                if (i % 2 == 0 && j % 2 == 0) {
                    map[i][j] = wall;
                }

                /// Generating side space for the start moves.
                if (((i == 1 || i == m_size - 2) && j < 3) || ((i == 1 || i == m_size - 2) && j > m_size - 4) || ((j == 1 || j == m_size - 2) && i < 3) || ((j == 1 || j == m_size - 2) && i > m_size - 4)) {
                    map[i][j] = floor;
                }

            }

            /// Placing enemy.
            if (i == 1 && j == 1) {
                ecoords.x = j;
                ecoords.y = i;
            }

            /// Placing player.
            if (i == m_size - 2 && j == m_size - 2) {
                map[i][j] = player;
                pcoords.x = j;
                pcoords.y = i;
            }

        }
    }

}

/// Dranwing objects.
void drawn(int map[m_size][m_size], HANDLE color) {
    int i = 0, j = 0;
    for (i = 0; i < m_size; i++) {
        for (j = 0; j < m_size; j++) {

            switch (map[i][j]) {

            case floor: /// Floor.
                SetConsoleTextAttribute(color, 0);
                cout << char(219);
                break;

            case wall: /// Solid wall.
                SetConsoleTextAttribute(color, 8);
                cout << char(177);
                break;

            case brk_wall: /// Breakable wall.
                SetConsoleTextAttribute(color, 15);
                cout << char(177);
                break;

            case player: /// Player.
                SetConsoleTextAttribute(color, 12);
                cout << char(3);
                break;

            case enemy: /// Enemy.
                SetConsoleTextAttribute(color, 15);
                cout << char(30);
                break;

            case bomb: /// Bomb!
                SetConsoleTextAttribute(color, 15);
                cout << char(15);
                break;


            case plr_and_bb: /// Bomb! & Player.
                SetConsoleTextAttribute(color, 15);
                cout << char(3);
                break;

            case explosion: /// Kabum!
                SetConsoleTextAttribute(color, 15);
                cout << char(15);
                break;

            }

            if (j == m_size - 1) {
                cout << "\n";
            }
        }
    }
}

void drawn_time(clock_t start_timer, clock_t end_timer, int minutes) {

    cout << "TIME - " << minutes << ":";
    if ((end_timer - start_timer) / CLOCKS_PER_SEC < 10) {
        cout << "0";
    }
    cout << (end_timer - start_timer) / CLOCKS_PER_SEC << "\n";

}

void drawn_message(string message) {

    cout << "-----------\n";
    cout << "-" << message << "-\n";
    cout << "-----------\n";

}

/// Clearing map
void clear() {
    int i, j;
    for (i = 0; i < m_size + 1; i++) {
        for (j = 0; j < m_size; j++) {
            cout << char(219);
            if (j == m_size - 1) {
                cout << "\n";
            }
        }
    }
}

/// Replacing new and old objects in the map matrix column.

void new_obj(int map[m_size][m_size], int x, int y, int direction[2], int new_obj, int old_obj) {

    map[y + direction[0]][x + direction[1]] = new_obj;
    map[y][x] = old_obj;

}

/// Checking collisions.
bool collision(int map[m_size][m_size], int x, int y, int collider, int direction[2]) {
    if (map[y + direction[0]][x + direction[1]] == collider) {
        return true;
    }
    else {
        return false;
    }

}

/// Controling player.
void player_control(int map[m_size][m_size], int& x, int& y, char key) {

    static int player_direction[2];
    switch (key) {
    case 72: case 'w':
        player_direction[0] = -1;
        player_direction[1] = 0;
        break;
    case 75: case 'a':
        player_direction[0] = 0;
        player_direction[1] = -1;
        break;
    case 80: case 's':
        player_direction[0] = 1;
        player_direction[1] = 0;
        break;
    case 77: case 'd':
        player_direction[0] = 0;
        player_direction[1] = 1;
        break;
    default:
        return;
    }

    if (!collision(map, x, y, wall, player_direction) && !collision(map, x, y, brk_wall, player_direction)) {
        if (collision(map, x, y, bomb, player_direction)) {
            new_obj(map, x, y, player_direction, plr_and_bb, floor);
        }
        else {
            if (map[y][x] == plr_and_bb) {
                new_obj(map, x, y, player_direction, player, bomb);
            }
            else {
                new_obj(map, x, y, player_direction, player, floor);
            }
        }
        y += player_direction[0];
        x += player_direction[1];
    }

}

/// Timer function.
bool timer_check(clock_t start_timer, clock_t end_timer, int timer_size) {

    if (((end_timer - start_timer) / (CLOCKS_PER_SEC / 1000)) >= timer_size) {
        return true;
    }
    else {
        return false;
    }

}

/// Placing bomb.
void place_bomb(int map[m_size][m_size], int x0, int y0, int& x1, int& y1, char key, int& bombs_remaining, clock_t& start_timer) {

        if (key == 32 || key == ' ') {
            if (bombs_remaining > 0) {
                map[y0][x0] = plr_and_bb;
                x1 = x0;
                y1 = y0;
                bombs_remaining--;
                start_timer = clock();
            }
        }

}

/// Destroying bomb.
void destroy_bomb(int map[m_size][m_size], int x, int y, int& bombs_in_inventory) {
    for (int i = 0; i < 4; i++) {
        if (!collision(map, x, y, wall, possibles_directions[i])) {
            new_obj(map, x, y, possibles_directions[i], explosion, explosion);
        }
    }
    bombs_in_inventory++;

}

/// Cleaning explosion area.
void clear_explosion(int map[m_size][m_size]) {
    int i, j;

    for (i = 1; i < m_size - 1; i++) {
        for (j = 1; j < m_size - 1; j++) {
            if (map[i][j] == explosion) {
                map[i][j] = floor;
            }
        }
    }

}

/// Moving enemy.
void move_enemy(int map[m_size][m_size], int& x, int& y, int direction[2]) {

    if (!collision(map, x, y, wall, direction) && !collision(map, x, y, brk_wall, direction) && !collision(map, x, y, bomb, direction)) {
        new_obj(map, x, y, direction, enemy, floor);
        y += direction[0];
        x += direction[1];
    }
    else {
        move_enemy(map, x, y, possibles_directions[rand() % 4]);
    }

}

/// Checking if the player or enemy dies or not.
bool bomb_kill(int map[m_size][m_size], int x, int y) {
    if (map[y][x] == explosion) {
        return true;
    }
    else {
        return false;
    }
}


bool player_die(int map[m_size][m_size], int x, int y, int killer) {

    for (int i = 0; i < 4; i++) {
        if (collision(map, x, y, killer, possibles_directions[i])) {
            return true;
        }
    }
    if (bomb_kill(map, x, y)) {
        return true;
    }
    else {
        return false;
    }

}

/// Manipulating map.
void read_map(int map[m_size][m_size]) {
    ifstream new_map;
    new_map.open("New_map.txt");
    for (int i = 0; i < m_size; i++) {
        for (int j = 0; j < m_size; j++) {
            new_map >> map[i][j];
        }
    }

}

void write_map(int map[m_size][m_size]) {
    ofstream my_map;
    my_map.open("My_map.txt");
    for (int i = 0; i < m_size; i++) {
        for (int j = 0; j < m_size; j++) {
            my_map << map[i][j];
            if (j < m_size - 1) {
                my_map << " ";
            }
            if (j == m_size - 1) {
                my_map << "\n";
            }
        }
    }
    my_map.close();
}


/// Main function hahaha XD.
int main()
{
    srand(time(NULL));
    /// Teacher's code -->
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO     cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);

    short int CX = 0, CY = 0;
    COORD coord;
    coord.X = CX;
    coord.Y = CY;
    /// <-- Teacher's code

    /// Int variables
    int map[m_size][m_size];
    int player_x, player_y, enemy_x, enemy_y, bomb_x, bomb_y;
    int bombs_remaining = max_bombs;
    int enemy_range, sort_direction, enemy_check_range = 0;
    int minutes = 0;

    /// Char variables
    char key{};

    /// Char variables
    bool bomb_exploded = false;
    bool enemy_moving = false;

    /// Clock_t variables
    clock_t start_bomb_timer = clock();
    clock_t start_explosion_timer = clock();
    clock_t start_enemy_timer = clock(), interval_enemy_timer = clock();
    clock_t start_game_timer = clock(), end_game_timer;

    create_map(map, player_x, player_y, enemy_x, enemy_y);
    write_map(map);
    //read_map(map);

    while (true && !player_die(map, player_x, player_y, enemy) && !bomb_kill(map, enemy_x, enemy_y)) {

        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        /// Updating timers
        end_game_timer = clock();

        /// Geting inputs and controling player
        if (_kbhit()) {
            key = _getch();
            player_control(map, player_x, player_y, key);
            place_bomb(map, player_x, player_y, bomb_x, bomb_y, key, bombs_remaining, start_bomb_timer);
        }

        /// Destroing bomb and cleaning the explosion area
        if (bombs_remaining == 0) {
            if (!bomb_exploded && timer_check(start_bomb_timer, end_game_timer, bomb_timer)) {
                destroy_bomb(map, bomb_x, bomb_y, bombs_remaining);
                bomb_exploded = true;
                start_explosion_timer = clock();
            }
        }
        if (bomb_exploded && timer_check(start_explosion_timer, end_game_timer, explosion_timer)) {
            clear_explosion(map);
            bomb_exploded = false;
        }

        /// Sorting range, direction and moving enemy.
        if (!enemy_moving && timer_check(start_enemy_timer, end_game_timer, enemy_timer)) {
            sort_direction = rand() % 4;
            enemy_range = rand() % 3;
            interval_enemy_timer = clock();
            enemy_moving = true;
        }
        if (enemy_moving && timer_check(interval_enemy_timer, end_game_timer, enemy_interval_timer)) {
            move_enemy(map, enemy_x, enemy_y, possibles_directions[sort_direction]);
            interval_enemy_timer = clock();
            if (enemy_check_range == enemy_range) {
                start_enemy_timer = clock();
                enemy_moving = false;
                enemy_check_range = 0;
            }
            else {
                enemy_check_range++;
            }
        }

        if (timer_check(start_game_timer, end_game_timer, 60000)) {
            minutes++;
            start_game_timer = clock();
        }

        /// Drawing objects.

        SetConsoleTextAttribute(out, 15);
        drawn_time(start_game_timer, end_game_timer, minutes);
        drawn(map, out);

    }

    if (player_die(map, player_x, player_y, enemy)) {
        Beep(100, 500);
        Beep(80, 500);
        Beep(100, 500);
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        SetConsoleTextAttribute(out, 0);
        clear();
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        SetConsoleTextAttribute(out, 4);
        drawn_message("YOUD DIED");
        SetConsoleTextAttribute(out, 15);
        drawn_time(start_game_timer, end_game_timer, minutes);
        Beep(80, 500);
        Beep(70, 500);
        Beep(60, 500);
        Beep(40, 1500);
    }

    if (bomb_kill(map, enemy_x, enemy_y)) {
        Beep(587.33, 1000);
        Beep(698.46, 500);
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        SetConsoleTextAttribute(out, 0);
        clear();
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        SetConsoleTextAttribute(out, 3);
        drawn_message("YOUD WIN!");
        SetConsoleTextAttribute(out, 15);
        drawn_time(start_game_timer, end_game_timer, minutes);
        Beep(880, 500);
        Beep(587.33, 1000);
        Beep(698.46, 500);
        Beep(1046.50, 250);
        Beep(987.77, 500);
        Beep(783.99, 500);
        Beep(698.46, 250);
        Beep(783.99, 250);
        Beep(880, 500);
        Beep(587.33, 500);
        Beep(523.25, 250);
        Beep(659.26, 250);
        Beep(587.33, 750);
    }

}
