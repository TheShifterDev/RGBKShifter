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
to test the program outside of vscode/vscodium cat a test fileinto the built binary, for example
```
./Built/rgbkshifter.bin $(cat ./Tests/Shift.test)
```
and debuging should be done via
```
gdb --args ./Built/rgbkshifter.bin $(cat ./Tests/Shift.test)
```
mass testing can be done via the 'unit test' script
```
./Autotest.sh
```
## TODO
* get handrolled png reading done
* get handrolled png writing done
* test handrolled png code tested
* look into creating "sequences" for gifs/animations/variants/bumpmaps
* make cross platform
## DONE
* test way of naming groups
* add way of naming groups
* prevent creation of output files if no input files were read
* fix freetype ttf reading outputing kneecaped glyphs
* fix crashes on reading stimpacs containing font data
* create a simple unit test suite to run all test files

# FAQ
## Q: what is RGBKShifter
It was originaly a program for pushing image colours to the extremes so that
they could be used in colour replacement shaders for a failed unity project
I dabbled with but over time I thought it could be good c++ practice for
potential jobs and maybe a tool for others wanting a simmilar applicaiton
eventualy changing into an example project for stoma image pack (stimpac)
image file format usage

## Q: why a bespoke format and not a series of pngs
A series of pngs would be cumbersome to read and push into gpu memory
indevidualy, also gpu texture slots are limited so having all textures of
a type be in a singular pretiled image is better overall and what most games
have anyway.

## Q: why did you handroll png reading
I started running into issuess with glibc when using png++ suddenly
(while working on ttf reading) and had various frustrations with libpng,
its docs are garbage and required me looking at the spec for 2 different projects
for clues on how functions were supposed to be used and it does nothing
to actualy help the user to read png files, just changed the problem to 
a different type of problem with additional confusion.
