
rm .\sdl2\ -Force -Recurse -ErrorAction SilentlyContinue
rm .\sdl2-mixer\ -Force -Recurse -ErrorAction SilentlyContinue
rm .\sdl2-ttf\ -Force -Recurse -ErrorAction SilentlyContinue
rm .\sdl2-image\ -Force -Recurse -ErrorAction SilentlyContinue

iwr "https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.1/SDL2_ttf-devel-2.20.1-VC.zip" -outfile "sdl2-ttf.zip"
iwr "https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.6.2/SDL2_mixer-devel-2.6.2-VC.zip" -outfile "sdl2-mixer.zip"
iwr "https://github.com/libsdl-org/SDL_image/releases/download/release-2.6.2/SDL2_image-devel-2.6.2-VC.zip" -outfile "sdl2-image.zip"
iwr "https://github.com/libsdl-org/SDL/releases/download/release-2.28.3/SDL2-devel-2.28.3-VC.zip" -outfile "sdl2.zip"

Expand-Archive -Path sdl2.zip -DestinationPath .\temp\
Move-Item -Path .\temp\SDL2* -Destination .\sdl2\
Expand-Archive -Path sdl2-ttf.zip -DestinationPath .\temp\
Move-Item -Path .\temp\SDL2* -Destination .\sdl2-ttf\
Expand-Archive -Path sdl2-mixer.zip -DestinationPath .\temp\
Move-Item -Path .\temp\SDL2* -Destination .\sdl2-mixer\
Expand-Archive -Path sdl2-image.zip -DestinationPath .\temp\
Move-Item -Path .\temp\SDL2* -Destination .\sdl2-image\
rm .\temp\
rm .\sdl2.zip
rm .\sdl2-mixer.zip
rm .\sdl2-ttf.zip
rm .\sdl2-image.zip
