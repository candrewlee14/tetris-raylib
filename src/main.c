#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "game.h"

typedef void* module_main_func(void* data);

int main(void)
{
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
            printf("Error: %s\n", dlerror());
            return 1;
        }

        if (state != NULL) {
            state = main_func((void*)state);
        } else {
            state = main_func(NULL);
        }
        if ((*state).playstate == STATE_QUITTING) {
            dlclose(handle);  
            break;
        }
        dlclose(handle);  
    }
    if (state != NULL) {
        free(state);
    }
    return 0;
}
