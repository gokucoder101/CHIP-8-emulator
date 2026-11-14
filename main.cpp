#include <array>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include <SDL.h>

using namespace std;

class CHIP_8
{
    public:
        uint8_t memory[4096] = {0};
        uint8_t V[16] = {0};
        uint16_t I = 0;
        uint16_t pc = 0x200; //512
        std::vector<uint16_t> stack;                 // return addresses
        uint8_t delay_timer = 0;
        uint8_t sound_timer = 0;
        array<uint8_t, 64 * 32> display{};      // 64x32 pixels, 0 or 1
        array<bool, 16> keys{}; 

    CHIP_8(){load_font();}
    void load_font()
    {
        static const uint8_t font[80]={
            0xF0,0x90,0x90,0x90,0xF0, // 0
            0x20,0x60,0x20,0x20,0x70, // 1
            0xF0,0x10,0xF0,0x80,0xF0, // 2
            0xF0,0x10,0xF0,0x10,0xF0, // 3
            0x90,0x90,0xF0,0x10,0x10, // 4
            0xF0,0x80,0xF0,0x10,0xF0, // 5
            0xF0,0x80,0xF0,0x90,0xF0, // 6
            0xF0,0x10,0x20,0x40,0x40, // 7
            0xF0,0x90,0xF0,0x90,0xF0, // 8
            0xF0,0x90,0xF0,0x10,0xF0, // 9
            0xF0,0x90,0xF0,0x90,0x90, // A
            0xE0,0x90,0xE0,0x90,0xE0, // B
            0xF0,0x80,0x80,0x80,0xF0, // C
            0xE0,0x90,0x90,0x90,0xE0, // D
            0xF0,0x80,0xF0,0x80,0xF0, // E
            0xF0,0x80,0xF0,0x80,0x80  // F
        };

    for (int i = 0; i < 80; i++)
    {
        memory[0x050 + i] = font[i];
    }
    }

    void load_rom(const string &path)
    {
        ifstream file(path, ios::binary);
        if (!file) throw runtime_error("No such file: " + path);
        vector<char> data((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>( )));
        if (data.size() > 4096 - 0x200) throw runtime_error("Rom is too big!");
        for (int i = 0; i < data.size(); i++)
        {
            memory[0x200 + i] = static_cast<uint8_t>(data[i]);
        }
    }

    void step()
    {
        
        //fetch
        uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
        pc+=2;
        
        //decode
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t y = (opcode & 0x00F0) >> 4;
        uint8_t n = (opcode & 0x000F);
        uint8_t kk = (opcode & 0x00FF);
        uint16_t nnn = (opcode & 0x0FFF);

        //execut

        switch(opcode >> 12)
        {
            case 0x0:
            {
                if (opcode == 0x00E0) display.fill(0);
                else if (opcode == 0x00EE)
                {
                    pc = stack.back();
                    stack.pop_back();
                }
                break;
            }
            case 0x1: pc = nnn;        break;               // 1nnn: jump
            case 0x2: 
            {
                stack.push_back(pc);
                pc = nnn;
                break;
            }
            case 0x3: 
            {
                if (V[x] == kk)
                {
                    pc+=2;
                    
                }
                break;
            }
            case 0x4:
            {
                if (V[x] != kk)
                {
                    pc+=2;
                }
                break;
            }
            case 0x5:
            {
                if(V[x] == V[y])
                {
                    pc += 2;
                }
                break;
            }
            case 0x6: V[x] = kk;       break;               // 6xkk: Vx = kk
            case 0x7: V[x] += kk;      break;               // 7xkk: Vx += kk (wraps automatically)
            case 0x8:
            {
                if (n == 0x0)
                    V[x] = V[y];
                else if (n == 0x1)
                    V[x] |= V[y];
                else if (n == 0x2)
                    V[x] &= V[y];
                else if (n == 0x3)
                    V[x] ^= V[y];
                else if (n == 0x4)
                {
                    int flag;
                    if (V[x] + V[y] > 255)
                    {
                        flag = 1;
                    } 
                    else    
                        flag = 0;
                    V[x] += V[y];
                    V[0xF] = flag;
                }
                else if(n == 0x5)
                {
                    int flag;
                    if (V[x] >= V[y])
                        flag = 1;
                    else   
                        flag = 0;
                    V[x] -= V[y];
                    V[0xF] = flag;
                }
                else if(n == 0x6)
                {
                    int flag = V[x] & 1;
                    V[x] >>= 1;
                    V[0xF] = flag;
                }
                else if(n == 0x7)
                {
                    int flag = 0;
                    if (V[y] >= V[x])
                        flag = 1;
                    V[x] = V[y] - V[x];
                    V[0xF] = flag;
                }
                else if (n == 0xE)
                {
                    int flag = V[x] >> 7;
                    V[x] <<= 1;
                    V[0xF] = flag;
                }
                break;
                
            }
            case 0x9:
            {
                if (V[x] != V[y])
                    pc += 2;
                break;
            }
            case 0xA: I = nnn;         break;               // Annn: I = nnn
            case 0xB: pc = V[0] + nnn;   break;
            case 0xC: 
            {
                int random_num = std::rand() % 256;
                V[x] = random_num & kk;  
                break;              
            }
            case 0xD: 
            {
                V[0xF] = 0;
                int py = V[y] % 32;
                int px = V[x] % 64;

                for (int row = 0; row < n; row++)
                {
                    uint8_t spriteByte = memory[I + row];
                    
                    for (int col = 0; col < 8; col++)
                    {
                        
                        if (spriteByte & 0x80 >> col)
                        {
                            int sx = px + col;
                            int sy = py + row;
                            if (sx >= 64 || sy >= 32) continue;
                            if(display[((sy)*64 + sx)] == 1) V[0xF] = 1;
                            display[((sy)*64 + sx)] ^= 1;
                        }

                        
                    }
                }
            break;
            }
            case 0xE:
            {
                switch(kk){
                case 0x9E:{
                    if (keys[V[x]&0xF] == 1)
                    pc += 2;
                    break;
                }
                case 0xA1:{
                    if(keys[V[x]&0xF] == 0)
                    {
                        pc+=2;
                    }
                    break;
                }
                }
                break;
            }
            case 0xF:
            {
                switch (kk)
                {
                    case 0x0A:
                    {
                        int flag = 0;
                        for (int i = 0; i < 16; i++)
                        {
                            if (keys[i] == 1)
                            {
                                V[x] = i;
                                flag = 1;
                                break;
                            }
                        }
                        if (flag == 0)
                            pc -= 2;
                        break;
                    }
                    case 0x1E:
                    {
                        I += V[x];
                        break;
                    }
                    case 0x55:
                    {
                        for (int i = 0; i <= x; i++)
                        {
                            memory[I + i] = V[i];
                        }
                        break;
                    }
                    case 0x65:
                    {
                        for (int i = 0; i <= x; i++)
                        {
                            V[i] = memory[I + i];
                        }
                        break;
                    }
                    case 0x15:
                    {
                        delay_timer = V[x];
                        break;
                    }
                    case 0x18:
                    {
                        sound_timer = V[x];
                        break;
                    }
                    case 0x07:
                    {
                        V[x] = delay_timer;
                        break;
                    }
                    case 0x29:
                    {
                        I = 0x050 + (V[x] & 0xF)*5;
                        break;
                    }
                    case 0x33:
                    {
                        int temp = V[x];
                        int temp2 = 2;
                        while (temp2 > -1)
                        {
                            memory[I + temp2] = temp % 10;
                            temp2--;
                            temp/=10;
                        }
                        break;
                    }
                }

                break;

            }

            default: break;
        }


    }

    void printDisplay()
    {
        for(int row = 0; row < 32; row++)
        {
            
            for (int col = 0; col < 64; col++)
            {
                if(display[row * 64 + col] == 1)
                    cout << "##";
                else
                    cout << "  ";
            }
            cout << endl;
        }
    }
    void drawDisplay(SDL_Renderer* renderer)
    {
        SDL_SetRenderDrawColor(renderer, 0 ,0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect rect = {10, 10, 10 ,10};
        for(int row = 0; row < 32; row++)
        {
            
            for (int col = 0; col < 64; col++)
            {
                if(display[row * 64 + col] == 1)
                {
                    rect = {col * 10, row * 10, 10, 10};;
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
        SDL_RenderPresent(renderer);
    }
    void tick_timers()
    {
        if (delay_timer > 0)
            delay_timer --;
        if (sound_timer > 0)
            sound_timer --;
    }
};

int keyIndex(SDL_Keycode key)
{
    switch (key)
    {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;
        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;
        default: return -1;
    }
}

int main(int argc, char* argv[])
{
    CHIP_8 chip;
    chip.load_rom("Pong (1 player).ch8");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, 0);
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    
    bool running = true;
    
    while(running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            if (event.type == SDL_KEYDOWN)
            {
                int k = keyIndex(event.key.keysym.sym);
                if (k != -1)
                {
                    chip.keys[k] = 1;
                }
            }
            if (event.type == SDL_KEYUP)
            {
                int k = keyIndex(event.key.keysym.sym);
                if(k != -1)
                {
                    chip.keys[k] = 0;
                }
            }
        }
        for (int i = 0; i < 10; i++)
            chip.step();
        chip.drawDisplay(renderer);
        chip.tick_timers();
        SDL_Delay(16);
    }
    
    chip.printDisplay();
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
    