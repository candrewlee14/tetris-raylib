<p align="center">
  <img src="https://github.com/candrewlee14/tetris-raylib/assets/32912555/aa5a2007-718d-46d2-8b6c-023236e18f74"/>
</p> 

# Tetris-Raylib


I made for this for [Coding Challenges](https://codingchallenges.fyi/)'s Tetris challenge.

I wanted to try hot reloading in C through recompiling & reloading the core game logic as a shared library.
Being a big Zig fan, I decided to use Zig's build system to enable doing this in debug mode, but compile it all statically in release modes.
If you hit the 'R' key in-game in a debug build, the game will reload but the state will be retained.

My final goal was getting this to run on the Steam Deck, which in the end worked wonderfully (see [tweet](https://x.com/c_andrew_lee/status/1748146829504692384?s=20))!

## Features
- Working Tetris game
- Supports Linux & Macos
- Keyboard & Mouse + Controller Support
- Music & Sound Effects
- Windowed & Fullscreen Modes

## Setup

- You'll need Zig, which you can get [here](https://ziglang.org/download/). I'm currently using the latest nightly: `0.12.0-dev.2139+e025ad7b4`. This includes a C compiler with `zig cc`!
- You will also need the Raylib development library dependencies. See the _Working on {YOUR OS}_ sections of Raylib's [wiki](https://github.com/raysan5/raylib/wiki).

Now you're ready to go!

To run a debug build, run:
```bash
zig build run
```

To run a release build, run:
```bash
zig build -Doptimize=ReleaseSafe run
```

You can run in fullscreen mode with the `--fullscreen` flag.
```
zig build run -- --fullscreen
```

![image](https://github.com/candrewlee14/tetris-raylib/assets/32912555/7657e5df-1859-4a0e-8f92-46fc7d482844)

> This image was made for the purposes of a colorful screenshot, ignore all the gaps :)
