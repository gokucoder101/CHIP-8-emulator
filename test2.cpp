#include <SDL.h>

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("zuckerberg", SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 640, 320, 0);
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,0);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    SDL_Rect rect = {100, 50, 10, 10};

    SDL_RenderFillRect(renderer, &rect);
    

    SDL_RenderPresent(renderer);

    
    SDL_Delay(3000);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
