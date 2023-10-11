#include <iostream>
using namespace std;

#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <time.h>
#include <fstream>

#define m_size 9 /// Please only use >= 5 odd numbers.

#define m_floor 0
#define m_wall 1
#define m_brk_wall 2
#define m_player 3
#define m_enemy 4
#define m_bomb 5
#define m_plr_and_bb 6
#define m_explosion 7

#define max_bombs 1
#define enemy_count 1
#define bomb_timer 1000
#define explosion_timer 250
#define enemy_timer 500
#define enemy_interval_timer enemy_timer / 3

struct coords {

    int x, y;

};

coords directions[4]{
    {0,-1}, //Up = [0]
    {-1,0}, //Left = [1]
    {0,1}, //Down = [2]
    {1,0}, //Right = [3]
};

/// Checking collisions.
bool collision(int map[m_size][m_size], coords position, int collider, coords direction) {
    if (map[position.y + direction.y][position.x + direction.x] == collider) {
        return true;
    }
    else {
        return false;
    }

}

void new_obj(int map[m_size][m_size], coords position, coords direction, int new_obj, int old_obj) {

    map[position.y + direction.y][position.x + direction.x] = new_obj;
    map[position.y][position.x] = old_obj;

}

/// Timer function.
int interval_to_ms(clock_t start_ts, clock_t end_ts) {
    return (end_ts - start_ts) / (CLOCKS_PER_SEC / 1000);
}

bool timer_check(clock_t start_ts, clock_t end_ts, int timer_size) {

    if (interval_to_ms(start_ts, end_ts) >= timer_size) {
        return true;
    }
    else {
        return false;
    }

}

struct Bomb {
public:
    coords position;
    clock_t bomb_start_ts;
    clock_t explosion_start_ts;
    bool exploded = false;

    void start_bomb(int map[m_size][m_size], coords player_position) {
        position.x = player_position.x;
        position.y = player_position.y;
        map[position.y][position.x] = m_plr_and_bb;
        bomb_start_ts = clock();
    }

    void destroy(int map[m_size][m_size]) {
        for (int i = 0; i < 4; i++) {
            if (!collision(map, position, m_wall, directions[i])) {
                new_obj(map, position, directions[i], m_explosion, m_explosion);
            }
        }
        exploded = true;
        explosion_start_ts = clock();
    }

    void clear_explosion(int map[m_size][m_size]) {
        exploded = false;
        int i, j;

            for (i = 1; i < m_size - 1; i++) {
                for (j = 1; j < m_size - 1; j++) {
                    if (map[i][j] == m_explosion) {
                        map[i][j] = m_floor;
                    }
                }
            }
    }

    bool has_killed(int map[m_size][m_size], coords position) {
        if (map[position.y][position.x] == m_explosion) {
            return true;
        }
        else {
            return false;
        }
    }
};

struct Player {

public:
    coords position = { 5,5 };
    int bombs_remaining = max_bombs;
    Bomb bomb;

    void check_bomb_loop(int map[m_size][m_size]) {
        if (bombs_remaining == 0) {
            if (!bomb.exploded && timer_check(bomb.bomb_start_ts, clock(), bomb_timer)) {
                bomb.destroy(map);
            }
        }
        if (bomb.exploded && timer_check(bomb.explosion_start_ts, clock(), explosion_timer)) {
            bomb.clear_explosion(map);
            bombs_remaining++;
        }
    }

    bool check_death(int map[m_size][m_size]) {
        for (int i = 0; i < 4; i++) {
            if (collision(map, position, m_enemy, directions[i])) {
                return true;
            }
        }
        if (bomb.has_killed(map, position)) {
            return true;
        }
        else {
            return false;
        }

    }

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
        case 28: case ' ':
            if (bombs_remaining > 0) {
                bomb.start_bomb(map, position);
                bombs_remaining--;
            }
            break;
        default:
            return;
        }

        if (!collision(map, position, m_wall, player_direction) && !collision(map, position, m_brk_wall, player_direction)) {
            if (collision(map, position, m_bomb, player_direction)) {
                new_obj(map, position, player_direction, m_plr_and_bb, m_floor);
            }
            else {
                if (map[position.y][position.x] == m_plr_and_bb) {
                    new_obj(map, position, player_direction, m_player, m_bomb);
                }
                else {
                    new_obj(map, position, player_direction, m_player, m_floor);
                }
            }

            position.x += player_direction.x;
            position.y += player_direction.y;
        }
    }
};

struct Enemy {

public:
    coords position;
    bool alive = true;
    bool enemy_moving = false;

    clock_t movement_start_ts;
    clock_t interval_start_ts;
    coords sort_direction;
    int sort_range;

    void move_enemy(int map[m_size][m_size], coords direction) {
        if (!collision(map, position, m_wall, direction) && !collision(map, position, m_brk_wall, direction)) {
            new_obj(map, position, direction, m_enemy, m_floor);
            position.x += direction.x;
            position.y += direction.y;
        }
        else {
            direction = directions[rand() % 4];
            move_enemy(map, direction);
        }
    }

    void check_enemy_movement(int map[m_size][m_size]) {

        if (!enemy_moving && timer_check(movement_start_ts, clock(), enemy_timer)) {
            sort_direction = directions[rand() % 4];
            sort_range = (rand() % 3) + 1;
            interval_start_ts = clock();
            enemy_moving = true;
        }
        if (enemy_moving && timer_check(interval_start_ts, clock(), enemy_timer)) {
            move_enemy(map, sort_direction);
            interval_start_ts = clock();
            if (sort_range == 0) {
                movement_start_ts = clock();
                enemy_moving = false;
            }
            else {
                sort_range--;
            }
        }
    }
};

void create_map(int map[m_size][m_size]) {

    int i, j;

    for (i = 0; i < m_size; i++) {
        for (j = 0; j < m_size; j++) {

            if (i == 0 || i == m_size - 1 || j == 0 || j == m_size - 1) {
                map[i][j] = m_wall;
            }
            else {
                map[i][j] = m_floor;
            }

        }
    }

}

/// Dranwing objects.
void draw(int map[m_size][m_size], HANDLE color) {
    int i = 0, j = 0;
    for (i = 0; i < m_size; i++) {
        for (j = 0; j < m_size; j++) {

            switch (map[i][j]) {

            case m_floor: /// m_floor.
                SetConsoleTextAttribute(color, 0);
                cout << char(219);
                break;

            case m_wall: /// Solid m_wall.
                SetConsoleTextAttribute(color, 8);
                cout << char(177);
                break;

            case m_brk_wall: /// Breakable m_wall.
                SetConsoleTextAttribute(color, 15);
                cout << char(177);
                break;

            case m_player: /// Player.
                SetConsoleTextAttribute(color, 12);
                cout << char(3);
                break;

            case m_enemy: /// Enemy.
                SetConsoleTextAttribute(color, 15);
                cout << char(30);
                break;

            case m_bomb: /// Bomb!
                SetConsoleTextAttribute(color, 15);
                cout << char(15);
                break;


            case m_plr_and_bb: /// Bomb! & Player.
                SetConsoleTextAttribute(color, 15);
                cout << char(3);
                break;

            case m_explosion: /// Kabum!
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

void draw_time(clock_t start_ts, clock_t end_ts) {
    int s_time = interval_to_ms(start_ts, end_ts)/1000;
    int seconds = s_time % 60;
    int minutes = s_time / 60;
    cout << "TIME - " << minutes << ":";
    if (seconds < 10) cout << "0";
    cout << seconds << "\n";

}

void draw_message(string message) {

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

void game_loop(Enemy enemies[enemy_count], Player player, int map[m_size][m_size], HANDLE out, COORD coord) {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        /// Updating timers
        static clock_t start_game_ts = clock();
        static char key;
        static int minutes = 0;
        clock_t current_ts = clock();

        /// Geting inputs and controling player
        if (_kbhit()) {
            key = _getch();
            player.player_control(map, key);
        }

        player.check_bomb_loop(map);

        for (int i = 0; i < enemy_count; i++) {
            if (enemies[i].alive) {
                enemies[i].check_enemy_movement(map);
            }
        }

        SetConsoleTextAttribute(out, 15);
        draw_time(start_game_ts, current_ts);
        draw(map, out);

    
    if (!player.check_death(map)) {
        game_loop(enemies, player, map, out, coord);
    }
}

void menu_loop(Enemy enemies[enemy_count], Player player, int map[m_size][m_size], HANDLE out, COORD coord, bool pause, clock_t start_game_ts=0, clock_t current_ts=0) {
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    static char key;
    static int menu_select = 0;

    string menu_options[2] = {
        "Jogar",
        "Sair",
    };

    if (pause) {
        draw_time(start_game_ts, current_ts);
    }
    for (int i = 0; i < 2; i++) {
        if (menu_select == i) { 
            cout << "-> "; 
        }
        else {
            cout << "   ";
        }
        cout << menu_options[i] << "\n";
    }

    while (true) {
        if (_kbhit()) {
            key = _getch();
            switch (key) {
            case ' ':
                switch (menu_select) {
                case 0:
                    game_loop(enemies, player, map, out, coord);
                    return;
                case 1:
                    return;
                }
            case 72: case 'w':
                if (menu_select > 0) menu_select--;
                menu_loop(enemies, player, map, out, coord, pause, start_game_ts, current_ts);
                return;
            case 80: case 's':
                if (menu_select < 1) menu_select++;
                menu_loop(enemies, player, map, out, coord, pause, start_game_ts, current_ts);
                return;
            }
        }
    }
}

int main()
{
    srand(time(NULL));
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO     cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false; 
    SetConsoleCursorInfo(out, &cursorInfo);

    short int CX = 0, CY = 0;
    COORD coord;
    coord.X = CX;
    coord.Y = CY;

    int map[m_size][m_size];
    Player player; 
    Enemy enemies[enemy_count];
    for (int i = 0; i < enemy_count; i++) {
        enemies[i].position = { i + 1, i + 1 };
    }

    create_map(map);
    write_map(map);
    //read_map(map);

    menu_loop(enemies, player, map, out, coord, false);
    return 0;
}
