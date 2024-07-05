## Description 
a cli app to make creation of game-ready Textures and Atlas' easier
## Requirements 
png++ - mand
## Credits 
"Maber" for the "amber glow" pallet used in testing https://lospec.com/palette-list/amber-glow-5
"Polyducks" for the "japanese woodblock" pallet used in testing https://lospec.com/palette-list/japanese-woodblock
## Notes
# TODO
* [ ] change packer to test for unused lines and columns and trimm them
* [ ] rename confusing variable names to clearer ones
* [ ] change code to have a unified style
* [x] move to string in glyph
* [x] fix bug where creating an atlas always inores the last fed image
* [x] have atlas gen trim unused vertical space
* [x] remove glyphcount redundant integer
* [x] add option to take multiple images and create an atlas with packing data
* [x] create "unit tests" for features
* [x] add option to output to folder
* [x] add option to paletise fed images
* [x] change to be a cli program so it can be used in scripts
* [x] get base shifting program working