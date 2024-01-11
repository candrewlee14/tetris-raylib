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
        system("tcc -shared -o game.so game.c -lraylib");
        printf("Built game.so\n");

        void* handle = dlopen("./game.so", RTLD_LAZY);
        if (handle == NULL) {
            fprintf(stderr, "Failed to load module. (%s)\n", dlerror());
            fprintf(stderr, "Press return to try again.\n");
            getchar();
        }

        module_main_func* main_func = (module_main_func*)dlsym(handle, "module_main");
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
