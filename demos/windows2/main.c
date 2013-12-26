#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"

#ifndef SDL_GPU_USE_SDL2
// This demo doesn't work for SDL 1.2 because of the missing windowing features in that version.
int main(int argc, char* argv[])
{
    GPU_LogError("Sorry, this demo requires SDL 2.\n");
    return 0;
}

#else

typedef struct Sprite
{
    GPU_Image* image;
    float x, y;
    float velx, vely;
} Sprite;

typedef struct Group
{
    GPU_Target* target;
    Sprite sprite;
} Group;

#define screen_w 300
#define screen_h 300

#define sprite_w 100
#define sprite_h 100

Group create_first_group()
{
    Group g;
    Uint32 windowID = GPU_GetCurrentRenderer()->current_target->windowID;
    SDL_Log("New windowID: %u\n", windowID);
    
    g.target = GPU_GetCurrentRenderer()->current_target;
    
    SDL_Surface* surface = SDL_LoadBMP("data/test.bmp");
    
    g.sprite.image = GPU_CopyImageFromSurface(surface);
    g.sprite.x = rand()%screen_w;
    g.sprite.y = rand()%screen_h;
    g.sprite.velx = 10 + rand()%screen_w/10;
    g.sprite.vely = 10 + rand()%screen_h/10;
    
    SDL_FreeSurface(surface);
    
    return g;
}

Group create_group()
{
    Group g;
    SDL_Window* window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_OPENGL);
    Uint32 windowID = SDL_GetWindowID(window);
    SDL_Log("New windowID: %u\n", windowID);
    
    g.target = GPU_CreateTargetFromWindow(windowID);
    
    SDL_Surface* surface = SDL_LoadBMP("data/test.bmp");
    
    g.sprite.image = GPU_CopyImageFromSurface(surface);
    g.sprite.x = rand()%screen_w;
    g.sprite.y = rand()%screen_h;
    g.sprite.velx = 10 + rand()%screen_w/10;
    g.sprite.vely = 10 + rand()%screen_h/10;
    
    SDL_FreeSurface(surface);
    
    return g;
}

void destroy_group(Group* groups, int i)
{
    GPU_FreeImage(groups[i].sprite.image);
    SDL_DestroyWindow(SDL_GetWindowFromID(groups[i].target->windowID));
    GPU_FreeTarget(groups[i].target);
    groups[i].target = NULL;
}

int main(int argc, char* argv[])
{
	printRenderers();
	
    GPU_Target* screen = GPU_Init(screen_w, screen_h, 0);
    if(screen == NULL)
        return -1;
    
	printCurrentRenderer();
	
    int max_groups = 30;
    Group groups[max_groups];
    memset(groups, 0, sizeof(Group)*max_groups);
	
	int num_groups = 0;
	groups[num_groups] = create_first_group();
	num_groups++;
	
	int i = 0;
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
    Uint8 done = 0;
	SDL_Event event;
	while(!done)
	{
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				done = 1;
			else if(event.type == SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_ESCAPE)
					done = 1;
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
                    for(i = 0; i < max_groups; i++)
                    {
                        if(groups[i].target == NULL)
                        {
                            groups[i] = create_group();
                            num_groups++;
                            SDL_Log("num_groups: %d\n", num_groups);
                            break;
                        }
                    }
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(num_groups > 0)
                    {
						
                        for(i = max_groups-1; i >= 0; i--)
                        {
                            if(groups[i].target != NULL)
                            {
                                destroy_group(groups, i);
                                
                                num_groups--;
                                SDL_Log("num_groups: %d\n", num_groups);
                                break;
                            }
                        }
                        
                        if(num_groups == 0)
                            done = 1;
                    }
				}
			}
			else if(event.type == SDL_WINDOWEVENT)
            {
                if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    Uint8 closed = 0;
                    for(i = 0; i < max_groups; i++)
                    {
                        if(groups[i].target != NULL && groups[i].target->windowID == event.window.windowID)
                        {
                            destroy_group(groups, i);
                            
                            closed = 1;
                            num_groups--;
                            SDL_Log("num_groups: %d\n", num_groups);
                            break;
                        }
                    }
                    
                    // The last window was closed, then.
                    if(!closed || num_groups == 0)
                        done = 1;
                }
            }
		}
		
		for(i = 0; i < max_groups; i++)
		{
		    if(groups[i].target == NULL)
                continue;
            
			groups[i].sprite.x += groups[i].sprite.velx*dt;
			groups[i].sprite.y += groups[i].sprite.vely*dt;
			if(groups[i].sprite.x < 0)
			{
				groups[i].sprite.x = 0;
				groups[i].sprite.velx = -groups[i].sprite.velx;
			}
			else if(groups[i].sprite.x > screen_w)
			{
				groups[i].sprite.x = screen_w;
				groups[i].sprite.velx = -groups[i].sprite.velx;
			}
			
			if(groups[i].sprite.y < 0)
			{
				groups[i].sprite.y = 0;
				groups[i].sprite.vely = -groups[i].sprite.vely;
			}
			else if(groups[i].sprite.y > screen_h)
			{
				groups[i].sprite.y = screen_h;
				groups[i].sprite.vely = -groups[i].sprite.vely;
			}
		}
		
		for(i = 0; i < max_groups; i++)
		{
		    if(groups[i].target == NULL)
                continue;
            
		    GPU_Clear(groups[i].target);
		    
		    GPU_Blit(groups[i].sprite.image, NULL, groups[i].target, groups[i].sprite.x, groups[i].sprite.y);
		    
		    GPU_Flip(groups[i].target);
		}
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
    
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
    for(i = 0; i < max_groups; i++)
    {
        if(groups[i].target == NULL)
            continue;
        
        destroy_group(groups, i);
    }
    
    
    GPU_Quit();
    
    return 0;
}

#endif
