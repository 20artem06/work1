#pragma once

#include <array>

#include <QColor>
#include <QtCore/Qt>

namespace video {

inline constexpr std::array<Qt::PenStyle, 5> kPenStyles = {
    Qt::SolidLine,
    Qt::DashLine,
    Qt::DashDotLine,
    Qt::DashDotDotLine,
    Qt::DotLine
};

float roundation(float value);
int divide(float numerator, float denominator);
float scaling(int gridCount, float maxValue, float minValue, bool autoGrid, bool logarithmic, float* step);
QColor makeColor(float hueValue, int palette, int type = 0);

}  // namespace video

