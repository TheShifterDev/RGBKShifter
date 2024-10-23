# Description 
* A cli app to make creation of game-ready Textures and Atlas' easier.
* Made for me but expanded so that it has features others would like.
* Requires StomaImagePack from github.com/TheShifterDev/StomaImagePack
* While this is a project without any specific licence, credit and atribution would be appreciated if you use this for something cool \<3.
# Requirements 
* StomaImagePack
* png++
# Features
* Paletises images to the colours of an input file "-p/--paletise"
* Shifts all an images colours to extremes "-s/--shift" 
* Outputs texture atlas'/sprite sheets "-a/--atlas"
* Outputs texture glyphs as indevidual images "-c/--cutup"
* Outputs as a specified format "-f/--format"
# Credits 
* "Maber" for the "amber glow" pallet used in testing https://lospec.com/palette-list/amber-glow-5
* "Polyducks" for the "japanese woodblock" pallet used in testing https://lospec.com/palette-list/japanese-woodblock
* "be5invis" and other contributors for the font used in testing https://github.com/be5invis/Iosevka/releases
# Notes
## TODO
* add freetype ttf reading
* prevent creation of stimpac's if no files were fed
* look into using gd to support more formats
* look into creating "sequences" for gifs/animations/variants/bumpmaps
* create a way to use wildcards '*' in file input
* make cross platform