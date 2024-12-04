#include <iostream>
#include <SFML/Graphics.hpp>
#include <Eigen/Dense>
#include <format>
#include <unistd.h>

#include "grid.h"
#include "logisticMap.h"
#include "range.h"

int MAX_ITERATIONS = 1000;
int MAX_SPLITS = 100;
int X_STEPS = 1000;
float RELOAD_TIME = 3;
bool RELOAD = true;

Eigen::Matrix3f transformationMatrix;
Eigen::Matrix3f screenSpaceMatrix;


int getNumVerticies(rangeTwoD viewRange_,  int xSteps)
{
    range xStepRange = range(0, xSteps);
    int numVerticies = 0;
    for (int xStepNum = 0; xStepNum < xStepRange.max; xStepNum++)
    {
        numVerticies += getStableValues(xStepRange.mapTo(viewRange_.x, xStepNum), viewRange_.y, MAX_ITERATIONS, MAX_SPLITS).size();
    }
    return numVerticies;
}


void populateVeticies(sf::VertexArray& vertices, rangeTwoD viewRange_, int xSteps)
{
    int vertIndex = 0;
    int vertCount = vertices.getVertexCount();
    // std::clog << vertCount << " " << getNumVerticies(xRange, pixelRange.x) << std::endl;

    range xStepRange(0,xSteps);

    for (int xPix = 0; xPix < xStepRange.max; xPix++)
    {
        float x = xStepRange.mapTo(viewRange_.x, xPix);


        std::vector<float> stableValues = getStableValues(x, viewRange_.y, MAX_ITERATIONS, MAX_SPLITS);

        for (float value : stableValues)
        {
            sf::Vector2 vec = sf::Vector2f(x, value);
            vertices[vertIndex].position = vec;

            // vertices[vertIndex].position = sf::Vector2f(xRange.mapTo(pixelRange.x, x), yRange.mapTo(pixelRange.y, value));
            vertices[vertIndex].color = sf::Color(255,0,0, 255);
            vertIndex++;
        }
        std::clog << "x : " << x << ", vertex : " << vertIndex << " / " << vertCount << "     " << "\r";
    }
    std::clog << std::endl;
}

int main()
{
    int screenWidth = 1000;
    int screenHeight = 1000;

    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Logistic Map");

    rangeTwoD viewRange = rangeTwoD(
        range(0, 4),
        range(0, 4)
    );

    int margin = 50;
    rangeTwoD pixelRange = rangeTwoD(
        range(0 + margin, window.getSize().x - margin, false, true),
        range(0 + margin, window.getSize().y - margin, true, true)
    );


    std::clog << "counting verticies ...";

    int numVerticies = getNumVerticies(viewRange, X_STEPS);

    sf::VertexArray worldSpaceVertices(sf::Points, numVerticies);
    sf::VertexArray pixelSpaceVertices(sf::Points, numVerticies);

    populateVeticies(
        worldSpaceVertices,
        viewRange,
        X_STEPS
    );

    sf::Vector2f savedMouseWorldPos = sf::Vector2f(sf::Mouse::getPosition(window));
    bool pressedFrame = false;

    float xZoom = log(viewRange.x.size) / log(10);
    float yZoom = log(viewRange.y.size) / log(10);

    Eigen::Matrix3f viewToPixel;

    sf::Font arial;
    arial.loadFromFile("../assets/fonts/arial.ttf");

    sf::Text fpsText;
    fpsText.setFont(arial);
    fpsText.setCharacterSize(14);
    fpsText.setPosition(100, 100);

    float fpsAverageReloadTime = 0.3;
    float fpsTotal, fpsCount;

    sf::Clock dtClock;
    sf::Clock reloadClock;
    sf::Clock fpsAverageClock;

    while (window.isOpen())
    {
        sf::Time elapsed = dtClock.restart();
        float dt = static_cast<float>(elapsed.asSeconds());

        float fpsAverageSeconds = fpsAverageClock.getElapsedTime().asSeconds();
        fpsTotal += (1 / elapsed.asSeconds());
        fpsCount += 1;

        if (fpsAverageSeconds > fpsAverageReloadTime){
            fpsAverageClock.restart();
            fpsText.setString("FPS : " + std::to_string(
                static_cast<int>(fpsTotal/fpsCount)
            ));
            fpsTotal = 0;
            fpsCount = 0;
        }


        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // reload

        float reloadSeconds = reloadClock.getElapsedTime().asSeconds();
        if (reloadSeconds > RELOAD_TIME && RELOAD)
        {
            reloadClock.restart();
            numVerticies = getNumVerticies(viewRange, X_STEPS);
            worldSpaceVertices.resize(numVerticies);
            populateVeticies(
                worldSpaceVertices,
                viewRange,
                X_STEPS
            );
        }
        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f currentMouseWorldPos = transformPoint(viewToPixel.inverse(), sf::Vector2f(currentMousePos));

        // zooming

        if (event.type == sf::Event::MouseWheelScrolled)
        {
            float viewScalar = event.mouseWheelScroll.delta * dt * 0.3f;

            if (dt > 0.05)
            {
                viewScalar = event.mouseWheelScroll.delta * 0.005;
            }

            xZoom += viewScalar;
            yZoom += viewScalar;

            float minZoom = -3;
            float maxZoom = 3;

            xZoom = std::max(std::min(xZoom, maxZoom), minZoom);
            yZoom = std::max(std::min(yZoom, maxZoom), minZoom);

            viewRange.x.setSize(pow(10, xZoom), currentMouseWorldPos.x);
            viewRange.y.setSize(pow(10, yZoom), currentMouseWorldPos.y);
        }

        // panning

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
            pixelRange.x.contains(currentMousePos.x) &&
            pixelRange.y.contains(currentMousePos.y)
        )
        {
            if (pressedFrame == false)
            {
                pressedFrame = true;
                savedMouseWorldPos = currentMouseWorldPos;

            }
            sf::Vector2f currentMouseWorldPos = transformPoint(viewToPixel.inverse(), sf::Vector2f(currentMousePos));
            sf::Vector2f moveDist = savedMouseWorldPos - currentMouseWorldPos;

            viewRange.x += moveDist.x * (1 / viewRange.x.size);
            viewRange.y += moveDist.y * (1 / viewRange.y.size);
        }
        else
        {
            pressedFrame = false;
        }

        // apply transform

        viewToPixel = transformToMatrix3f(viewRange, pixelRange);

        sf::Vector2 vec = transformPoint(viewToPixel, sf::Vector2f(0, 0));

        pixelSpaceVertices.resize(worldSpaceVertices.getVertexCount());
        for (int vertexIndex = 0; vertexIndex < worldSpaceVertices.getVertexCount(); vertexIndex++)
        {
            sf::Vector2f pos = transformPoint(viewToPixel, worldSpaceVertices[vertexIndex].position);
            if (!clampedVector2(worldSpaceVertices[vertexIndex].position, viewRange))
                {pixelSpaceVertices[vertexIndex].color = sf::Color(0,0,0,0);}
            else
                {pixelSpaceVertices[vertexIndex].color = worldSpaceVertices[vertexIndex].color;}
            pixelSpaceVertices[vertexIndex].position = pos;
        }

        // grids
        // putting a lot of trust in the user of the function (me, not deserved)
        GridInfo gridInfo = setupGrid(viewRange, 10, true);
        sf::VertexArray gridVerticies(sf::Lines, gridInfo.vertCount + 8); // for the boarders
        std::vector<sf::Text> numberText{};

        populateGridVerticies(gridVerticies, gridInfo, viewRange, viewToPixel);
        populateBoarderVerticies(gridVerticies, gridInfo.vertCount, pixelRange);
        populateNumberText(numberText, gridInfo, viewToPixel, pixelRange, arial);

        window.clear();
        window.draw(gridVerticies);
        window.draw(pixelSpaceVertices);
        window.draw(fpsText);
        for (int i = 0; i < numberText.size(); i++){window.draw(numberText.at(i));}
        window.display();
    }
    return 0;
}
