#ifndef DISPLAYSTYLE_H
#define DISPLAYSTYLE_H


#include <QColor>
#include <QFont>
#include <QVector>
#include <QSharedPointer>
#include "testdisplay_defaults.h"


class DisplayStyle : public QObject {
    Q_OBJECT

public:
    DisplayStyle(const QString & label);
    DisplayStyle(const QString & label, QColor initFg, QColor initBg);
    virtual ~DisplayStyle();

    bool            isValid() const;

    bool            SetHeadingFontName(const QString & fontName);
    bool            SetMessageFontName(const QString & fontName);
    bool            SetHeadingFontSize(int fontSize);
    bool            SetMessageFontSize(int fontSize);
    bool            SetHeadingFontWeight(QFont::Weight weight);
    bool            SetMessageFontWeight(QFont::Weight weight);
    bool            SetTimerEnabled(bool enabled);

    bool            SetBgColor(QColor color);
    bool            SetFgColor(QColor color);

    QFont           GetHeadingFont() const;
    QFont           GetMessageFont() const;
    QColor          GetBgColor() const;
    QColor          GetFgColor() const;

    QString         GetLabel() const;

protected:
    QString         styleLabel;

    bool            bIsValid            = false;
    bool            displayTimer        = false;

    QString         headingFontName     = HEADING_FONT_NAME;
    QFont::Weight   headingFontWeight   = HEADING_FONT_WEIGHT;
    int             headingFontSize     = HEADING_FONT_POINTS;

    QString         messageFontName     = MESSAGE_FONT_NAME;
    QFont::Weight   messageFontWeight   = MESSAGE_FONT_WEIGHT;
    int             messageFontSize     = MESSAGE_FONT_POINTS;

    QColor          bgColor;
    QColor          fgColor;
};

using DisplayStylePtr = QSharedPointer<DisplayStyle>;
using DisplayStyleVec = QVector<DisplayStylePtr>;

#endif // DISPLAYSTYLE_H
