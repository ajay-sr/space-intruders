#include "PixelText.hpp"

#include <SDL.h>
#include <array>

namespace {
// Five-bit rows keep the lettering crisp without external font assets.
using Glyph = std::array<unsigned char, 7>;

Glyph glyph(char character) {
    switch (character) {
    case 'G': return {14, 17, 16, 23, 17, 17, 14};
    case 'O': return {14, 17, 17, 17, 17, 17, 14};
    case 'm': return {0, 0, 26, 21, 21, 21, 21};
    case 'S': return {14, 17, 16, 14, 1, 17, 14};
    case 'I': return {31, 4, 4, 4, 4, 4, 31};
    case 'L': return {16, 16, 16, 16, 16, 16, 31};
    case '0': return {14, 17, 19, 21, 25, 17, 14};
    case '1': return {4, 12, 4, 4, 4, 4, 14};
    case '2': return {14, 17, 1, 2, 4, 8, 31};
    case '3': return {30, 1, 1, 14, 1, 1, 30};
    case '4': return {2, 6, 10, 18, 31, 2, 2};
    case '5': return {31, 16, 16, 30, 1, 1, 30};
    case '6': return {14, 16, 16, 30, 17, 17, 14};
    case '7': return {31, 1, 2, 4, 8, 8, 8};
    case '8': return {14, 17, 17, 14, 17, 17, 14};
    case '9': return {14, 17, 17, 15, 1, 1, 14};
    case 'l': return {12, 4, 4, 4, 4, 4, 14};
    case 'v': return {0, 0, 17, 17, 17, 10, 4};
    case 'P': return {30, 17, 17, 30, 16, 16, 16};
    case 'a': return {0, 0, 14, 1, 15, 17, 15};
    case 'c': return {0, 0, 14, 17, 16, 17, 14};
    case 'd': return {1, 1, 15, 17, 17, 17, 15};
    case 'e': return {0, 0, 14, 17, 31, 16, 14};
    case 'k': return {16, 16, 18, 20, 24, 20, 18};
    case 'n': return {0, 0, 30, 17, 17, 17, 17};
    case 'o': return {0, 0, 14, 17, 17, 17, 14};
    case 'p': return {0, 0, 30, 17, 30, 16, 16};
    case 'r': return {0, 0, 22, 25, 16, 16, 16};
    case 's': return {0, 0, 15, 16, 14, 1, 30};
    case 't': return {8, 8, 28, 8, 8, 9, 6};
    case 'u': return {0, 0, 17, 17, 17, 19, 13};
    case 'y': return {0, 0, 17, 17, 15, 1, 14};
    default: return {};
    }
}

} // namespace

void drawCenteredText(SDL_Renderer* renderer, std::string_view text,
                      int width, int y, int scale) {
    const int textWidth = (static_cast<int>(text.size()) * 6 - 1) * scale;
    int x = (width - textWidth) / 2;
    for (char character : text) {
        const Glyph rows = glyph(character);
        for (int row = 0; row < 7; ++row) {
            for (int column = 0; column < 5; ++column) {
                if (rows[row] & (1 << (4 - column))) {
                    SDL_Rect pixel{x + column * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
        x += 6 * scale;
    }
}
