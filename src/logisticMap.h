#ifndef LOGISTIC_MAP_H
#define LOGISTIC_MAP_H

#include <vector>
#include "range.h"

bool CHECK_OVERLAP = true;
int LOG_ONE_OVER_TOLERANCE = 8;

bool checkOverlap(std::vector<float> stableValues, float x, float tolerance)
{
    for (float value : stableValues)
    {
        if ((value - x) * (value - x) < tolerance)
        {
            return true;
        }
    }
    return false;
}

inline std::vector<float> getStableValues(float r, range output, int maxIterations, int maxSplits)
{
    float x = 0.5;
    for (int i = 0; i < maxIterations; i++)
    {
        x = r * x * (1 - x);
    }

    std::vector stableValues = std::vector<float>{};

    int hardStop = 0;

    for (int i = 0; i < maxSplits; i++)
    {
        hardStop++;
        if (hardStop > 10000)
        {
            return stableValues;
        }

        x = r * x * (1 - x);
        if (!output.contains(x))
        {
            i--; // regenerate
            continue;
        }

        if (!CHECK_OVERLAP)
        {
            stableValues.push_back(x);
            continue;
        }

        if (checkOverlap(stableValues, x, 1 / pow(10, LOG_ONE_OVER_TOLERANCE)))
        {
            break;
        }
        stableValues.push_back(x);
    }
    return stableValues;
}


#endif