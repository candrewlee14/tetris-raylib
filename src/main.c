#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifdef DEBUG
#include <dlfcn.h>
#endif
#include "game.h"

typedef void* module_main_func(bool fullscreen, void* data);

#ifndef DEBUG
    extern void* module_main(bool fullscreen, void* data);
#endif

bool fullscreen = false;

int main(int argc, char** argv)
{
    // check varargs if "--fulscreen" is present"
    // if so, set fullscreen to true
    // if not, set fullscreen to false
    if (argc > 1) {
        if (strcmp(argv[1], "--fullscreen") == 0) {
            fullscreen = true;
        }
    }

    
    struct GameState* state = NULL;
    while (true) {
        #ifdef DEBUG
            system("zig build game-reloadable");
            printf("Built game reloadable\n");
            #ifdef LINUX
                void* handle = dlopen("./zig-out/lib/libgame-reloadable.so", RTLD_LAZY | RTLD_GLOBAL);
            #elif MACOS
                void* handle = dlopen("./zig-out/lib/libgame-reloadable.dylib", RTLD_LAZY | RTLD_GLOBAL);
            #endif
            if (handle == NULL) {
                fprintf(stderr, "Failed to load module. (%s)\n", dlerror());
                fprintf(stderr, "Press return to try again.\n");
                getchar();
            }
            module_main_func* main_func = (module_main_func*)dlsym(handle, "module_main");
        #else 
            // this is linked in and will be module_main
            module_main_func* main_func = (module_main_func*)module_main;
        #endif

        if (main_func == NULL) {
            #ifdef DEBUG
            printf("Error: %s\n", dlerror());
            #endif
            return 1;
        }

        if (state != NULL) {
            state = main_func(fullscreen, (void*)state);
        } else {
            state = main_func(fullscreen, NULL);
        }
        if ((*state).playstate == STATE_QUITTING) {
            #ifdef DEBUG
            dlclose(handle);  
            #endif
            break;
        }
        #ifdef DEBUG
        dlclose(handle);  
        #endif
    }
    if (state != NULL) {
        free(state);
    }
    return 0;
}
