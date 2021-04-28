#include "PCH.hpp"
#include <Emulator.hpp>
#ifdef __SWITCH__
#include <unistd.h>
#include <switch.h>
#include <dirent.h>
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
        

    emulator.SetInput(input, buttons);
}
#ifdef __SWITCH__
int getInd(char* curFile, int curIndex) {
	DIR* dir;
	struct dirent* ent;

	if (curIndex < 0)
		curIndex = 0;

	dir = opendir("sdmc:/roms/");//Open current-working-directory.
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
			sprintf(curFile, "sdmc:/roms/%s", ent->d_name);
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

    std::string romPath = "sdmc:/roms/zelda.gb";            // PASSED
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
        
        { romPath = "roms/zelda.gb";   
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

    // Create window
    spWindow = std::unique_ptr<SDL_Window, SDLWindowDeleter>(
        SDL_CreateWindow(
            "GameLad",
            0,
            0,
           windowWidth,// windowWidth * windowScale, // Original = 160
           windowHeight,// windowHeight * windowScale, // Original = 144
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
    romfsExit();
    SDL_Quit();

    return 0;
}
