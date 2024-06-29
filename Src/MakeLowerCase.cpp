
#include "MakeLowerCase.hpp"

namespace MakeLowerCase {
std::string Lower(std::string inp) {
	for(uint32_t i = 0; i < inp.length(); i++) {
		if(inp[i] < 91) {
			inp[i] += 32;
		}
	}
	return inp;
}
} // namespace MakeLowerCase
