# Adblock
Working with WebKitGtk extensions is pretty difficult normally, so I've tried to make this process less tedious, but you'll still have to do a relatively large process (It's not really big, but in comparison with another web browsers it's long)

# Depencencies
- Cargo (To install `adblock-rust-server`)

# Building

1. Install the server:
```
cargo install adblock-rust-server
```

2. Clone and install the adblock:
```
git clone https://github.com/dudik/blockit
cd blockit
sudo make install
```
3. Create a folder named `extensions` in the greending-ng folder <br>
Note: Must be next to the `conf` folder and the `greending-ng` executable

4. Link the adblock to the Greending-ng extensions directory
```
sudo ln -s /usr/local/lib/blockit.so /path/to/greeding-ng/extensions/directory/blockit.so
```
Or just copy the library
```
cp blockit.so /path/to/greeding-ng/extensions/directory/blockit.so
```

Now the tricky part begins

5. Open `~/.config/ars/urls` with a text editor and append this to its contents:
```
https://easylist.to/easylist/easylist.txt
https://easylist.to/easylist/easyprivacy.txt
https://easylist.to/easylist/fanboy-social.txt
https://secure.fanboy.co.nz/fanboy-annoyance.txt
```
6. Open a new terminal and execute `adblock-rust-server`<br>
Do **not** close it. For some reasons I'm yet to understand, Greending-ng doesn't open it correctly in my machine. Try not opening a terminal and see if it works on yours.

# Disclaimer

I've found issues with this adblock: If the page is loaded too fast, it will not block ads at first. This is specially annoying with YouTube, as YouTube loads first the ads and you have to wait until Blockit has finished blocking them. Otherwise, it works pretty well.

# Mentions
Thanks to <a href=https://github.com/dudik/>dudik</a> for this adblock.
