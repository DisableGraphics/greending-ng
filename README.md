# greending-ng
The web browser that protects you from onlookers, rewritten in C++

# Can you guess what's happening on the other tabs?

![Greending-ng](https://user-images.githubusercontent.com/48135147/163360614-1e6345bb-1481-407d-a938-37d9ee336866.png)

Most surely not. Therefore, nobody else does!

# Features
- Now rewritten in C++, which means Greending-ng is even faster than its predecessor (Which already was)
- Extensions! Now you can block ads <a href=adblock.md>if you follow this tutorial</a>
- New look: Rectangular buttons and Client Side Decorations.
- WebKit based. This alternative (To Blink and Gecko) browser engine is incredibly fast and lightweight (Runs on my 10 year old potato pretty nicely). 
- Saves tabs when closed.
- Hideously small. The executable weights approximately 250KiB.

# Installation
**Dependencies:**
- GtkMM 3 <br>Arch: `sudo pacman -S gtkmm3`<br>Ubuntu: `sudo apt install libgtkmm-3.0-dev`
- WebKitGtk <br>Arch: `sudo pacman -S webkit2gtk`<br>Ubuntu: `sudo apt install libwebkit2gtk-4.0-dev`

**Building:**
```
git clone https://github.com/DisableGraphics/greending-ng
cd greending-ng

make
cd build
./greending-ng
```
