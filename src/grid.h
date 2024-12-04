#ifndef GRID_H
#define GRID_H
#include <Eigen/Dense>
#include <SFML/Graphics.hpp>

#include "range.h"

float GRID_INTENSITY = 0.2;
float AXIS_INTENSITY = 0.3;
float BOARDER_INTENSITY = 0.5;

class GridAxis
{
public:
    int exponent;
    float spacing;
    int count;
    float offset;

    GridAxis(){}

    GridAxis(range displayRange, int maxLines)
    {
        exponent = ceil(log(displayRange.size/maxLines) / log(10));
        spacing = pow(10, exponent); // space between lines in pixels
        offset = ceil(displayRange.min / spacing) * spacing;
        count = ceil((displayRange.max - offset) / spacing);

    }

    GridAxis(range displayRange, int exponent_, bool this_arg_is_here_so_the_signature_of_the_constructor_is_different)
    {
        exponent = exponent_;
        spacing = pow(10, exponent);
        offset = ceil(displayRange.min / spacing) * spacing;
        count = ceil((displayRange.max - offset) / spacing);
    }
};

struct GridInfo
{
public:
    int maxLines;
    int vertCount;
    bool subGrids;

    GridAxis xGrid;
    GridAxis yGrid;

    GridAxis xSubGrid;
    GridAxis ySubGrid;

    float gridIntensity;
    float subgridIntensity;
};

// must run before anything else to setup grids
GridInfo setupGrid(rangeTwoD view, int maxLines, bool subGrids)
{
    GridInfo info{};

    info.maxLines = maxLines;
    info.subGrids = subGrids;

    info.xGrid = GridAxis(view.x, maxLines);
    info.yGrid = GridAxis(view.y, maxLines);

    info.vertCount = 2 * (info.xGrid.count + info.yGrid.count);

    if (subGrids)
    {
        info.xSubGrid = GridAxis(view.x, info.xGrid.exponent - 1, false);
        info.ySubGrid = GridAxis(view.y, info.xGrid.exponent - 1, false);
        info.vertCount += 2 * (info.xSubGrid.count + info.ySubGrid.count);
    }
    info.vertCount += 4; // +4 to fix malloc error and allow for one extra line on each axis
    return info;
}

void fillLines(sf::VertexArray& gridArray, int& vertIndex, sf::Color color, GridAxis grid, range perpAxis, Eigen::Matrix3f &viewToPixel, bool flipDiagonal)
{
    float xy = grid.offset;
    for (int i = 0; i < grid.count; i++)
    {
        if (!flipDiagonal)
        {

            gridArray[vertIndex].position = transformPoint(viewToPixel, sf::Vector2f(xy, perpAxis.min));
            gridArray[vertIndex + 1].position = transformPoint(viewToPixel, sf::Vector2f(xy, perpAxis.max));
        }
        else
        {
            gridArray[vertIndex].position = transformPoint(viewToPixel, sf::Vector2f(perpAxis.min, xy));
            gridArray[vertIndex + 1].position = transformPoint(viewToPixel, sf::Vector2f(perpAxis.max, xy));
        }

        gridArray[vertIndex].color = color;
        gridArray[vertIndex + 1].color = color;

        if (xy == 0)
        {
            sf::Color axisColor = sf::Color(
                255*AXIS_INTENSITY,
                255*AXIS_INTENSITY,
                255*AXIS_INTENSITY
            );
            gridArray[vertIndex].color = axisColor;
            gridArray[vertIndex + 1].color = axisColor;
        }

        xy += grid.spacing;
        vertIndex += 2;
    }
}

void populateGridVerticies(sf::VertexArray& gridArray, GridInfo& info, rangeTwoD view, Eigen::Matrix3f &viewToPixel)
{
    int vertIndex = 0;

    int gridIntensity = 255 * GRID_INTENSITY;
    int subGridIntensity = gridIntensity; // would only change for debugging

    info.gridIntensity = ((view.x.size / (info.maxLines * info.xGrid.spacing)) - 0.1f) * 10.0f/9.0f;
    info.subgridIntensity = 1 - info.gridIntensity;


    sf::Color xColor = sf::Color(gridIntensity, gridIntensity, gridIntensity, 255 * info.gridIntensity);
    sf::Color yColor = sf::Color(gridIntensity, gridIntensity, gridIntensity, 255 * info.gridIntensity);
    sf::Color xSubColor = sf::Color(subGridIntensity, subGridIntensity, subGridIntensity, 255 * info.subgridIntensity);
    sf::Color ySubColor = sf::Color(subGridIntensity, subGridIntensity, subGridIntensity, 255 * info.subgridIntensity);

    if (info.subGrids)
    {
        fillLines(gridArray, vertIndex, xSubColor, info.xSubGrid, view.y, viewToPixel, false);
        fillLines(gridArray, vertIndex, ySubColor, info.ySubGrid, view.x, viewToPixel, true);
    }

    fillLines(gridArray, vertIndex, xColor, info.xGrid, view.y, viewToPixel, false);
    fillLines(gridArray, vertIndex, yColor, info.yGrid, view.x, viewToPixel, true);
}

void populateBoarderVerticies(sf::VertexArray& gridArray, int offset, rangeTwoD pixels)
{
    bool a,b;
    for (int i = 0; i < 8; i++)
    {
        a = (i < 4) ? ((i % 2 == 0) ? !a : a) : !a;
        b = (i > 4) ? ((i % 2 == 0) ? !b : b) : !b;
        gridArray[offset + i] = sf::Vector2f((a) ? pixels.x.min : pixels.x.max,(b) ? pixels.y.min : pixels.y.max);
        gridArray[offset + i].color = sf::Color(
            255*BOARDER_INTENSITY,
            255*BOARDER_INTENSITY,
            255*BOARDER_INTENSITY
        );
    }
}

void setupText(sf::Text& text, std::string content, float intensity, sf::Font& font)
{
    text.setFillColor(sf::Color(255,  255,255,intensity * 255));
    text.setString(content);
    text.setFont(font);
    text.setCharacterSize(10);
}
void setTextPos(sf::Text& text, sf::Vector2f posWorld, Eigen::Matrix3f &viewToPixel, rangeTwoD pixels)
{
    sf::Vector2f posScreen = transformPoint(viewToPixel, posWorld);
    clampVector2(posScreen, pixels);
    text.setPosition(posScreen);
}
std::string numToString(float num, int exponent)
{
    float maxRound = std::pow(10, 5);
    num = round(num * maxRound) / maxRound;

    std::string str = std::to_string(num);
    int numPreChars = 0;

    for (int i = 0; i < str.length(); i++)
    {
        char c = str.at(i);
        if (c == '.')
        {
            numPreChars = i;
            break;
        }
    }
    int decimPlaces = abs(std::min(exponent,0));
    if (decimPlaces > 0)
    {
        decimPlaces++; // for dot
    }
    return  str.substr(0,  decimPlaces + numPreChars);
}

void populateNumberText(std::vector<sf::Text>& numberText, GridInfo& info, Eigen::Matrix3f &viewToPixel, rangeTwoD pixels, sf::Font& font)
{
    numberText.resize(info.xGrid.count + info.yGrid.count + 1 * (info.xSubGrid.count + info.ySubGrid.count));

    float x = info.xGrid.offset;
    float y = info.yGrid.offset;
    int i = 0;

    float gridIntensity = pow(info.gridIntensity, 0.5); // easing rate
    float subgridIntensity = pow(info.subgridIntensity, 5); // easing rate

    // n is out of use
    for (int n = 0; i < info.xGrid.count; i++)
    {
        if (x == 0)
        {
            gridIntensity = 1;
        }
        setupText(numberText.at(i), numToString(x, info.xGrid.exponent), gridIntensity, font);
        setTextPos(numberText.at(i), sf::Vector2f(x,0), viewToPixel, pixels);
        x += info.xGrid.spacing;
    }
    for (int n = 0; i < info.yGrid.count + info.xGrid.count; i++)
    {
        // dont repeat it
        if (y == 0)
        {
            y += info.yGrid.spacing;
            continue;
        }

        setupText(numberText.at(i), numToString(y, info.yGrid.exponent), gridIntensity, font);
        setTextPos(numberText.at(i), sf::Vector2f(0,y), viewToPixel, pixels);
        y += info.yGrid.spacing;
    }

    x = info.xSubGrid.offset;
    y = info.ySubGrid.offset;

    for (int n = 0; i < info.yGrid.count + info.xGrid.count + info.xSubGrid.count; i++)
    {
        float largeGrid = pow(10, -info.xGrid.exponent);
        float smallGrid = largeGrid * 10;

        // dont repeat it
        if (fmod((round(x * smallGrid) / smallGrid) * largeGrid, 1.0f) == 0)
        {
            x += info.xSubGrid.spacing;
            continue;
        }
        setupText(numberText.at(i), numToString(x, info.xSubGrid.exponent), subgridIntensity, font);
        setTextPos(numberText.at(i), sf::Vector2f(x,0), viewToPixel, pixels);
        x += info.xSubGrid.spacing;
    }

    for (int n = 0; i < info.yGrid.count + info.xGrid.count + info.xSubGrid.count + info.ySubGrid.count; i++)
    {
        float largeGrid = pow(10, -info.yGrid.exponent);
        float smallGrid = largeGrid * 10;

        // dont repeat it
        if (fmod((round(y * smallGrid) / smallGrid) * largeGrid, 1.0f) == 0)
        {
            y += info.ySubGrid.spacing;
            continue;
        }

        setupText(numberText.at(i), numToString(y, info.ySubGrid.exponent), subgridIntensity, font);
        setTextPos(numberText.at(i), sf::Vector2f(0,y), viewToPixel, pixels);
        y += info.ySubGrid.spacing;
    }
}

#endif
