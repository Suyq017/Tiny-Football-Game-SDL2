#include "main.h"

// https://youtu.be/uv4fda8Z8Tk?si=vH9J8FVxFco3eKH6
// g++ -o game -Llib main.cpp -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf

void renderMenu(SDL_Renderer* renderer, TTF_Font* font, const std::string& selected) {
    // Xóa màn hình (màu nền đen)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Màu văn bản
    SDL_Color textColor = { 255, 255, 255, 255 };  // Màu trắng
    SDL_Color selectedColor = { 255, 0, 0, 255 };  // Màu đỏ cho lựa chọn hiện tại

    // Vị trí văn bản
    SDL_Rect playerRect = { 320, 200, 200, 40 };
    SDL_Rect computerRect = { 320, 400, 200, 40 };

    
    // **Chỉ vẽ hình chữ nhật nếu chuột đang hover vào lựa chọn**
    bool isHovered = (selected == "player" || selected == "computer");

    if (isHovered) {
        SDL_SetRenderDrawColor(renderer, selectedColor.r, selectedColor.g, selectedColor.b, selectedColor.a);
        if (selected == "player") {
            SDL_RenderFillRect(renderer, &playerRect);
        } else if (selected == "computer") {
            SDL_RenderFillRect(renderer, &computerRect);
        }
    }

    // Vẽ chữ "Player vs Player"
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Player vs Player", textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, nullptr, &playerRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Vẽ chữ "Player vs Computer"
    surface = TTF_RenderText_Solid(font, "Player vs Computer", textColor);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, nullptr, &computerRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Cập nhật màn hình
    SDL_RenderPresent(renderer);
}

bool handleMenuInput(SDL_Event& event, std::string& selected, bool& quit) {
    if (event.type == SDL_QUIT) {
        quit = true;
    }
    int x, y;
    SDL_GetMouseState(&x, &y); // Lấy tọa độ chuột
    // Xử lý khi rê chuột vào menu
    if (event.type == SDL_MOUSEMOTION) {
        if (x >= 320 && x <= 520) { // Giới hạn ngang của cả hai lựa chọn
            if (y >= 200 && y <= 240) {
                selected = "player";
                running = true;
            } else if (y >= 400 && y <= 440) {
                selected = "computer";
                running = true;
            } else selected = ""; // Không hover vào menu nếu ngoài phạm vi
        } else selected = ""; // Không hover vào menu nếu ngoài phạm vi
    }
    // Xử lý khi click chuột chọn menu
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (selected == "player" || selected == "computer") {
            quit = true; // Thoát menu khi có lựa chọn
        }
        std::cout << "Selected: " << selected << std::endl;
    }
    return quit;
}

void menu(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Event event;
    bool quit = false;

    while (!quit) {
        // Xử lý sự kiện
        while (SDL_PollEvent(&event)) {
            quit = handleMenuInput(event, selected, quit);
        }
        // Vẽ màn hình menu
        renderMenu(renderer, font, selected);
        // Delay để giảm tốc độ vòng lặp (giúp giao diện mượt mà hơn)
        SDL_Delay(100);
    }
}

int generateRandomDirection() {
    // Khởi tạo random device và generator
    std::random_device rd;  // Phát sinh ngẫu nhiên từ thiết bị phần cứng (nếu có)
    std::mt19937 gen(rd()); // Mảng ngẫu nhiên dựa trên random device
    std::uniform_int_distribution<> dis(0, 3);  // Phân phối đều từ 0 đến 3 (gồm cả 0 và 3)

    // Trả về một hướng ngẫu nhiên
    return dis(gen);
}

bool check_collision_player(Player &player1, Player &player2) {
    float dx = player1.x - player2.x;
    float dy = player1.y - player2.y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance <= (player1.radius + player2.radius)) {
        float overlap = (player1.radius + player2.radius) - distance;
        float nx = dx / distance;  // Vector đơn vị theo trục x
        float ny = dy / distance;  // Vector đơn vị theo trục y

        // Tính vận tốc tương đối theo hướng va chạm
        float vx_rel = player1.velocity_x - player2.velocity_x;
        float vy_rel = player1.velocity_y - player2.velocity_y;
        float dot_product = vx_rel * nx + vy_rel * ny; // Vận tốc theo hướng va chạm

        if (dot_product > 0) return false;  // Nếu hai cầu thủ đang di chuyển ra xa nhau, không cần xử lý va chạm

        // Hệ số đàn hồi (coefficient of restitution) có thể chỉnh để kiểm soát mức độ phản hồi
        float e = 0.8f;  // 1.0 = va chạm hoàn toàn đàn hồi, 0.0 = không đàn hồi

        // Tính toán vận tốc mới theo bảo toàn động lượng
        float impulse = (-(1 + e) * dot_product) / 2; // Lực tác động chia đều cho cả 2 cầu thủ
        float push_x = impulse * nx;
        float push_y = impulse * ny;

        player1.velocity_x += push_x;
        player1.velocity_y += push_y;
        player2.velocity_x -= push_x;
        player2.velocity_y -= push_y;

        // Đẩy cầu thủ ra khỏi nhau một chút để tránh dính nhau
        float push_distance = overlap / 2;
        player1.x += nx * push_distance;
        player1.y += ny * push_distance;
        player2.x -= nx * push_distance;
        player2.y -= ny * push_distance;

        return true;
    }
    return false;
}

void updateBallPosition(vector<Player>& red_team, vector<Player>& blue_team, Ball& ball, Mix_Chunk* goalSound) {
    // Áp dụng ma sát liên tục để giảm dần tốc độ
    ball.velocity_x *= friction;
    ball.velocity_y *= friction;

    // Cập nhật vị trí bóng theo vận tốc
    ball.x += ball.velocity_x;
    ball.y += ball.velocity_y;

    // Kiểm tra ghi bàn cho đội xanh
    if (ball.x <= 22 && ball.y >= 295 && ball.y <= 405) {
        // Đội xanh ghi bàn
        blueScore++;
        std::cout << "Green Team scores! Score: " << blueScore << std::endl;
        Mix_PlayChannel(-1, goalSound, 0);
        red_team[0].x = 362;
        red_team[0].y = 345;
        red_team[0].velocity_x = 0;
        red_team[0].velocity_y = 0;
    
        red_team[1].x = 93;
        red_team[1].y = 345;
        red_team[1].velocity_x = 0;
        red_team[1].velocity_y = 0;
    
        blue_team[0].x = 652;
        blue_team[0].y = 345;
        blue_team[0].velocity_x = 0;
        blue_team[0].velocity_y = 0;
    
        blue_team[1].x = 931;
        blue_team[1].y = 345;
        blue_team[1].velocity_x = 0;
        blue_team[1].velocity_y = 0;
    
        ball.x = SCREEN_WIDTH / 2;
        ball.y = SCREEN_HEIGHT / 2;
        ball.velocity_x = 0;
        ball.velocity_y = 0;
    }

    // Kiểm tra ghi bàn cho đội đỏ
    if (ball.x >= 998 && ball.y >= 295 && ball.y <= 405) {
        // Đội đỏ ghi bàn
        redScore++;
        std::cout << "Red Team scores! Score: " << redScore << std::endl;
        Mix_PlayChannel(-1, goalSound, 0);
        red_team[0].x = 362;
        red_team[0].y = 345;
        red_team[0].velocity_x = 0;
        red_team[0].velocity_y = 0;
    
        red_team[1].x = 93;
        red_team[1].y = 345;
        red_team[1].velocity_x = 0;
        red_team[1].velocity_y = 0;
    
        blue_team[0].x = 652;
        blue_team[0].y = 345;
        blue_team[0].velocity_x = 0;
        blue_team[0].velocity_y = 0;
    
        blue_team[1].x = 931;
        blue_team[1].y = 345;
        blue_team[1].velocity_x = 0;
        blue_team[1].velocity_y = 0;
    
        ball.x = SCREEN_WIDTH / 2;
        ball.y = SCREEN_HEIGHT / 2;
        ball.velocity_x = 0;
        ball.velocity_y = 0;
    }
    
     // Kiểm tra va chạm với biên phải (right)
     if (ball.x + ball.radius > SCREEN_WIDTH - 9) {
        ball.x = SCREEN_WIDTH - ball.radius - 9;  // Đặt bóng vào đúng vị trí biên
        ball.velocity_x = -ball.velocity_x; // Đảo chiều vận tốc theo hướng X
        if (fabs(ball.velocity_x) < 0.1f) {
            ball.velocity_x = (ball.velocity_x > 0) ? 2.0f : -2.0f; // Thêm lực đẩy nhỏ nếu vận tốc quá chậm
        }
    }
    
    // Kiểm tra va chạm với biên trái (left)
    else if (ball.x < ball.radius + 5) {
        ball.x = ball.radius + 5; // Đặt bóng vào đúng vị trí biên
        ball.velocity_x = -ball.velocity_x; // Đảo chiều vận tốc theo hướng X
        if (fabs(ball.velocity_x) < 0.1f) {
            ball.velocity_x = (ball.velocity_x > 0) ? 2.0f : -2.0f; // Thêm lực đẩy nhỏ nếu vận tốc quá chậm
        }
    }

    // Kiểm tra va chạm với biên trên (up)
    else if (ball.y < 20) {
        ball.y = 20; // Đặt bóng vào đúng vị trí biên
        ball.velocity_y = -ball.velocity_y; // Đảo chiều vận tốc theo hướng Y
        if (fabs(ball.velocity_y) < 0.1f) {
            ball.velocity_y = (ball.velocity_y > 0) ? 2.0f : -2.0f; // Thêm lực đẩy nhỏ nếu vận tốc quá chậm
        }
    }

    // Kiểm tra va chạm với biên dưới (bottom)
    else if (ball.y + ball.radius  > 690) {
        ball.y = 690 - ball.radius ; // Đặt bóng vào đúng vị trí biên
        ball.velocity_y = -ball.velocity_y; // Đảo chiều vận tốc theo hướng Y
        if (fabs(ball.velocity_y) < 0.1f) {
            ball.velocity_y = (ball.velocity_y > 0) ? 2.0f : -2.0f; // Thêm lực đẩy nhỏ nếu vận tốc quá chậm
        }
    }
}

void handleCollision_ball(Ball& ball, const Player& player) {
    // Tính khoảng cách giữa hai tâm hình tròn
    float dx = ball.x - player.x;
    float dy = ball.y - player.y;
    float distance = sqrt(dx * dx + dy * dy);

    // Kiểm tra va chạm, nếu không có thì không cần xử lý
    if (distance > (ball.radius + player.radius)) return;

    // Tính vector hướng va chạm (từ cầu thủ đến bóng)
    float nx = dx / distance;  // Vector pháp tuyến từ bóng đến cầu thủ
    float ny = dy / distance;

    // Khi bóng va chạm với cầu thủ, luôn khởi tạo vận tốc là 5 và theo hướng va chạm
    float initialSpeed = 12.0f;  // Vận tốc khởi tạo
    ball.velocity_x = nx * initialSpeed;  // Vận tốc theo hướng X
    ball.velocity_y = ny * initialSpeed;  // Vận tốc theo hướng Y

    // Đảm bảo bóng không bị dính vào cầu thủ bằng cách đẩy nó ra ngoài một chút
    float overlap = (ball.radius + player.radius) - distance;
    if (overlap > 0) {
        // Đẩy bóng ra ngoài một chút để tránh bị dính cầu thủ
        ball.x += nx * overlap * 1.5f;  // Đẩy bóng ra ngoài một chút
        ball.y += ny * overlap * 1.5f;
    }

    // Cập nhật vị trí bóng
    ball.x += ball.velocity_x;
    ball.y += ball.velocity_y;
}

void renderBall(SDL_Renderer* renderer, SDL_Texture* ballTexture, const Ball& ball) {
    // Tạo một SDL_Rect để xác định vị trí và kích thước của bóng
    SDL_Rect destRect = { ball.x - ball.radius, ball.y - ball.radius, ball.radius * 2, ball.radius * 2 };

    // Vẽ bóng lên renderer
    SDL_RenderCopy(renderer, ballTexture, nullptr, &destRect);
}

void renderPlayer(SDL_Renderer* renderer, SDL_Texture* playerTexture, const Player& player) {
    // Kích thước mỗi frame (192x160)
    int frameWidth = 192;
    int frameHeight = 160;

    // Xác định phần của sprite sheet cần vẽ
    SDL_Rect srcRect = { frameWidth * player.frame, 0, frameWidth, frameHeight };  // Cắt frame từ sprite sheet
    SDL_Rect destRect = { player.x - player.radius, player.y - player.radius, player.radius * 2, player. radius * 2 };

    // Vẽ texture
    SDL_RenderCopy(renderer, playerTexture, &srcRect, &destRect);
}

void renderWind(SDL_Renderer* renderer, SDL_Texture* windTexture, Wind& wind, float scale) {
    if (!wind.active) return; // Nếu gió không hoạt động, không vẽ
    
    int frameWidth = 85;
    int frameHeight = 33;

    // Tính toán frame dựa trên thời gian trôi qua
    int totalFrames = 4;  // Giả sử sprite sheet có 4 frame gió

    // Cập nhật frame liên tục
    wind.frame = (currentTime / 100) % totalFrames;  // Thay đổi frame mỗi 100ms

    // Cắt frame từ sprite sheet
    SDL_Rect srcRect = { frameWidth * wind.frame, 0, frameWidth, frameHeight };

    // Điều chỉnh vị trí và kích thước của gió
    SDL_Rect destRect = { static_cast<int>(wind.x), static_cast<int>(wind.y), 
                          static_cast<int>(frameWidth * scale), static_cast<int>(frameHeight * scale) };
    
    // Vẽ gió
    SDL_RenderCopy(renderer, windTexture, &srcRect, &destRect);
}

void spawnWind(Wind& wind, const Ball& ball, int screenWidth, int screenHeight, int direction, float windStrength) {
    wind.active = true;
    wind.speed = 5.0f;
    //0 - phải->trái, 1 - trái->phải, 2 - Trên -> dưới, 3 - dưới -> trên
    if (direction == 0 || direction == 1) {  // Gió theo phương ngang
        wind.y = ball.y;  // Gió cùng độ cao với bóng
        if (direction == 1) {
            wind.x = ball.x - 10;  // Gió từ trái qua phải
            wind.dirX = 1;
        } else {
            wind.x = ball.x + 10;  // Gió từ phải qua trái
            wind.dirX = -1;
        }
        wind.dirY = 0;
    } else if (direction == 2 || direction == 3){  // Gió theo phương dọc
        wind.x = ball.x;  // Gió cùng hoành độ với bóng
        if (direction == 2) {
            wind.y = ball.y - 10;  // Gió từ trên xuống
            wind.dirY = 1;
        } else {
            wind.y = ball.y + 10;  // Gió từ dưới lên
            wind.dirY = -1;
        }
        wind.dirX = 0;
    }
}

void drawPlayerHitbox(SDL_Renderer* renderer, int centerX, int centerY, int radius, SDL_Color color) {
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);  // radius * 2

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    if (drawHitbox) {
        for (int w = 0; w < 2 * radius; w++) {
            for (int h = 0; h < 2 * radius; h++) {
                int dx = radius - w;  // Khoảng cách theo chiều ngang
                int dy = radius - h;  // Khoảng cách theo chiều dọc
                if (dx * dx + dy * dy <= radius * radius) {
                    SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
                }
            }
        }
    }
    // Nếu drawHitbox = false, không làm gì cả, không vẽ hình tròn
}

void drawScore(SDL_Renderer* renderer, TTF_Font* font, int redScore, int blueScore) {
    // Chuyển điểm số thành chuỗi văn bản
    std::string scoreText = "Red Team: " + std::to_string(redScore) + "  |  Blue Team: " + std::to_string(blueScore);

    // Tạo texture từ văn bản
    SDL_Color textColor = {255, 255, 255}; // Màu trắng cho văn bản
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Lấy kích thước của văn bản
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface); // Giải phóng surface sau khi tạo texture

    // Đặt vị trí vẽ (x, y) theo ý muốn
    int xPosition = 50;  // Vị trí X của văn bản (ví dụ, 100 pixels từ bên trái)
    int yPosition = 5;   // Vị trí Y của văn bản (ví dụ, 20 pixels từ trên cùng)

    // Tạo rect để vẽ văn bản
    SDL_Rect renderQuad = {xPosition, yPosition, textWidth, textHeight};

    // Vẽ văn bản lên màn hình
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
    SDL_DestroyTexture(textTexture);  // Giải phóng texture
}

void applyWindForce(Ball& ball, Wind& wind_object) {

    //0 - phải->trái, 1 - trái->phải, 2 - Trên -> dưới, 3 - dưới -> trên
    int direction = generateRandomDirection();  

    // Tạo lực gió tại vị trí của bóng
    spawnWind(wind_object, ball, SCREEN_WIDTH, SCREEN_HEIGHT, direction, windStrength);

    // Áp dụng lực gió cho bóng theo hướng đã chọn
    if (direction == 0) {
        // Gió thổi từ trái qua phải (tăng velocity_x)
        ball.velocity_x -= windStrength;
    } 
    else if (direction == 1) {
        // Gió thổi từ phải qua trái (giảm velocity_x)
        ball.velocity_x += windStrength;
    } 
    else if (direction == 2) {
        // Gió thổi từ trên xuống dưới (tăng velocity_y)
        ball.velocity_y += windStrength;
    } 
    else {
        // Gió thổi từ dưới lên trên (giảm velocity_y)
        ball.velocity_y -= windStrength;
    }
    ball.x += ball.velocity_x;
    ball.y += ball.velocity_y;
    cout << "ball.velocity_x: " << ball.velocity_x << " ball.velocity_y: " << ball.velocity_y << endl;
}   

void AIControlBlueTeam(vector<Player>& blueteam, Ball& ball) {
    blue_up = blue_down = blue_left = blue_right = false;

    float goal_x = 22.0f;    // Vị trí khung thành đối phương
    float goal_y = 350.0f;   // Trung tâm khung thành đối phương
    float mid_x = 510.0f;
    float mid_y = 345.0f;

    float signed_distance_ball_x = ball.x - mid_x;
    float signed_distance_ball_y = ball.y - mid_y;

    if (ball.x > 850 && (ball.y < 420 || ball.y > 260)) {  // Nếu bóng gần khung thành đội mình, điều khiển thủ môn 931 345
        blue_current_player = 4;
        for (Player& player : blueteam) {
            if (player.id == 4) {  // Thủ môn
                if (player.x < ball.x) blue_right = true;
                if (player.x > ball.x) blue_left = true;
                if (player.y < ball.y) blue_down = true;
                if (player.y > ball.y) blue_up = true;
            }
        }
    } else {  
        blue_current_player = 3;
        for (Player& player : blueteam) {
            if (player.id == 3) { // Tiền đạo
                float dx = goal_x - ball.x;
                float dy = goal_y - ball.y;
                
                // Nếu bóng ở xa khung thành, di chuyển về bóng
                if (fabs(dx) > 10 || fabs(dy) > 10) {
                    if (player.x < ball.x) blue_right = true;
                    if (player.x > ball.x) blue_left = true;
                    if (player.y < ball.y) blue_down = true;
                    if (player.y > ball.y) blue_up = true;
                }

                // Khi chạm bóng, đẩy bóng về phía khung thành
                if (fabs(player.x - ball.x) < 20 && fabs(player.y - ball.y) < 20) {
                    if (dx < 0) blue_left = true;  // Đẩy về bên trái (về khung thành đối phương)
                    if (dy < 0) blue_up = true;    // Điều chỉnh hướng bóng nếu cần
                    if (dy > 0) blue_down = true;
                }
            }
        }
    }
}


int main() {
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        cerr << "SDL_Init Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Tiny Football", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface* img = IMG_Load("assets/freepik1__adjust__15964.png");
    if (!img) {
        cerr << "Failed to load image: " << IMG_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    // Tính tỷ lệ thu nhỏ Background cho phù hợp với Screen
    float scale_width = static_cast<float>(SCREEN_WIDTH) / img->w;
    float scale_height = static_cast<float>(SCREEN_HEIGHT) / img->h;
    float scale = std::min(scale_width, scale_height);
    int new_width = static_cast<int>(img->w * scale);
    int new_height = static_cast<int>(img->h * scale);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);
    if (!texture) {
        cerr << "Failed to create texture: " << SDL_GetError() << endl;
        SDL_FreeSurface(img);
        SDL_Quit();
        return 1;
    }

    SDL_Surface* ball_image = IMG_Load("assets/SoccerBall.png");
    if (!ball_image) {
        cerr << "Failed to load image: " << IMG_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    SDL_Texture* ball_texture = SDL_CreateTextureFromSurface(renderer, ball_image);
    if (!ball_texture) {
        cerr << "Failed to create texture: " << SDL_GetError() << endl;
        SDL_FreeSurface(ball_image);
        SDL_DestroyTexture(ball_texture);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* redteam_left = IMG_LoadTexture(renderer, "assets/redteam_left.png");
    if (!redteam_left) {
            cerr << "Failed to create texture: " << SDL_GetError() << endl;
            SDL_DestroyTexture(redteam_left);
            SDL_Quit();
            return 1;
        }
    
    SDL_Texture* redteam_right = IMG_LoadTexture(renderer, "assets/redteam_right.png");
    if (!redteam_right) {
            cerr << "Failed to create texture: " << SDL_GetError() << endl;
            SDL_DestroyTexture(redteam_right);
            SDL_Quit();
            return 1;
        }

    SDL_Texture* blueteam_left = IMG_LoadTexture(renderer, "assets/blueteam_left.png");
    if (!blueteam_left) {
            cerr << "Failed to create texture: " << SDL_GetError() << endl;
            SDL_DestroyTexture(blueteam_left);
            SDL_Quit();
            return 1;
        }

    SDL_Texture* blueteam_right = IMG_LoadTexture(renderer, "assets/blueteam_right.png");
    if (!blueteam_right) {
            cerr << "Failed to create texture: " << SDL_GetError() << endl;
            SDL_DestroyTexture(blueteam_right);
            SDL_Quit();
            return 1;
        }
    
    SDL_Texture* wind = IMG_LoadTexture(renderer, "assets/Arcade_-_Asterix_-_Dust_Tornado-removebg-preview.png");
    if (!wind) {
            cerr << "Failed to create texture: " << IMG_GetError() << endl;  // Dùng IMG_GetError() thay vì SDL_GetError()
            SDL_Quit();
            return 1;
        }
        
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
            // Xử lý lỗi và thoát chương trình
    }

    Mix_Music* music = Mix_LoadMUS("assets/football-271520.mp3");
    if (!music) {
        cerr << "Failed to load music: " << Mix_GetError() << endl;
        SDL_FreeSurface(img);
        SDL_DestroyTexture(texture);
        SDL_Quit();
        return 1;
    }

    Mix_Chunk* goalSound = Mix_LoadWAV("assets/99636__tomlija__small-crowd-yelling-yeah.wav");
    if (!music) {
        cerr << "Failed to load music: " << Mix_GetError() << endl;
        SDL_FreeSurface(img);
        SDL_DestroyTexture(texture);
        SDL_Quit();
        return 1;
    }

    if (Mix_PlayMusic(music, -1) == -1) {
        cerr << "Failed to play music: " << Mix_GetError() << endl;
        SDL_FreeSurface(img);
        SDL_DestroyTexture(texture);
        Mix_FreeMusic(music);
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return 1;  // Hoặc xử lý lỗi
    }
    
    TTF_Font* font = TTF_OpenFont("assets/Roboto_Condensed-BoldItalic.ttf", 28); // Đảm bảo bạn có file font.ttf
    if (!font) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return 1;  // Hoặc xử lý lỗi
    }
    
    vector<Player> red_team, blue_team;
        red_team.push_back({1, 362, 345, 
                    0, 0, 0, 0, 0.1f, true, 0.0f, 
                    "Player" + std::to_string(1), {255, 0, 0, 1}, false, 
                    25, 40});
        red_team.push_back({2, 93, 345, 
                    0, 0, 0, 0, 0.1f, true, 0.0f, 
                    "Player" + std::to_string(2), {255, 0, 0, 1}, false, 
                    25, 40});
        for (auto &player : red_team) {
            player.radius = (player.id == 1) ? 25 : 17;  
        }

        blue_team.push_back({3, 652, 345, 
                    0, 0, 0, 0, 0.1f, true, 0.0f, 
                    "Player" + std::to_string(3), {0, 0, 255, 1}, false, 
                    25, 40});

        blue_team.push_back({4, 931, 345, 
                    0, 0, 0, 0, 0.1f, true, 0.0f, 
                    "Player" + std::to_string(4), {0, 0, 255, 1}, false, 
                    25, 40});
        for (auto &player : blue_team) {
            player.radius = (player.id == 3) ? 25 : 17;  
        }

    Ball ball = { 
        510, 345,  // Vị trí (x, y)
        15.0f,
        0, 0,  
        250.0f,  // Tốc độ (speed)
        true  // Bóng đang hoạt động (is_alive)
    };    
    Wind wind_object;
    menu(renderer, font);

while (running) {
    currentTime = SDL_GetTicks();  // Lấy thời gian hiện tại tính bằng miligiây
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) { // Chuyển cầu thủ bằng phím đặc biệt
                if (event.key.keysym.sym == SDLK_TAB) {
            if (red_current_player == 1) {
                red_current_player = 2;  // Chuyển sang cầu thủ 2
            } else if (red_current_player == 2) {
                red_current_player = 1;  // Chuyển sang cầu thủ 1
            }
        } else if (event.key.keysym.sym == SDLK_KP_0) {
            if (blue_current_player == 3) {
                blue_current_player = 4;  // Chuyển sang cầu thủ 4
            } else if (blue_current_player == 4) {
                blue_current_player = 3;  // Chuyển sang cầu thủ 3
            }
        }
            // Điều khiển cầu thủ hiện tại
            if (red_current_player == 1 || red_current_player == 2) {
                if (event.key.keysym.sym == SDLK_w) red_up = true;
                if (event.key.keysym.sym == SDLK_s) red_down = true;
                if (event.key.keysym.sym == SDLK_a) red_left = true;
                if (event.key.keysym.sym == SDLK_d) red_right = true;
            }
            
            if ((blue_current_player == 3 || blue_current_player == 4) && selected != "computer") {
                if (event.key.keysym.sym == SDLK_UP) blue_up = true;
                if (event.key.keysym.sym == SDLK_DOWN) blue_down = true;
                if (event.key.keysym.sym == SDLK_LEFT) blue_left = true;
                if (event.key.keysym.sym == SDLK_RIGHT) blue_right = true;
            }
        } else if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_w) red_up = false;
            if (event.key.keysym.sym == SDLK_s) red_down = false;
            if (event.key.keysym.sym == SDLK_a) red_left = false;
            if (event.key.keysym.sym == SDLK_d) red_right = false;
            if (event.key.keysym.sym == SDLK_UP) blue_up = false;
            if (event.key.keysym.sym == SDLK_DOWN) blue_down = false;
            if (event.key.keysym.sym == SDLK_LEFT) blue_left = false;
            if (event.key.keysym.sym == SDLK_RIGHT) blue_right = false;
        }
    }
    // } else if (event.type == SDL_KEYDOWN) {
    //     if (event.key.keysym.sym == SDLK_j) {  // Phím 'J' di chuyển sang trái
    //         ball_left = true;
    //     }
    //     if (event.key.keysym.sym == SDLK_l) {  // Phím 'L' di chuyển sang phải
    //         ball_right = true;
    //     }
    //     if (event.key.keysym.sym == SDLK_i) {  // Phím 'I' di chuyển lên trên
    //         ball_up = true;
    //     }
    //     if (event.key.keysym.sym == SDLK_k) {  // Phím 'K' di chuyển xuống dưới
    //         ball_down = true;
    //     }
    // }
    // // Xử lý sự kiện khi phím được thả ra
    // else if (event.type == SDL_KEYUP) {
    //     if (event.key.keysym.sym == SDLK_j) {  // Phím 'J' dừng di chuyển trái
    //         ball_left = false;
    //     }
    //     if (event.key.keysym.sym == SDLK_l) {  // Phím 'L' dừng di chuyển phải
    //         ball_right = false;
    //     }
    //     if (event.key.keysym.sym == SDLK_i) {  // Phím 'I' dừng di chuyển lên
    //         ball_up = false;
    //     }
    //     if (event.key.keysym.sym == SDLK_k) {  // Phím 'K' dừng di chuyển xuống
    //         ball_down = false;
    //     }
    // }
    // // Cập nhật vị trí bóng theo các phím được nhấn
    // float speed = 5.0f;  // Tốc độ di chuyển của bóng
    // if (ball_left) {
    //     ball.x -= speed;
    // }
    // if (ball_right) {
    //     ball.x += speed;
    // }
    // if (ball_up) {
    //     ball.y -= speed;
    // }
    // if (ball_down) {
    //     ball.y += speed;
    // }
    // }

    if (selected == "computer") AIControlBlueTeam(blue_team, ball);

    for (auto &player1 : red_team) {
        if (player1.id == red_current_player) {
            if (red_up) player1.velocity_y -= acceleration;
            if (red_down) player1.velocity_y += acceleration;
            if (red_left) {
                player1.velocity_x -= acceleration;
                player1.facingRight = false;
            }
            if (red_right) {
                player1.velocity_x += acceleration;
                player1.facingRight = true;
            }

            player1.velocity_x = std::min(std::max(player1.velocity_x, -max_speed), max_speed);
            player1.velocity_y = std::min(std::max(player1.velocity_y, -max_speed), max_speed);
        }
    }
    for (auto &player1 : red_team) {
            //Ma sát
            player1.velocity_x *= friction;
            player1.velocity_y *= friction;

            player1.x += player1.velocity_x;
            player1.y += player1.velocity_y;

            // Kiểm tra và xử lí va chạm 
            for (auto &player2 : red_team) {
                if (&player1 != &player2 && check_collision_player(player1, player2)) {
                }
            }

            for (auto &player2 : blue_team) {
                if (check_collision_player(player1, player2)) {
                }
            }

            handleCollision_ball(ball,player1);
                // Cập nhật vị trí bóng theo vận tốc
            updateBallPosition(red_team, blue_team, ball, goalSound);


            // Kiểm tra biên giới
            if (player1.radius == 17){
            if (player1.x < 22) player1.x = 22; //left
            if (player1.x + player1.radius > 117) player1.x = 117 - player1.radius;  //right
            if (player1.y < 237) player1.y = 237; //up
            if (player1.y + player1.radius * 2 > 490) player1.y = (490) - player1.radius * 2; //bottom
            } 
            else if (player1.radius == 25){
                if (player1.x < 30) player1.x = 30; //left
                if (player1.x + player1.radius > SCREEN_WIDTH -9) player1.x = SCREEN_WIDTH - player1.radius -9;  //right
                if (player1.y < 30) player1.y = 30; //up
                if (player1.y + player1.radius * 2 > SCREEN_HEIGHT+20) player1.y = (SCREEN_HEIGHT+20) - player1.radius * 2; //bottom
            }
    }
    for (auto &player1 : blue_team) {
        if (player1.id == blue_current_player) {
            if (blue_up) player1.velocity_y -= acceleration;
            if (blue_down) player1.velocity_y += acceleration;
            if (blue_left) {
                player1.velocity_x -=acceleration;
                player1.facingRight = false;
            }
            if (blue_right) {
                player1.velocity_x += acceleration;
                player1.facingRight = true;
            }

            player1.velocity_x = std::min(std::max(player1.velocity_x, -max_speed_bot), max_speed_bot);
            player1.velocity_y = std::min(std::max(player1.velocity_y, -max_speed_bot), max_speed_bot);
        }
    }
    for (auto &player1 : blue_team) {
            //Ma sát
            player1.velocity_x *= friction;
            player1.velocity_y *= friction;

            player1.x += player1.velocity_x;
            player1.y += player1.velocity_y;

            // Kiểm tra và xử lí va chạm 
            for (auto &player2 : blue_team) {
                if (&player1 != &player2 && check_collision_player(player1, player2)) {
                }
            }

            for (auto &player2 : red_team) {
                if (check_collision_player(player1, player2)) {
                }
            }

            handleCollision_ball(ball,player1);
                // Cập nhật vị trí bóng theo vận tốc
            updateBallPosition(red_team, blue_team, ball, goalSound);
            // Kiểm tra biên giới
            if (player1.radius == 17){
            if (player1.x + player1.radius < SCREEN_WIDTH -87) player1.x = SCREEN_WIDTH - player1.radius -87;  //left
            if (player1.x + player1.radius > SCREEN_WIDTH -9) player1.x = SCREEN_WIDTH - player1.radius -9;  //right
            if (player1.y < 237) player1.y = 237; //up
            if (player1.y + player1.radius * 2 > 490) player1.y = (490) - player1.radius * 2; //bottom
            } 
            else if (player1.radius == 25){
                if (player1.x < 30) player1.x = 30; //left
                if (player1.x + player1.radius > SCREEN_WIDTH -9) player1.x = SCREEN_WIDTH - player1.radius -9;  //right
                if (player1.y < 30) player1.y = 30; //up
                if (player1.y + player1.radius * 2 > SCREEN_HEIGHT+20) player1.y = (SCREEN_HEIGHT+20) - player1.radius * 2; //bottom
            }
    }

    for (auto &player1 : red_team) {player1.frame = (player1.frame + 1) % 4;}
    for (auto &player1 : blue_team) {player1.frame = (player1.frame + 1) % 4;}

    if (currentTime - lastWindTime >= windInterval) {
        // Áp dụng lực gió
        applyWindForce(ball, wind_object);
        // renderWind(renderer, wind, ball.x, ball.y, ball.radius*2, ball.radius*2);

        // Cập nhật thời gian lần áp dụng lực gió
        lastWindTime = currentTime;
    }

    // Vẽ lại các đối tượng trên màn hình
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Rect dst_rect_background = {0, 0, new_width, new_height};
    SDL_RenderCopy(renderer, texture, NULL, &dst_rect_background);

    renderBall(renderer, ball_texture, ball);
    
    wind_object.update(); // Cập nhật vị trí gió
    renderWind(renderer, wind, wind_object, 1.0f); // Vẽ gió


    for (auto player : red_team) {
        if (player.facingRight) {
            renderPlayer(renderer, redteam_right, player);
        } else {
            renderPlayer(renderer, redteam_left, player);
        }
    }

    for (auto player : blue_team) {
        if (player.facingRight) {
            renderPlayer(renderer, blueteam_right, player);
        } else {
            renderPlayer(renderer, blueteam_left, player);
        }
    }

    for (const auto &player : red_team) {
        drawPlayerHitbox(renderer, static_cast<int>(player.x), static_cast<int>(player.y), player.radius, player.color);
    }

    for (const auto &player : blue_team) {
        drawPlayerHitbox(renderer, static_cast<int>(player.x), static_cast<int>(player.y), player.radius, player.color);
    }
    drawScore(renderer, font, redScore, blueScore);
    SDL_RenderPresent(renderer);
    SDL_Delay(16); 
}

SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
SDL_Quit();
}