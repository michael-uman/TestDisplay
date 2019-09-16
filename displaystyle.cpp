#include <QDebug>
#include "displaystyle.h"

DisplayStyle::DisplayStyle(const QString &label)
    : styleLabel(label)
{
    qDebug() << Q_FUNC_INFO << " " << label;
}

DisplayStyle::DisplayStyle(const QString &label, QColor initFg, QColor initBg)
    : styleLabel(label),
      bgColor(initBg),
      fgColor(initFg)
{
    qDebug() << Q_FUNC_INFO << " " << label;
}

DisplayStyle::~DisplayStyle()
{
    qDebug() << Q_FUNC_INFO;
}

bool DisplayStyle::isValid() const
{
    return bIsValid;
}

bool DisplayStyle::SetHeadingFontName(const QString &fontName)
{
    headingFontName = fontName;
    return true;
}

bool DisplayStyle::SetMessageFontName(const QString &fontName)
{
    messageFontName = fontName;
    return true;
}

bool DisplayStyle::SetHeadingFontSize(int fontSize)
{
    headingFontSize = fontSize;
    return true;
}

bool DisplayStyle::SetMessageFontSize(int fontSize)
{
    messageFontSize = fontSize;
    return true;
}

bool DisplayStyle::SetHeadingFontWeight(QFont::Weight weight)
{
    headingFontWeight = weight;
    return true;
}

bool DisplayStyle::SetMessageFontWeight(QFont::Weight weight)
{
    messageFontWeight = weight;
    return true;
}

bool DisplayStyle::SetTimerEnabled(bool enabled)
{
    displayTimer = enabled;
    return true;
}

bool DisplayStyle::SetBgColor(QColor color)
{
    bgColor = color;
    return true;
}

bool DisplayStyle::SetFgColor(QColor color)
{
    fgColor = color;
    return true;
}

QFont DisplayStyle::GetHeadingFont() const
{
    QFont font(headingFontName, headingFontSize, headingFontWeight);
    return font;
}

QFont DisplayStyle::GetMessageFont() const
{
    QFont font(messageFontName, messageFontSize, messageFontWeight);
    return font;
}

QColor DisplayStyle::GetBgColor() const
{
    return bgColor;
}

QColor DisplayStyle::GetFgColor() const
{
    return fgColor;
}

