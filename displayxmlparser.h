#ifndef MYXMLPARSER_H
#define MYXMLPARSER_H

#include <QXmlDefaultHandler>
#include "displaystyle.h"

class DisplayXmlParser : public QXmlDefaultHandler
{
public:
    DisplayXmlParser(DisplayStyleVec & vec);

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &atts) override;
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName) override;

    bool characters(const QString &name) override;

    QFont::Weight parseWeight(QString weight);

private:

    DisplayStyleVec &   styleVec;

    DisplayStylePtr     currentStyle;

    // Parsing state
    bool                bInStyleSheet   = false;
    bool                bInStyle        = false;
    bool                bInFont         = false;
    bool                bInBg           = false;
    bool                bInFg           = false;
    bool                bInColor        = false;
    bool                bInTimer        = false;

    QString             inFontType;
    QString             inColorType;
};

#endif // MYXMLPARSER_H
