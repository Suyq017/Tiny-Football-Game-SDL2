#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include "inc/SDL_image.h"
#include "inc/SDL_mixer.h"
#include "inc/SDL_ttf.h"
#include <math.h>
#include <cmath>  
#include <random>
#undef main
using namespace std;

string selected = "";  // Lựa chọn ban đầu
bool running;  // Biến kiểm soát vòng lặp chính

SDL_Event event;
Uint32 currentTime;  // Thời gian hiện tại
Uint32 lastWindTime = 0;  // Thời gian đã trôi qua kể từ lần áp dụng gió cuối cùng
const Uint32 windInterval = 15000;  // Thời gian giữa các lần gió, tính bằng mili giây (ví dụ: 1000 ms = 1 giây)
float windStrength = 10.5f;  

bool drawHitbox = 0;  // Biến điều khiển việc vẽ hitbox, có thể bật/tắt trong game
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 693;

float friction =0.98; // Hệ số ma sát
float push_factor = 2.5f;  // Lực đẩy
float reflectionFactor = 1.5f; // Hệ số phản xạ (tăng tốc độ khi va chạm với biên)
const float acceleration = 0.4f;
const float max_speed = 10.0f;
const float max_speed_bot = 5.0f;

int redScore = 0; // Điểm đội đỏ
int blueScore = 0; // Điểm đội xanh
bool red_up = false, red_down = false, red_left = false, red_right = false;
bool blue_up = false, blue_down = false, blue_left = false, blue_right = false;
int red_current_player = 1; 
int blue_current_player = 3; 
bool ball_left = false;
bool ball_right = false;
bool ball_up = false;
bool ball_down = false;

struct Player {
    unsigned int id;
    float x, y;
    float velocity_x, velocity_y;
    float acceleration_x, acceleration_y;
    float score;
    std::string name;
    SDL_Color color;  
    int width, height;
    int radius;
    int frame =0;
    bool facingRight = true;
    Player(unsigned int id, float x, float y, float velocity_x, float velocity_y, 
           float acceleration_x, float acceleration_y, float friction, 
           bool is_alive, float score, std::string name, SDL_Color color, 
           bool is_goalkeeper, int width, int height)
        : id(id), x(x), y(y), velocity_x(velocity_x), velocity_y(velocity_y), 
          acceleration_x(acceleration_x), acceleration_y(acceleration_y), score(score), name(name), 
          color(color), width(width), height(height) {}
};

struct Team {
    std::string name;
    std::vector<Player> players;     // List of players in the team
};

struct Ball {
    unsigned int x, y;         // Vị trí
    float radius;              // Bán kính
    float velocity_x, velocity_y; // Vận tốc
    float speed;               // Tốc độ
    bool is_alive;             // Trạng thái
    SDL_Color ballColor = {255, 255, 255, 255}; // Màu trắng
};

struct Wind {
    float x, y;       // Vị trí hiện tại của gió
    float dirX, dirY; // Hướng di chuyển (-1, 0, 1)
    float speed;      // Tốc độ di chuyển
    bool active;      // Trạng thái gió (đang thổi hay không)
    int frame = 0;    // Frame hiện tại của gió
    void update() {
        if (active) {
                x += dirX * speed;
                y += dirY * speed;
                    
            // Nếu gió đến điểm cuối, dừng lại
            if ((dirX > 0 && x >= 800) || (dirX < 0 && x <= 0) ||
                (dirY > 0 && y >= 600) || (dirY < 0 && y <= 0)) {
                active = false; // Gió biến mất khi đến giới hạn
            }
        }
    }
};
