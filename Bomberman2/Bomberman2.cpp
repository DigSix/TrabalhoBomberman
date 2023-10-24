﻿#include <iostream>
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
#define m_powerup 8

#define power_count 2
#define max_bombs 1
#define powerup_timer 5000
#define bomb_timer 1000
#define explosion_timer 250
#define enemy_timer 500
#define enemy_interval_timer enemy_timer / 3

int map_select = 0;
string map_options[2] = {
    "C:\\Users\\gabim\\source\\repos\\DigSix\\TrabalhoBomberman\\Bomberman2\\map1.txt",
    "C:\\Users\\gabim\\source\\repos\\DigSix\\TrabalhoBomberman\\Bomberman2\\map2.txt"
};

struct coords {
    int x, y;
};

coords directions[5]{
    {0,-1}, //Up = [0]
    {-1,0}, //Left = [1]
    {0,1}, //Down = [2]
    {1,0}, //Right = [3]
    {0,0},
};

bool collision(int **map, coords position, int collider, coords direction) {
    if (map[position.y + direction.y][position.x + direction.x] == collider) {
        return true;
    }
    else {
        return false;
    }

}

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

    void start_bomb(coords player_position) {
        position.x = player_position.x;
        position.y = player_position.y;
        bomb_start_ts = clock();
    }

    void destroy(int **map) {
        for (int i = 0; i < 4; i++) {
            if (map[position.y+directions[i].y][position.x + directions[i].x] == m_brk_wall) {
                map[position.y + directions[i].y][position.x + directions[i].x] = m_floor;
            }
        }
        exploded = true;
        explosion_start_ts = clock();
    }

    void clear_explosion() {
        exploded = false;
    }

    bool has_killed( coords entity_position) {
        if (exploded) {
            for (int i = 0; i < 5; i++) {
                if (entity_position.y == position.y + directions[i].y && entity_position.x == position.x + directions[i].x) {
                    return true;
                }
            }
        }
        return false;
    }
};

struct PowerupStatus {
    bool active = false;
    int usage_count = 0;
    clock_t start_timer = 0;
};

struct Player {

public:
    coords position;
    coords player_direction;
    int bombs_remaining = max_bombs;
    PowerupStatus current_powers[power_count];
    Bomb bomb;

    void get_random_powerup() {
        int powerup_number = rand() % power_count;
        current_powers[powerup_number].active = true;
        current_powers[powerup_number].start_timer = clock();
        current_powers[powerup_number].usage_count++;
    }

    void check_bomb_loop(int **map) {
        clock_t current_ts = clock();
        if (bombs_remaining == 0) {
            if (!bomb.exploded && timer_check(bomb.bomb_start_ts, current_ts, bomb_timer)) {
                bomb.destroy(map);
            }
            else if (bomb.exploded && timer_check(bomb.explosion_start_ts, current_ts, explosion_timer)) {
                bomb.clear_explosion();
                bombs_remaining++;
            }
        }

    }

    void check_powerups_loop() {
        clock_t current_ts = clock();
        for (int i = 0; i < power_count; i++) {
            if (current_powers[i].active) {
                if (timer_check(current_powers[i].start_timer, current_ts, powerup_timer)) {
                    current_powers[i].active = false;
                }
            }
        }
    }

    bool check_death(int **map) {
        for (int i = 0; i < 4; i++) {
            if (collision(map, position, m_enemy, directions[i])) {
                return true;
            }
        }
        if (bomb.has_killed(position)) {
            return true;
        }
        return false;

    }

    void player_control(int **map, char key) {
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
        case ' ':
            if (bombs_remaining > 0) {
                bomb.start_bomb(position);
                bombs_remaining--;
            }
            return;
        default:
            return;
        }

        if ((!collision(map, position, m_wall, player_direction) && !collision(map, position, m_brk_wall, player_direction)) || current_powers[0].active) {
            position.x += player_direction.x;
            position.y += player_direction.y;
        }

    }
};

struct Powerup {
public:
    coords position;
    bool used = false;

    void check_catch(Player& player) {
        if (player.position.x == position.x && player.position.y == position.y) {
            used = true;
            player.get_random_powerup();
        }
    }
};

struct Enemy {

public:
    coords position;
    bool alive = true;
    bool is_moving = false;

    clock_t movement_start_ts;
    clock_t interval_start_ts;
    coords sort_direction;
    int sort_range;

    void move_enemy(int **map, coords direction) {
        if (!collision(map, position, m_wall, direction) && !collision(map, position, m_brk_wall, direction)) {
            position.x += direction.x;
            position.y += direction.y;
        }
        else {
            direction = directions[rand() % 4];
            move_enemy(map, direction);
        }
    }

    void check_enemy_movement(int **map) {

        if (!is_moving && timer_check(movement_start_ts, clock(), enemy_timer)) {
            sort_direction = directions[rand() % 4];
            sort_range = (rand() % 3) + 1;
            interval_start_ts = clock();
            is_moving = true;
        }
        if (is_moving && timer_check(interval_start_ts, clock(), enemy_timer)) {
            move_enemy(map, sort_direction);
            interval_start_ts = clock();
            if (sort_range == 0) {
                movement_start_ts = clock();
                is_moving = false;
            }
            else {
                sort_range--;
            }
        }
    }
};

void draw(int **map, int rows, int cols, Player player, HANDLE color) {
    int p_color = player.current_powers[0].active ? 9 : 12;
    int b_color = player.current_powers[1].active ? 14 : 15;

    int i = 0, j = 0;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            switch (map[i][j]) {

            case m_floor: /// m_floor.
                SetConsoleTextAttribute(color, 0);
                cout << " ";
                break;

            case m_wall: /// Solid m_wall.
                SetConsoleTextAttribute(color, 8);
                cout << "w";
                break;

            case m_brk_wall: /// Breakable m_wall.
                SetConsoleTextAttribute(color, 15);
                cout << "w";
                break;

            case m_player: /// Player.
                SetConsoleTextAttribute(color, p_color);
                cout << "p";
                break;

            case m_enemy: /// Enemy.
                SetConsoleTextAttribute(color, 15);
                cout << "e";
                break;

            case m_bomb: /// Bomb!
                SetConsoleTextAttribute(color, b_color);
                cout << "b";
                break;

            case m_powerup: /// Bomb!
                SetConsoleTextAttribute(color, 15);
                cout << "?";
                break;

            case m_plr_and_bb: /// Bomb! & Player.
                SetConsoleTextAttribute(color, 15);
                cout << "2";
                break;

            case m_explosion: /// Kabum!
                SetConsoleTextAttribute(color, 15);
                cout << "k";
                break;

            }

            if (j == cols - 1) {
                cout << "\n";
            }
        }
    }
}

void draw_hud(clock_t start_ts, clock_t end_ts, int enemies_killed, HANDLE out) {
    SetConsoleTextAttribute(out, 15);
    static int s_time, minutes, seconds;
    s_time = interval_to_ms(start_ts, end_ts)/1000;
    seconds = s_time % 60;
    minutes = s_time / 60;
    cout << "TIME - " << minutes << ":";
    if (seconds < 10) cout << "0";
    cout << seconds;
    cout << " SCORE: " << enemies_killed << "\n";
}

void draw_message(string message) {

    cout << "-----------\n";
    cout << "-" << message << "-\n";
    cout << "-----------\n";

}

void read_map(int **&map, int &rows, int &cols, Player &player, Powerup *&powerups, int &map_powerups_count, Enemy *&enemies, int &enemy_count, string map_path, clock_t& time_offset) {
    for (int i = 0; i < rows; i++) {
        delete[]map[i];
    }
    delete[]map;
    delete[]enemies;
    delete[]powerups;

    ifstream map_file;
    map_file.open(map_path);
    map_file >> rows;
    map_file >> cols;
    
    map = new int* [rows];
    for (int i = 0; i < rows; i++) {
        map[i] = new int[cols];
    }

    enemy_count = 0;
    map_powerups_count = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            map_file >> map[i][j];
            if (map[i][j] == m_powerup) {
                map_powerups_count++;
            }
            else if (map[i][j] == m_enemy) {
                enemy_count++;
            }
            else if(map[i][j] == m_player){
                player.position.y = i;
                player.position.x = j;
            }
        }
    }
    enemies = new Enemy[enemy_count];
    powerups = new Powerup[map_powerups_count];
    int enemy_spawn_counter = 0;
    int powerups_spawn_counter = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (map[i][j] == m_powerup) {
                powerups[powerups_spawn_counter].position.y = i;
                powerups[powerups_spawn_counter].position.x = j;
                powerups_spawn_counter++;
            }
            else if (map[i][j] == m_enemy) {
                enemies[enemy_spawn_counter].position.y = i;
                enemies[enemy_spawn_counter].position.x = j;
                enemy_spawn_counter++;
            }
        }
    }

    if (!map_file.eof()) {
        map_file >> time_offset;
    }
    map_file.close();
}

void save_map(int **map, int rows, int cols, clock_t elapsed_time) {
    ofstream my_map;
    my_map.open("C:\\Users\\gabim\\source\\repos\\DigSix\\TrabalhoBomberman\\Bomberman2\\save.txt");
    my_map << rows;
    my_map << ' ';
    my_map << cols;
    my_map << "\n";
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            my_map << map[i][j] << " ";
        }
        my_map << "\n";
    }
    my_map << elapsed_time;
    my_map.close();
}

void reset_game(Enemy *enemies, int enemy_count, Player &player, Powerup *powerups, int** map) {
    player.bomb.exploded = false;
    player.bombs_remaining = max_bombs;
}

void updateMatrix(Enemy *enemies, int enemy_count, Player player, Powerup *powerups, int map_powerups_count, int**map, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (map[i][j] != m_wall && map[i][j] != m_brk_wall) {
                map[i][j] = m_floor;
            }
        }
    }
    map[player.position.y][player.position.x] = m_player;
    if (player.bombs_remaining == 0) {
        if (player.bomb.exploded) {
            for (int i = 0; i < 5; i++) {
                if (map[player.bomb.position.y + directions[i].y][player.bomb.position.x + directions[i].x] != m_wall) {
                    map[player.bomb.position.y + directions[i].y][player.bomb.position.x + directions[i].x] = m_explosion;
                }
            }
        }
        else {
            if (player.position.y == player.bomb.position.y && player.position.x == player.bomb.position.x) {
                map[player.bomb.position.y][player.bomb.position.x] = m_plr_and_bb;
            }
            else {
                map[player.bomb.position.y][player.bomb.position.x] = m_bomb;
            }
        }
    }

    for (int i = 0; i < enemy_count; i++) {
        if (enemies[i].alive) {
            map[enemies[i].position.y][enemies[i].position.x] = m_enemy;
        }
    }
    for (int i = 0; i < map_powerups_count; i++) {
        if (!powerups[i].used) {
            map[powerups[i].position.y][powerups[i].position.x] = m_powerup;
        }
    }
}

void game_loop(Enemy *enemies, int enemy_count, Player player, Powerup* powerups, int map_powerups_count, int **map, int rows, int cols, HANDLE out, COORD coord, clock_t time_offset);

void leave_game(HANDLE out, COORD coord) {
    system("cls");
    SetConsoleTextAttribute(out, 15);
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    cout << "Obrigado por jogar! \n";
    return;
}

void menu_loop(Enemy *enemies, int enemy_count, Player player, Powerup *powerups, int map_powerups_count, int **map, int rows, int cols, HANDLE out, COORD coord) {
    system("cls");
    SetConsoleTextAttribute(out, 15);
    clock_t time_offset = 0;
    static char key;
    static int menu_select = 0;
    static string menu_options[3] = {
        "Novo jogo",
        "Carregar jogo",
        "Sair",
    };

    while (true) {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        for (int i = 0; i < 3; i++) {
            if (menu_select == i) {
                cout << "-> ";
            }
            else {
                cout << "   ";
            }
            cout << menu_options[i];
            if (i == 0) cout << " (Mapa " << map_select+1 << ")";
            cout << "\n";
        }

        if (_kbhit()) {
            key = _getch();
            switch (key) {
            case ' ':
                switch (menu_select) {
                case 0:
                    reset_game(enemies, enemy_count, player, powerups, map);
                    read_map(map, rows, cols, player, powerups, map_powerups_count, enemies, enemy_count, map_options[map_select], time_offset);
                    game_loop(enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols, out, coord, time_offset);
                    return;
                case 1:
                    reset_game(enemies, enemy_count, player, powerups, map);
                    read_map(map, rows, cols, player, powerups, map_powerups_count, enemies, enemy_count, "C:\\Users\\gabim\\source\\repos\\DigSix\\TrabalhoBomberman\\Bomberman2\\save.txt", time_offset);
                    game_loop(enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols, out, coord, time_offset);
                    return;
                case 2:
                    leave_game(out, coord);
                    return;
                }
            case 72: case 'w':
                if (menu_select > 0) menu_select--;
                break;
            case 80: case 's':
                if (menu_select < 2) menu_select++;
                break;
            case 'n':
                if (map_select > 0) map_select--;
                break;
            case 'm':
                if(map_select < 1) map_select++;
                break;
            }

        }
    }
}

void end_game_loop(bool win, int enemies_killed, clock_t game_start_ts, clock_t game_end_ts, Enemy *enemies, int enemy_count, Player player, Powerup* powerups, int map_powerups_count, int **map, int rows, int cols, HANDLE out, COORD coord) {
    system("cls");
    SetConsoleTextAttribute(out, 15);
    clock_t time_offset = 0;
    static char key;
    static int s_time, minutes, seconds, menu_select = 0;
    static string menu_options[2] = {
        "Novo jogo",
        "Voltar para menu"
    };
    s_time = interval_to_ms(game_start_ts, game_end_ts) / 1000;
    seconds = s_time % 60;
    minutes = s_time / 60;

    string end_message = win ? "Voce venceu!" : "Voce perdeu!";
    while (true) {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        cout << end_message << "\n";
        cout << "Inimigos mortos: " << enemies_killed << "\n";
        cout << "Tempo de jogo: " << minutes << ":";
        if (seconds < 10) cout << "0";
        cout << seconds << "\n \n";
        for (int i = 0; i < 2; i++) {
            if (menu_select == i) {
                cout << "-> ";
            }
            else {
                cout << "   ";
            }
            cout << menu_options[i];
            if (i == 0) cout << " (Mapa " << map_select+1 << ")";
            cout << "\n";
        }


        if (_kbhit()) {
            key = _getch();
            switch (key) {
            case ' ':
                switch (menu_select) {
                case 0:
                    reset_game(enemies, enemy_count, player, powerups, map);
                    read_map(map, rows, cols, player, powerups, map_powerups_count, enemies, enemy_count, map_options[map_select], time_offset);
                    game_loop(enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols, out, coord, time_offset);
                    return;
                case 1:
                    menu_loop(enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols, out, coord);
                    return;
                }
            case 72: case 'w':
                if (menu_select > 0) menu_select--;
                break;
            case 80: case 's':
                if (menu_select < 1) menu_select++;
                break;
            case 'n':
                if (map_select > 0) map_select--;
                break;
            case 'm':
                if (map_select < 1) map_select++;
                break;
            }

        }
    }
}

void pause_loop(Enemy *enemies, int enemy_count, Player player, Powerup* powerups, int map_powerups_count, int **map, int rows, int cols, HANDLE out, COORD coord, clock_t start_game_ts, clock_t final_game_ts, int enemies_killed) {
    system("cls");
    SetConsoleTextAttribute(out, 15);
    static char key;
    static int menu_select = 0;
    bool saved = false;

    static string menu_options[3] = {
        "Continuar",
        "Salvar jogo",
        "Voltar para menu",
    };

    while (true) {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        draw_hud(start_game_ts, final_game_ts, enemies_killed, out);

        for (int i = 0; i < 3; i++) {
            if (menu_select == i) {
                cout << "-> ";
            }
            else {
                cout << "   ";
            }
            cout << menu_options[i] << "\n";
        }
        if (saved) cout << "\n O jogo foi salvo.";

        if (_kbhit()) {
            key = _getch();
            switch (key) {
            case ' ':
                switch (menu_select) {
                case 0:
                    game_loop(enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols, out, coord, final_game_ts-start_game_ts);
                    return;
                case 1:
                    save_map(map, rows, cols, final_game_ts - start_game_ts);
                    saved = true;
                    break;
                case 2:
                    menu_loop(enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols, out, coord);
                    return;
                }
                break;
            case 72: case 'w':
                if (menu_select > 0) menu_select--;
                break;
            case 80: case 's':
                if (menu_select < 2) menu_select++;
                break;
            }
        }
    }
}

void game_loop(Enemy *enemies, int enemy_count, Player player, Powerup* powerups, int map_powerups_count, int **map, int rows, int cols, HANDLE out, COORD coord, clock_t time_offset=0) {
    system("cls");
    clock_t start_game_ts = clock()-time_offset, current_ts;
    static char key;
    // Colocar valor inicial para variável de inimigos mortos dinâmica
    int enemies_killed = 0;

    while (!player.check_death(map) && enemies_killed < enemy_count) {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        updateMatrix(enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols);

        current_ts = clock();

        if (_kbhit()) {
            key = _getch();
            if (key == 'p') {
                pause_loop(enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols, out, coord, start_game_ts, current_ts, enemies_killed);
                return;
            }
            player.player_control(map, key);
        }

        player.check_bomb_loop(map);
        player.check_powerups_loop();

        for (int i = 0; i < enemy_count; i++) {
            if (enemies[i].alive) {
                if (player.bomb.has_killed(enemies[i].position)) {
                    enemies[i].alive = false;
                    enemies_killed++;
                }
                else {
                    enemies[i].check_enemy_movement(map);
                }
            }
        }

        for (int i = 0; i < map_powerups_count; i++) {
            if (!powerups[i].used) {
                powerups[i].check_catch(player);
            }
        }

        SetConsoleTextAttribute(out, 15);
        draw_hud(start_game_ts, current_ts, enemies_killed, out);
        draw(map, rows, cols, player, out);

    }
    bool victory = enemies_killed == enemy_count ? true : false;
    end_game_loop(victory, enemies_killed, start_game_ts, clock(), enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols, out, coord);
}

int** initialize_map(int &rows, int &cols) {
    int** new_map;
    new_map = new int* [rows];
    for (int i = 0; i < rows; i++) {
        new_map[i] = new int[cols];
    }
    new_map[0][0] = 1;
    return new_map;
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

    int rows = 1, cols = 1, enemy_count = 0, map_powerups_count = 0;
    int** map = initialize_map(rows, cols);

    Player player; 
    Powerup* powerups;
    powerups = new Powerup[0];
    Enemy *enemies;
    enemies = new Enemy[0];

    menu_loop(enemies, enemy_count, player, powerups, map_powerups_count, map, rows, cols, out, coord);
    return 0;
}
