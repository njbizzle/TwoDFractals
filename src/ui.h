#ifndef UI_H
#define UI_H

#include <SFML/Graphics.hpp>

#include "range.h"

class Button {
public:
    rangeTwoD boundingBox;
    sf::Text text;
    void(*actionPtr)();

    Button(rangeTwoD boundingBox_, void(*actionPtr_)(), sf::Text text_): boundingBox(boundingBox_), actionPtr(actionPtr_), text(text_){}
}

#endif //UI_H
install_name_tool -change /path/to/sfml/libsfml-graphics.dylib @executable_path/../Frameworks/libsfml-graphics.dylib "$MACOS_DIR/your_executable"
install_name_tool -change /path/to/sfml/libsfml-window.dylib @executable_path/../Frameworks/libsfml-window.dylib "$MACOS_DIR/your_executable"
install_name_tool -change /path/to/sfml/libsfml-system.dylib @executable_path/../Frameworks/libsfml-system.dylib "$MACOS_DIR/your_executable"
