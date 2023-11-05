#include <string>
#include <iostream>

namespace MakeLowerCase{
std::string Lower(std::string stringinput){
	std::string output = "";
	for(int i = 0; i < stringinput.length(); i++){
			switch(stringinput[i]){
			case 65: {output += "a"; break;}
			case 66: {output += "b"; break;}
			case 67: {output += "c"; break;}
			case 68: {output += "d"; break;}
			case 69: {output += "e"; break;}
			case 70: {output += "f"; break;}
			case 71: {output += "g"; break;}
			case 72: {output += "h"; break;}
			case 73: {output += "i"; break;}
			case 74: {output += "j"; break;}
			case 75: {output += "k"; break;}
			case 76: {output += "l"; break;}
			case 77: {output += "m"; break;}
			case 78: {output += "n"; break;}
			case 79: {output += "o"; break;}
			case 80: {output += "p"; break;}
			case 81: {output += "q"; break;}
			case 82: {output += "r"; break;}
			case 83: {output += "s"; break;}
			case 84: {output += "t"; break;}
			case 85: {output += "u"; break;}
			case 86: {output += "v"; break;}
			case 87: {output += "w"; break;}
			case 88: {output += "x"; break;}
			case 89: {output += "y"; break;}
			case 90: {output += "z"; break;}
			default: {output += ""; break;}
		}
	}
	return output;
}
}
