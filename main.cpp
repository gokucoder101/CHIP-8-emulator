#include <array>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
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
                if (opcode == 0x00E0) display.fill(0);
                break;
            case 0x1: pc = nnn;        break;               // 1nnn: jump
            case 0x6: V[x] = kk;       break;               // 6xkk: Vx = kk
            case 0x7: V[x] += kk;      break;               // 7xkk: Vx += kk (wraps automatically)
            case 0xA: I = nnn;         break;               // Annn: I = nnn
            case 0xD: 
            {
                int v1 = V[y] % 32;
                int v2 = V[x] % 64;
                display[((v1)*64 + v2)] = 1;
                break;
            }
            default: break;
        }


    }
};


int main()
{
    CHIP_8 chip;
    chip.load_rom("2-ibm-logo.ch8");
    for (int i = 0; i < 4; i++)

    while(true)
    {
        chip.step();
    }
}
