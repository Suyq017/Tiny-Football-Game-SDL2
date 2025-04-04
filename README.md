# Tiny Football Game

## Mô tả

Dự án **Tiny Football Game** là một trò chơi bóng đá 2D đơn giản, được phát triển sử dụng thư viện **SDL2** với ngôn ngữ lập trình C++. Trò chơi cho phép người chơi điều khiển cầu thủ di chuyển trên sân, tương tác với bóng và tham gia các trận đấu bóng đá.

## Cài đặt

### Bước 1: Cài đặt SDL2

Trước khi bắt đầu, bạn cần cài đặt thư viện SDL2 và các thư viện phụ trợ như SDL2_image, SDL2_mixer và SDL2_ttf. Bạn có thể tham khảo hướng dẫn cài đặt SDL2 cho hệ điều hành của mình trong video hướng dẫn dưới đây:

[Hướng dẫn cài đặt SDL2](https://youtu.be/uv4fda8Z8Tk?si=vH9J8FVxFco3eKH6)

### Bước 2: Biên dịch trò chơi

Sau khi cài đặt SDL2, bạn có thể biên dịch trò chơi bằng cách sử dụng dòng lệnh sau (trên hệ thống Unix/Linux):

```bash
g++ -o game -Llib main.cpp -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
```

- `-Llib`: Chỉ định thư mục chứa các thư viện SDL2.
- `main.cpp`: Tệp mã nguồn chính của trò chơi.
- `-lSDL2`: Liên kết với thư viện SDL2.
- `-lSDL2_image`: Liên kết với thư viện SDL2_image để xử lý hình ảnh.
- `-lSDL2_mixer`: Liên kết với thư viện SDL2_mixer để xử lý âm thanh.
- `-lSDL2_ttf`: Liên kết với thư viện SDL2_ttf để hỗ trợ font chữ.

### Bước 3: Chạy trò chơi

Sau khi biên dịch thành công, bạn có thể chạy trò chơi với lệnh sau:

```bash
./game
```

## Các tính năng

- **Điều khiển cầu thủ:** Người chơi có thể điều khiển cầu thủ chạy tự do, chuyền bóng và dứt điểm.
- **AI đối thủ:** Cài đặt AI cho đội đối thủ với các chiến thuật khác nhau.
- **Hiệu ứng âm thanh và hình ảnh:** Trò chơi sử dụng các hiệu ứng âm thanh và hình ảnh để tạo ra trải nghiệm sống động.
- **Xử lý va chạm:** Tính toán và xử lý va chạm giữa cầu thủ và bóng, đảm bảo gameplay mượt mà.

## Công nghệ sử dụng

- **Ngôn ngữ lập trình:** C++
- **Thư viện:** SDL2, SDL2_image, SDL2_mixer, SDL2_ttf
