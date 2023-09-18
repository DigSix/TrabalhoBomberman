#include <iostream>
using namespace std;

#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <time.h>

#define m_size 11 /// Please only use >= 9 odd numbers.

#define floor 0
#define wall 1
#define brk_wall 2
#define player 3
#define enemy 4
#define bomb 5
#define plr_and_bb 6
#define explosion 7

#define up_or_lt -1
#define dn_or_rt 1

#define max_bombs 1

#define bomb_timer 1000
#define explosion_timer 250
#define enemy_timer 300
#define enemy_interval_timer enemy_timer / 3

/// Generating map.
void create_map(int map[m_size][m_size], int& player_x, int& player_y, int& enemy_x, int& enemy_y) {

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
                map[i][j] = enemy;
                enemy_x = j;
                enemy_y = i;
            }

            /// Placing player.
            if (i == m_size - 2 && j == m_size - 2) {
                map[i][j] = player;
                player_x = j;
                player_y = i;
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
                SetConsoleTextAttribute(color, 1);
                cout << char(219);
                break;

            case enemy: /// Enemy.
                SetConsoleTextAttribute(color, 12);
                cout << char(219);
                break;

            case bomb: /// Bomb!
                SetConsoleTextAttribute(color, 15);
                cout << char(219);
                break;


            case plr_and_bb: /// Bomb! & Player.
                SetConsoleTextAttribute(color, 11);
                cout << char(219);
                break;

            case explosion: /// Kabum!
                SetConsoleTextAttribute(color, 14);
                cout << char(88);
                break;

            }

            if (j == m_size - 1) {
                cout << "\n";
            }
        }
    }
}

/// Checking horizontal collision.
bool h_collision(int map[m_size][m_size], int x, int y, int collider, int direction) {

    if (map[y][x + direction] == collider) {
        return true;
    }
    else {
        return false;
    }

}

/// Checking vertical collision.
bool v_collision(int map[m_size][m_size], int x, int y, int collider, int direction) {

    if (map[y + direction][x] == collider) {
        return true;
    }
    else {
        return false;
    }

}

/// Replacing new and old objects in the map matrix column.
void h_new_obj(int map[m_size][m_size], int x, int y, int direction, int new_obj, int old_obj) {

    map[y][x + direction] = new_obj;
    map[y][x] = old_obj;

}

/// Replacing new and old object in the map matrix row
void v_new_obj(int map[m_size][m_size], int x, int y, int direction, int new_obj, int old_obj) {

    map[y + direction][x] = new_obj;
    map[y][x] = old_obj;

}

/// Controling player.
void player_control(int map[m_size][m_size], int& x, int& y, char key) {

    switch (key) {
    
    /// Move up.
    case 72: case 'w':

        if (!v_collision(map, x, y, wall, up_or_lt) && !v_collision(map, x, y, brk_wall, up_or_lt)) {
            if (v_collision(map, x, y, bomb, up_or_lt)) {
                v_new_obj(map, x, y, up_or_lt, plr_and_bb, floor);
            }
            else {
                if (map[y][x] == plr_and_bb) {
                    v_new_obj(map, x, y, up_or_lt, player, bomb);
                }
                else {
                    v_new_obj(map, x, y, up_or_lt, player, floor);
                }
            }
            y--;
        }

        break;
    
     /// Move down.
    case 80: case 's':

        if (!v_collision(map, x, y, wall, dn_or_rt) && !v_collision(map, x, y, brk_wall, dn_or_rt)) {
            if (v_collision(map, x, y, bomb, dn_or_rt)) {
                v_new_obj(map, x, y, dn_or_rt, plr_and_bb, floor);
            }
            else {
                if (map[y][x] == plr_and_bb) {
                    v_new_obj(map, x, y, dn_or_rt, player, bomb);
                }
                else {
                    v_new_obj(map, x, y, dn_or_rt, player, floor);
                }
            }
            y++;
        }

        break;

    /// Move right.
    case 77:case 'd':

        if (!h_collision(map, x, y, wall, dn_or_rt) && !h_collision(map, x, y, brk_wall, dn_or_rt)) {
            if (h_collision(map, x, y, bomb, dn_or_rt)) {
                h_new_obj(map, x, y, dn_or_rt, plr_and_bb, floor);
            }
            else {
                if (map[y][x] == plr_and_bb) {
                    h_new_obj(map, x, y, dn_or_rt, player, bomb);
                }
                else {
                    h_new_obj(map, x, y, dn_or_rt, player, floor);
                }
            }
            x++;
        }

        break;

    /// Move left.
    case 75: case 'a':

        if (!h_collision(map, x, y, wall, up_or_lt) && !h_collision(map, x, y, brk_wall, up_or_lt)) {
            if (h_collision(map, x, y, bomb, up_or_lt)) {
                h_new_obj(map, x, y, up_or_lt, plr_and_bb, floor);
            }
            else {
                if (map[y][x] == plr_and_bb) {
                    h_new_obj(map, x, y, up_or_lt, player, bomb);
                }
                else {
                    h_new_obj(map, x, y, up_or_lt, player, floor);
                }
            }
            x--;
        }

        break;

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
                max_bomb_in_map--;
                start_timer = clock();
            }
        }

}

/// Destroing bomb.
void destroy_bomb(int map[m_size][m_size], int x, int y, int& max_bomb_in_map) {

    if (!v_collision(map, x, y, wall, up_or_lt)) {
        v_new_obj(map, x, y, up_or_lt, explosion, explosion);
    }
    if (!v_collision(map, x, y, wall, dn_or_rt)) {
        v_new_obj(map, x, y, dn_or_rt, explosion, explosion);
    }
    if (!h_collision(map, x, y, wall, dn_or_rt)) {
        h_new_obj(map, x, y, dn_or_rt, explosion, explosion);
    }
    if (!h_collision(map, x, y, wall, up_or_lt)) {
        h_new_obj(map, x, y, up_or_lt, explosion, explosion);
    }
    max_bomb_in_map++;

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
void move_enemy(int map[m_size][m_size], int& x, int& y, int direction) {

    switch (direction) {

    case 0:
        if (!v_collision(map, x, y, wall, up_or_lt) && !v_collision(map, x, y, brk_wall, up_or_lt)) {
            v_new_obj(map, x, y, up_or_lt, enemy, floor);
            y--;
        }
        break;

    case 1:
        if (!v_collision(map, x, y, wall, dn_or_rt) && !v_collision(map, x, y, brk_wall, dn_or_rt)) {
            v_new_obj(map, x, y, dn_or_rt, enemy, floor);
            y++;
        }
        break;

    case 2:
        if (!h_collision(map, x, y, wall, dn_or_rt) && !h_collision(map, x, y, brk_wall, dn_or_rt)) {
            h_new_obj(map, x, y, dn_or_rt, enemy, floor);
            x++;
        }
        break;

    case 3:
        if (!h_collision(map, x, y, wall, up_or_lt) && !h_collision(map, x, y, brk_wall, up_or_lt)) {
            h_new_obj(map, x, y, up_or_lt, enemy, floor);
            x--;
        }
        break;

    }
}

/// Checking if the player or enemy dies or not.
bool player_or_enemy_die(int map[m_size][m_size], int x, int y, int killer) {

    if ( map[y][x] == explosion) {
        return true;
    }
    if (v_collision(map, x, y, killer, up_or_lt) || v_collision(map, x, y, killer, dn_or_rt) || h_collision(map, x, y, killer, up_or_lt) || h_collision(map, x, y, killer, dn_or_rt)) {
        return true;
    }
    else {
        return false;
    }

}


/// Main function hahaha XD.
int main()
{

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
    clock_t start_bomb_timer = clock(), end_bomb_timer;
    clock_t start_explosion_timer = clock(), end_explosion_timer;
    clock_t start_enemy_timer = clock(), interval_enemy_timer = clock(), end_enemy_timer;
    clock_t start_game_timer = clock(), end_game_timer;

    create_map(map, player_x, player_y, enemy_x, enemy_y);

    while (true && !player_or_enemy_die(map, player_x, player_y, enemy) && !player_or_enemy_die(map, enemy_x, enemy_y, bomb)) {

        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        /// Updating timers
        end_bomb_timer = clock();
        end_explosion_timer = clock();
        end_enemy_timer = clock();
        end_game_timer = clock();

        /// Geting inputs and controling player
        if (_kbhit()) {
            key = _getch();
            player_control(map, player_x, player_y, key);
            place_bomb(map, player_x, player_y, bomb_x, bomb_y, key, bombs_remaining, start_bomb_timer);
        }

        /// Destroing bomb and cleaning the explosion area
        if (max_bomb_in_map == 0) {
            if (!bomb_exploded && timer_check(start_bomb_timer, end_bomb_timer, bomb_timer)) {
                destroy_bomb(map, bomb_x, bomb_y, bombs_remaining);
                bomb_exploded = true;
                start_explosion_timer = clock();
            }
        }
        if (bomb_exploded && timer_check(start_explosion_timer, end_explosion_timer, explosion_timer)) {
            clear_explosion(map);
            bomb_exploded = false;
        }

        /// Sorting range, direction and moving enemy.
        if (!enemy_moving && timer_check(start_enemy_timer, end_enemy_timer, enemy_timer)) {
            sort_direction = rand() % 4;
            enemy_range = rand() % 3;
            interval_enemy_timer = clock();
            enemy_moving = true;
        }
        if (enemy_moving && timer_check(interval_enemy_timer, end_enemy_timer, enemy_interval_timer)) {
            move_enemy(map, enemy_x, enemy_y, sort_direction);
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

        /// HUD?
        SetConsoleTextAttribute(out, 15);
        cout << "TIME - " << minutes << ":";
        if ((end_game_timer - start_game_timer) / CLOCKS_PER_SEC < 10){
            cout << "0";
        }
        cout << (end_game_timer - start_game_timer) / CLOCKS_PER_SEC << "\n";
        SetConsoleTextAttribute(out, 11);
        cout << "--(" << player_x << ", " << player_y << ")---\n";
        SetConsoleTextAttribute(out, 12);
        cout << "--(" << enemy_x << ", " << enemy_y << ")---\n";

        /// Drawing objects.
        drawn(map, out);
    
    }

    if (player_or_enemy_die(map, player_x, player_y, enemy)) {
        SetConsoleTextAttribute(out, 4);
        cout << "\t--------\n";
        cout << "\tYOU DIED\n";
        cout << "\t--------\n";
        SetConsoleTextAttribute(out, 15);
        cout << "\t--------\n";
        cout << "\t--" << minutes << ":";
        if ((end_game_timer - start_game_timer) / CLOCKS_PER_SEC < 10) {
            cout << "0";
        }
        cout << (end_game_timer - start_game_timer) / CLOCKS_PER_SEC << "--\n";
        cout << "\t--------\n";
        SetConsoleTextAttribute(out, 0);
    }
    if (player_or_enemy_die(map, enemy_x, enemy_y, bomb)) {
        SetConsoleTextAttribute(out, 3);
        cout << "\t--------\n";
        cout << "\tYOU WIN!\n";
        cout << "\t--------\n";
        SetConsoleTextAttribute(out, 15);
        cout << "\t--------\n";
        cout << "\t--" << minutes << ":";
        if ((end_game_timer - start_game_timer) / CLOCKS_PER_SEC < 10) {
            cout << "0";
        }
        cout << (end_game_timer - start_game_timer) / CLOCKS_PER_SEC << "--\n";
        cout << "\t--------\n";
        Beep(587.33, 1000);
        Beep(698.46, 500);
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
        SetConsoleTextAttribute(out, 0);
    }

}
