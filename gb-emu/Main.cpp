#include "PCH.hpp"
#include <Emulator.hpp>
#ifdef __SWITCH__
#include <unistd.h>
#include <switch.h>
#include <dirent.h>
#include <iostream>

#endif 
// 60 FPS or 16.67ms
const double TimePerFrame = 1.0 / 60.0;

// The number of CPU cycles per frame
const unsigned int CyclesPerFrame = 70224;

struct SDLWindowDeleter
{
    void operator()(SDL_Window* window)
    {
        if (window != nullptr)
        {
            SDL_DestroyWindow(window);
        }
    }
};

struct SDLRendererDeleter
{
    void operator()(SDL_Renderer* renderer)
    {
        if (renderer != nullptr)
        {
            SDL_DestroyRenderer(renderer);
        }
    }
};

struct SDLTextureDeleter
{
    void operator()(SDL_Texture* texture)
    {
        if (texture != nullptr)
        {
            SDL_DestroyTexture(texture);
        }
    }
};
  int windowWidth = 1280;
    int windowHeight = 720;

void Render(SDL_Renderer* pRenderer, SDL_Texture* pTexture, Emulator& emulator)
{
    // Clear window
    SDL_SetRenderDrawColor(pRenderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(pRenderer);

    byte* pPixels;
    int pitch = 0;
    SDL_LockTexture(pTexture, nullptr, (void**)&pPixels, &pitch);

    // Render Game
    byte* pData = emulator.GetCurrentFrame();
    memcpy(pPixels, pData, 160 * 144 * 4);

    SDL_UnlockTexture(pTexture);
    SDL_Rect dstrect;
  
dstrect.w = 160*5;
dstrect.h = 144*5;
  dstrect.x = (windowWidth - dstrect.w)/2;
dstrect.y = 0;
    SDL_RenderCopy(pRenderer, pTexture, nullptr, &dstrect);

    // Update window
    SDL_RenderPresent(pRenderer);
}

// TODO: refactor this
std::unique_ptr<SDL_Renderer, SDLRendererDeleter> spRenderer;
std::unique_ptr<SDL_Texture, SDLTextureDeleter> spTexture;
Emulator emulator;

// The emulator will call this whenever we hit VBlank
void VSyncCallback()
{
    Render(spRenderer.get(), spTexture.get(), emulator);
}
PadState pad;
bool closegame = false;
void ProcessInput(Emulator& emulator)
{
    SDL_PumpEvents();
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    byte input = JOYPAD_NONE;
    byte buttons = JOYPAD_NONE;

    if(keys[SDL_SCANCODE_W])
    {
        input |= JOYPAD_INPUT_UP;
    }

    if(keys[SDL_SCANCODE_A])
    {
        input |= JOYPAD_INPUT_LEFT;
    }

    if(keys[SDL_SCANCODE_S])
    {
        input |= JOYPAD_INPUT_DOWN;
    }

    if(keys[SDL_SCANCODE_D])
    {
        input |= JOYPAD_INPUT_RIGHT;
    }

    if(keys[SDL_SCANCODE_K])
    {
        buttons |= JOYPAD_BUTTONS_A;
    }

    if(keys[SDL_SCANCODE_L])
    {
        buttons |= JOYPAD_BUTTONS_B;
    }

    if(keys[SDL_SCANCODE_N])
    {
        buttons |= JOYPAD_BUTTONS_START;
    }

    if(keys[SDL_SCANCODE_M])
    {
        buttons |= JOYPAD_BUTTONS_SELECT;
    }



        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        // padGetButtons returns the set of buttons that are currently pressed
        u64 kHeld = padGetButtons(&pad);

        // padGetButtonsUp returns the set of buttons that have been
        // newly released in this frame compared to the previous one
        u64 kUp = padGetButtonsUp(&pad);

        if (kHeld & HidNpadButton_Plus)
           {
buttons |= JOYPAD_BUTTONS_START;
           }
        if (kHeld & HidNpadButton_Minus)
           {
buttons |= JOYPAD_BUTTONS_SELECT;
           }
        if (kHeld & HidNpadButton_Left)
           {
 input |= JOYPAD_INPUT_LEFT;
           }
        if (kHeld & HidNpadButton_Up)
           {
               input |= JOYPAD_INPUT_UP;

           }
        if (kHeld & HidNpadButton_Right)
           {
input |= JOYPAD_INPUT_RIGHT;
           }
        if (kHeld & HidNpadButton_Down)
           {
input |= JOYPAD_INPUT_DOWN;
           }
        if (kHeld & HidNpadButton_A)
           {
buttons |= JOYPAD_BUTTONS_A;
           }
        if (kHeld & HidNpadButton_B)
           {
	buttons |= JOYPAD_BUTTONS_B;
           }
        if (kHeld & HidNpadButton_StickR && kHeld & HidNpadButton_StickL)
           {
	closegame = true;
           }
        

    emulator.SetInput(input, buttons);
}
#ifdef __SWITCH__
int getInd(char* curFile, int curIndex) {
	DIR* dir;
	struct dirent* ent;

	if (curIndex < 0)
		curIndex = 0;

	dir = opendir("sdmc:/gbroms/");//Open current-working-directory.
	if (dir == NULL)
	{
		sprintf(curFile, "Failed to open dir!");
		return curIndex;
	}
	else
	{
		int i;
		for (i = 0; i <= curIndex; i++) {
			ent = readdir(dir);
		}
		if (ent)
			sprintf(curFile, "sdmc:/gbroms/%s", ent->d_name);
		else
			curIndex--;
		closedir(dir);
	}

	return curIndex;
}
#endif




int main(int argc, char** argv)
{
    romfsInit();
    socketInitializeDefault();
    nxlinkStdio();

    struct stat st = { 0 };

	if (stat("sdmc:/gbroms", &st) == -1) {
		mkdir("sdmc:/gbroms", 0777);
	}
	
    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    
    padInitializeDefault(&pad);

  int curIndex = 0;
	char curFile[255];
	curIndex = getInd(curFile, curIndex);
   /* if(argc > 1)
    {
        windowScale = atoi(argv[1]);
    }
*/
    std::string bootROM;
    //std::string bootROM = "res/games/dmg_bios.bin";
    //std::string bootROM = "res/games/gbc_bios.bin";

    std::string romPath = "";            // PASSED
        //std::string romPath = "res/tests/01-special.gb";            // PASSED
        //std::string romPath = "res/tests/02-interrupts.gb";         // PASSED
        //std::string romPath = "res/tests/03-op sp,hl.gb";           // PASSED
        //std::string romPath = "res/tests/04-op r,imm.gb";           // PASSED
        //std::string romPath = "res/tests/05-op rp.gb";              // PASSED
        //std::string romPath = "res/tests/06-ld r,r.gb";             // PASSED
        //std::string romPath = "res/tests/07-jr,jp,call,ret,rst.gb"; // PASSED
        //std::string romPath = "res/tests/08-misc instrs.gb";        // PASSED
        //std::string romPath = "res/tests/09-op r,r.gb";             // PASSED
        //std::string romPath = "res/tests/10-bit ops.gb";            // PASSED
        //std::string romPath = "res/tests/11-op a,(hl).gb";          // PASSED

    //std::string romPath = "res/tests/instr_timing.gb";            // PASSED

    //std::string romPath = "res/tests/mem_timing.gb";            // FAILED
        //std::string romPath = "res/tests/01-read_timing.gb";        // FAILED
        //std::string romPath = "res/tests/02-write_timing.gb";       // FAILED
        //std::string romPath = "res/tests/03-modify_timing.gb";      // FAILED

    //std::string romPath = "res/tests/oam_bug.gb";            // FAILED

    //std::string romPath = "res/games/Pokemon - Blue Version.gb";
    //std::string romPath = "res/games/Tetris (World).gb";
    //std::string romPath = "res/games/Super Mario Land (World).gb";
    //std::string romPath = "res/games/Tamagotchi.gb";
    //std::string romPath = "res/games/Battletoads.gb";
    //std::string romPath = "res/games/Tetris.gb";
    //std::string romPath = "res/games/Zelda.gb";
    //std::string romPath = "res/games/plantboy.gb";
    //std::string romPath = "res/games/Metroid.gb";
    //std::string romPath = "res/games/Castlevania.gb";

    // CGB Only
    //std::string romPath = "res/games/Lemmings.gbc";   // Requires MBC5
    //std::string romPath = "res/games/Mario2.gbc";   // Requires MBC5
    if(argc > 2)
    {
        romPath = argv[2];
    }

    bool isRunning = true;
    std::unique_ptr<SDL_Window, SDLWindowDeleter> spWindow;
//selectrom
  /*  consoleInit(NULL);

while(appletMainLoop())
    {consoleClear();
        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);
printf("\x1b[16;0HSelect the gb rom from your Roms folder with Left Button and Right Button:");
        	printf("\x1b[18;0H%s", curFile);
        // padGetButtonsDown returns the set of buttons that have been newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Left)
        
        {
             curIndex--;
			curIndex = getInd(curFile, curIndex);
			}
        
        if (kDown & HidNpadButton_Right)
        
        {
            curIndex++;
			curIndex = getInd(curFile, curIndex);
		}
        
        if (kDown & HidNpadButton_A)
        
        {   
            break;}
       
        
        consoleUpdate(NULL);
    }

    consoleExit(NULL);*/

    SDL_Event event;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        Logger::LogError("SDL could not initialize! SDL error: '%s'", SDL_GetError());
        return false;
    }
   //Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        return false;    
    }

    // Create window
    spWindow = std::unique_ptr<SDL_Window, SDLWindowDeleter>(
        SDL_CreateWindow(
            "GameLad",
            0,
            0,
           windowWidth,
           windowHeight,
            0));
    if (spWindow == nullptr)
    {
        Logger::LogError("Window could not be created! SDL error: '%s'", SDL_GetError());
        return false;
    }

    // Create renderer
    spRenderer = std::unique_ptr<SDL_Renderer, SDLRendererDeleter>(
        SDL_CreateRenderer(spWindow.get(), 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    if (spRenderer == nullptr)
    {
        Logger::LogError("Renderer could not be created! SDL error: '%s'", SDL_GetError());
        return false;
    }

//test injection
bool quit = false;
 SDL_Surface* Loading_Surf;
  SDL_Texture* Background_Tx;
  SDL_Texture* BlueShapes;

  /* Rectangles for drawing which will specify source (inside the texture)
  and target (on the screen) for rendering our textures. */
  SDL_Rect SrcR;
  SDL_Rect DestR;

  
	while (!quit && appletMainLoop())
	{

		 // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        // padGetButtons returns the set of buttons that are currently pressed
        u64 kHeld = padGetButtons(&pad);

        // padGetButtonsUp returns the set of buttons that have been
        // newly released in this frame compared to the previous one
        u64 kUp = padGetButtonsUp(&pad);

		 if (kDown & HidNpadButton_Left)
           {
curIndex--;
			curIndex = getInd(curFile, curIndex);
				std::cout << curFile << std::endl;
           }
           if (kHeld & HidNpadButton_Right)
           {
curIndex++;
			curIndex = getInd(curFile, curIndex);
			std::cout << curFile << std::endl;
           }
        if (kHeld & HidNpadButton_A)
           {
                romPath = curFile;
quit = true;
            
           }


		


// Clear window
    SDL_SetRenderDrawColor(spRenderer.get(), 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(spRenderer.get());

   
  

 

TTF_Font* Sans = TTF_OpenFont("romfs:/lazy.ttf", 30); //this opens a font style and sets a size
TTF_Font* Sans2 = TTF_OpenFont("romfs:/lazy.ttf", 30); //this opens a font style and sets a size

SDL_Color White = {255, 255, 255};  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color

SDL_Surface* surfaceMessage = TTF_RenderText_Blended(Sans, "Select your GBRom from your gbroms Folder in SD. L3 + R3 for Exit with SRAM.", White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

SDL_Texture* Message = SDL_CreateTextureFromSurface(spRenderer.get(), surfaceMessage); //now you can convert it into a texture

SDL_Rect Message_rect; //create a rect
Message_rect.x = 10;  //controls the rect's x coordinate 
Message_rect.y = 360; // controls the rect's y coordinte
Message_rect.w = surfaceMessage->w; // controls the width of the rect
Message_rect.h = surfaceMessage->h; // controls the height of the rect
  
  SDL_Surface* surfaceMessage2 = TTF_RenderText_Blended(Sans2, curFile, White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

SDL_Texture* Message2 = SDL_CreateTextureFromSurface(spRenderer.get(), surfaceMessage2); //now you can convert it into a texture

SDL_Rect Message2_rect; //create a rect
Message2_rect.x = 10;  //controls the rect's x coordinate 
Message2_rect.y = 600; // controls the rect's y coordinte
Message2_rect.w = surfaceMessage2->w; // controls the width of the rect
Message2_rect.h = surfaceMessage2->h; // controls the height of the rect
SDL_FreeSurface(surfaceMessage);
SDL_FreeSurface(surfaceMessage2);
    SDL_RenderCopy(spRenderer.get(), Message, nullptr, &Message_rect);
SDL_RenderCopy(spRenderer.get(), Message2, nullptr, &Message2_rect);

    // Update window
    SDL_RenderPresent(spRenderer.get());
		}
//test injection



    spTexture = std::unique_ptr<SDL_Texture, SDLTextureDeleter>(
        SDL_CreateTexture(spRenderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144));

    if (emulator.Initialize(bootROM.empty() ? nullptr : bootROM.data(), romPath.data()))
    {
        emulator.SetVSyncCallback(&VSyncCallback);

        unsigned int cycles = 0;
        Uint64 frameStart = SDL_GetPerformanceCounter();
        while (isRunning)
        {
            // Poll for window input
            while (SDL_PollEvent(&event) != 0)
            {
                if (event.type == SDL_QUIT)
                {
                    isRunning = false;
                    emulator.SetVSyncCallback(nullptr);
                }
            }
            if (closegame == true)
                {
                    isRunning = false;
                    emulator.SetVSyncCallback(nullptr);
                }

            if (!isRunning)
            {
                // Exit early if the app is closing
                continue;
            }

            ProcessInput(emulator);
            while (cycles < CyclesPerFrame)
            {
                cycles += emulator.Step();
            }

            cycles -= CyclesPerFrame;

            Uint64 frameEnd = SDL_GetPerformanceCounter();
            // Loop until we use up the rest of our frame time
            while (true)
            {
                frameEnd = SDL_GetPerformanceCounter();
                double frameElapsedInSec = (double)(frameEnd - frameStart) / SDL_GetPerformanceFrequency();

                // Break out once we use up our time per frame
                if (frameElapsedInSec >= TimePerFrame)
                {
                    break;
                }
            }

            frameStart = frameEnd;
        }
    }

    emulator.Stop();

    spTexture.reset();
    spRenderer.reset();
    spWindow.reset();
    socketExit();
    romfsExit();
    SDL_Quit();

    return 0;
}
