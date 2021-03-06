#include <QDebug>
#include "displaystyle.h"

DisplayStyle::DisplayStyle(const QString &label)
    : styleLabel(label)
{
#ifdef DEBUG_STYLE
    qDebug() << Q_FUNC_INFO << " " << label;
#endif
}

DisplayStyle::DisplayStyle(const QString &label, QColor initFg, QColor initBg)
    : styleLabel(label),
      bgColor(initBg),
      fgColor(initFg)
{
#ifdef DEBUG_STYLE
    qDebug() << Q_FUNC_INFO << " " << label;
#endif
}

DisplayStyle::~DisplayStyle()
{
#ifdef DEBUG_STYLE
    qDebug() << Q_FUNC_INFO;
#endif
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

QString DisplayStyle::GetHeadingFontSpec() const
{
    QString spec = headingFontName + ":" + QString::number(headingFontSize);
    return spec;
}

QString DisplayStyle::GetMessageFontSpec() const
{
    QString spec = messageFontName + ":" + QString::number(messageFontSize);
    return spec;
}

QColor DisplayStyle::GetBgColor() const
{
    return bgColor;
}

QColor DisplayStyle::GetFgColor() const
{
    return fgColor;
}

QString DisplayStyle::GetLabel() const {
    return styleLabel;
}
