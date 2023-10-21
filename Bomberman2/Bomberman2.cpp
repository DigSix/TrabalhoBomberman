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

struct Player {

public:
    coords position;
    coords player_direction;
    int bombs_remaining = max_bombs;
    Bomb bomb;

    void check_bomb_loop(int **map) {
        if (bombs_remaining == 0) {
            if (!bomb.exploded && timer_check(bomb.bomb_start_ts, clock(), bomb_timer)) {
                bomb.destroy(map);
            }
            else if (bomb.exploded && timer_check(bomb.explosion_start_ts, clock(), explosion_timer)) {
                bomb.clear_explosion();
                bombs_remaining++;
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

        if (!collision(map, position, m_wall, player_direction) && !collision(map, position, m_brk_wall, player_direction)) {
            position.x += player_direction.x;
            position.y += player_direction.y;
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

void draw(int **map, int rows, int cols, HANDLE color) {
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
                SetConsoleTextAttribute(color, 12);
                cout << "p";
                break;

            case m_enemy: /// Enemy.
                SetConsoleTextAttribute(color, 15);
                cout << "e";
                break;

            case m_bomb: /// Bomb!
                SetConsoleTextAttribute(color, 15);
                cout << "b";
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

int** read_map(int **map, int &rows, int &cols, Player &player, Enemy enemies[enemy_count]) {
    for (int i = 0; i < rows; i++) {
        delete[]map[i];
    }
    delete[]map;
    ifstream map_file;
    map_file.open("C:\\Users\\gabim\\source\\repos\\DigSix\\TrabalhoBomberman\\Bomberman2\\map1.txt");
    map_file >> rows;
    map_file >> cols;
    
    int **new_map = new int* [rows];
    for (int i = 0; i < rows; i++) {
        new_map[i] = new int[cols];
    }

    int enemy_spawn_counter = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            map_file >> new_map[i][j];
            if (new_map[i][j] == m_enemy) {
                enemies[enemy_spawn_counter].position.y = i;
                enemies[enemy_spawn_counter].position.x = j;
                enemy_spawn_counter++;
            }
            else if(new_map[i][j] == m_player){
                player.position.y = i;
                player.position.x = j;
            }
        }
    }
    map_file.close();
    return new_map;
}

void save_map(int **map, int rows, int cols) {
    ofstream my_map;
    my_map.open("C:\\Users\\gabim\\source\\repos\\DigSix\\TrabalhoBomberman\\Bomberman2\\save.txt");
    my_map << rows;
    my_map << ' ';
    my_map << cols;
    my_map << "\n";
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            my_map << map[i][j];
            if (j < cols - 1) {
                my_map << ' ';
            }
            if (j == cols - 1) {
                my_map << "\n";
            }
        }
    }
    my_map.close();
}

void reset_game(Enemy enemies[enemy_count], Player &player, int** map) {
    player.bomb.exploded = false;
    player.bombs_remaining = max_bombs;
}

void updateMatrix(Enemy enemies[enemy_count], Player player, int**map, int rows, int cols) {
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
}

void game_loop(Enemy enemies[enemy_count], Player player, int **map, int rows, int cols, HANDLE out, COORD coord, clock_t time_offset);

void leave_game(HANDLE out, COORD coord) {
    system("cls");
    SetConsoleTextAttribute(out, 15);
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    cout << "Obrigado por jogar! \n";
    return;
}

void menu_loop(Enemy enemies[enemy_count], Player player, int **map, int rows, int cols, HANDLE out, COORD coord) {
    system("cls");
    SetConsoleTextAttribute(out, 15);
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
            cout << menu_options[i] << "\n";
        }

        if (_kbhit()) {
            key = _getch();
            switch (key) {
            case ' ':
                switch (menu_select) {
                case 0:
                    reset_game(enemies, player, map);
                    map = read_map(map, rows, cols, player, enemies);
                    game_loop(enemies, player, map, rows, cols, out, coord, 0);
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
            }
        }
    }
}

void end_game_loop(bool win, int enemies_killed, clock_t game_start_ts, clock_t game_end_ts, Enemy enemies[enemy_count], Player player, int **map, int rows, int cols, HANDLE out, COORD coord) {
    system("cls");
    SetConsoleTextAttribute(out, 15);
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
            cout << menu_options[i] << "\n";
        }


        if (_kbhit()) {
            key = _getch();
            switch (key) {
            case ' ':
                switch (menu_select) {
                case 0:
                    reset_game(enemies, player, map);
                    map = read_map(map, rows, cols, player, enemies);
                    game_loop(enemies, player, map, rows, cols, out, coord, 0);
                    return;
                case 1:
                    menu_loop(enemies, player, map, rows, cols, out, coord);
                    return;
                }
            case 72: case 'w':
                if (menu_select > 0) menu_select--;
                break;
            case 80: case 's':
                if (menu_select < 1) menu_select++;
                break;
            }

        }
    }
}

void pause_loop(Enemy enemies[enemy_count], Player player, int **map, int rows, int cols, HANDLE out, COORD coord, clock_t start_game_ts, clock_t final_game_ts, int enemies_killed) {
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
                    game_loop(enemies, player, map, rows, cols, out, coord, final_game_ts-start_game_ts);
                    return;
                case 1:
                    save_map(map, rows, cols);
                    saved = true;
                    break;
                case 2:
                    menu_loop(enemies, player, map, rows, cols, out, coord);
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

void game_loop(Enemy enemies[enemy_count], Player player, int **map, int rows, int cols, HANDLE out, COORD coord, clock_t time_offset=0) {
    system("cls");
    clock_t start_game_ts = clock()-time_offset, current_ts;
    static char key;
    // Colocar valor inicial para variável de inimigos mortos dinâmica
    int enemies_killed = 0;

    while (!player.check_death(map) && enemies_killed < enemy_count) {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
        updateMatrix(enemies, player, map, rows, cols);

        current_ts = clock();

        if (_kbhit()) {
            key = _getch();
            if (key == 'p') {
                pause_loop(enemies, player, map, rows, cols, out, coord, start_game_ts, current_ts, enemies_killed);
                return;
            }
            player.player_control(map, key);
        }

        player.check_bomb_loop(map);

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

        SetConsoleTextAttribute(out, 15);
        draw_hud(start_game_ts, current_ts, enemies_killed, out);
        draw(map, rows, cols, out);

    }
    bool victory = enemies_killed == enemy_count ? true : false;
    end_game_loop(victory, enemies_killed, start_game_ts, clock(), enemies, player, map, rows, cols, out, coord);
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

    int rows = 1;
    int cols = 1;
    int** map;
    map = new int* [rows];
    for (int i = 0; i < rows; i++) {
        map[i] = new int[cols];
    }
    map[0][0] = 1;

    Player player; 
    Enemy enemies[enemy_count];

    menu_loop(enemies, player, map, rows, cols, out, coord);
    return 0;
}
