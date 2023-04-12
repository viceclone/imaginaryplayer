# Imaginary Player
The Imaginary Player application is built for educational purposes. The player is a text-based (console) application which streams imaginary music (text) to the console.
# Definitions
## Tracks

Each track is encoded as a .txt file. Each file content both metadata and content.

Example:  

>title I Don't Want to Miss a Thing  
>artist Aerosmith  
>codec mp3  
>duration 500  
>content I could stay awake just to hear you breathin'  

## Playlists

Each playlist is encoded as a .txt file. Each file contain information of the playlist with respect to the structure:  
- First line: playlist's name.  
- Second line: description.  
- From 3rd line, each line contain a path (can be relative path or absolute path) to a track.  

Example:

>Playlist 1  
>This is the first playlist  
>tracks/track1.txt  
>tracks/track2.txt  
>tracks/track3.txt  
>tracks/track4.txt  

## Commands

> Text-based player receives command from keyboard input.  
> - **'H', '?':** *print help*  
> - **'N'     :** *import playlist from file*  
> - **'Z'     :** *play*  
> - **'X'     :** *pause*  
> - **'D'     :** *next track*  
> - **'A'     :** *previous track*  
> - **'S'     :** *shuffle/unshuffle*  
> - **'R'     :** *change repeat mode (none/repeat all/repeat currentsong)*  
> - **'I'     :** *current playlist info*  
> - **'Q'     :** *quit*  
----------------------------------------------------------

# Overall design
The application consists of two threads:
- The first thread receives commands (e.g. play, pause) from keyboard input and sends signal to the second one.
- The second thread streams tracks to ```std::cout```.
# Build project
The project requires C++17 and MSVC 17.4.5 for the Windows build.
```
mkdir build
cd build
cmake ..
cmake --build .
```
# Demo
Link to the demo video: https://youtu.be/H4RiD5uz6fk
