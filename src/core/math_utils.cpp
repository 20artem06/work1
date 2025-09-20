#include "math_utils.h"

#include <algorithm>
#include <cmath>
#include <QtGlobal>

namespace video {

float roundation(float value)
{
    float tmp = 1.0e-37f;
    float absValue = std::abs(value);
    for (int i = -37; i < 38; ++i) {
        tmp *= 10.0f;
        if (tmp > absValue) {
            break;
        }
    }
    tmp /= 10.0f;
    const int quotient = divide(absValue, tmp);
    return (quotient >= 5) ? tmp : tmp / 10.0f;
}

int divide(float numerator, float denominator)
{
    const float result = numerator / denominator;
    if (result >= 0.0f) {
        return static_cast<int>(result);
    }
    return static_cast<int>(result) - 1;
}

float scaling(int gridCount, float maxValue, float minValue, bool autoGrid, bool logarithmic, float* step)
{
    if (gridCount == 0) {
        return 0.0f;
    }

    float localStep = (maxValue - minValue) / static_cast<float>(gridCount);
    if (localStep == 0.0f) {
        return 0.0f;
    }

    float tmp = 1.0e-37f;
    float magnitude = localStep;
    for (int i = -37; i < 38; ++i) {
        tmp *= 10.0f;
        if (tmp > magnitude) {
            break;
        }
    }

    int integerStep = static_cast<int>(localStep * 100.0f / tmp);
    if ((integerStep - (localStep * 100.0f / tmp)) < 0.0f) {
        ++integerStep;
    }

    localStep = integerStep * tmp / 100.0f;
    if (step) {
        *step = localStep;
    }

    if (autoGrid) {
        if (logarithmic) {
            localStep = std::floor(localStep);
        } else {
            double tmpAuto = 1.0e-37;
            for (int i = -37; i < 38; ++i) {
                tmpAuto *= 10.0;
                if (tmpAuto > localStep) {
                    break;
                }
            }
            localStep = static_cast<float>(tmpAuto / 10.0);
        }
        if (step) {
            *step = localStep;
        }
    }

    float gridMin = minValue - (localStep * gridCount - (maxValue - minValue)) / 2.0f;

    tmp = 1.0e-37f;
    magnitude = std::abs(gridMin);
    const int sign = (gridMin * magnitude >= 0.0f) ? 1 : -1;
    for (int i = -37; i < 38; ++i) {
        tmp *= 10.0f;
        if (tmp > magnitude) {
            break;
        }
    }

    gridMin = sign * (1 + static_cast<int>(magnitude * 10.0f / tmp)) * tmp / 10.0f;
    while (minValue < gridMin) {
        gridMin -= tmp / 10.0f;
    }

    return gridMin;
}

QColor makeColor(float hueValue, int palette, int type)
{
    Q_UNUSED(type);

    float hx = std::clamp(hueValue, 0.0f, 1.0f);
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;

    switch (palette) {
    case 1:
        red = (hx > 0.5f) ? 1.0f : hx * 2.0f;
        blue = (hx > 0.5f) ? 0.0f : 2.0f * (0.5f - hx);
        green = 1.5f * hx * (1.0f - hx) / 0.5f;
        break;
    case 0:
        red = hx;
        blue = 1.0f - hx;
        green = 0.0f;
        break;
    case 2:
        red = green = blue = 1.0f - hx;
        break;
    default:
        red = hx;
        blue = 1.0f - hx;
        green = 0.0f;
        break;
    }

    red = std::clamp(red, 0.0f, 1.0f);
    green = std::clamp(green, 0.0f, 1.0f);
    blue = std::clamp(blue, 0.0f, 1.0f);

    return QColor::fromRgbF(red, green, blue);
}

}  // namespace video

