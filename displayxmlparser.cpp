#include <QDebug>
#include "displayxmlparser.h"

DisplayXmlParser::DisplayXmlParser(DisplayStyleVec & vec)
    : styleVec(vec)
{

}

bool DisplayXmlParser::startElement(const QString &namespaceURI, const QString &localName,
                               const QString &qName, const QXmlAttributes &atts)
{
#ifdef DEBUG_XML_PARSER
    qDebug() << Q_FUNC_INFO;
    qDebug() << namespaceURI << " " << localName << " " << qName; //<< " " << atts;
    for (int i = 0 ; i < atts.count(); i++) {
        qDebug() << atts.qName(i) << " " << atts.value(i);
    }
#else
    Q_UNUSED(namespaceURI)
    Q_UNUSED(qName)
#endif

    if (localName == "stylesheet") {
        if (bInStyleSheet) {
            qDebug() << "Encountered a 'stylesheet' inside a 'stylesheet'";
            return false;
        }
        bInStyleSheet = true;
    } else if (localName == "style") {
        if (!bInStyleSheet) {
            qDebug() << "Encountered a 'style' outside a 'stylesheet'";
            return false;
        }
        if (bInStyle) {
            qDebug() << "Encountered a 'style' inside a 'style'";
            return false;
        }

        // create new style
        QString label;

        for (int i = 0 ; i < atts.count() ; i ++) {
            if (atts.qName(i) == "label") {
                label = atts.value(i);
                break;
            }
        }

        currentStyle = DisplayStylePtr(new DisplayStyle(label));

        bInStyle = true;
    } else if (localName == "font") {
        if (!bInStyleSheet || !bInStyle) {
            qDebug() << "Encountered a 'font' outside of 'stylesheet' or 'style'";
            return false;
        }
        QString fontSize;
        QFont::Weight fontWeight = QFont::Normal;

//        QString fontType;

        for (int i = 0 ; i < atts.count() ; i++) {
            if (atts.qName(i) == "type") {
                inFontType = atts.value(i);
            } else if (atts.qName(i) == "size") {
                fontSize = atts.value(i);
            } else if (atts.qName(i) == "weight") {
                fontWeight = parseWeight(atts.value(i));
            }
        }

        if (inFontType == "header") {
            currentStyle->SetHeadingFontSize(fontSize.toInt());
            currentStyle->SetHeadingFontWeight(fontWeight);
        } else if (inFontType == "message") {
            currentStyle->SetMessageFontSize(fontSize.toInt());
            currentStyle->SetMessageFontWeight(fontWeight);
        }

        bInFont = true;
    } else if (localName == "timer") {
        if (!bInStyleSheet || !bInStyle) {
            qDebug() << "Encountered a 'timer' outside of 'stylesheet' or 'style'";
            return false;
        }

        bInTimer = true;
    } else if ((localName == "foreground") || (localName == "background")) {
        if (!bInStyleSheet || !bInStyle) {
            qDebug() << "Encountered a '" << localName <<  "' outside of 'stylesheet' or 'style'";
            return false;
        }
        inColorType = localName;
        if (localName == "foreground") {
            bInFg = true;
        } else if (localName == "background") {
            bInBg = true;
        }
    } else if (localName == "color") {
        if (!bInStyleSheet || !bInStyle || !(bInFg||bInBg)) {
            qDebug() << "Encountered a 'color' outside of 'stylesheet' or 'style'";
            return false;
        }

        bInColor = true;
    }

    return true;
}

bool DisplayXmlParser::endElement(const QString &namespaceURI, const QString &localName,
                             const QString &qName)
{
#ifdef DEBUG_XML_PARSER
    qDebug() << Q_FUNC_INFO;
    qDebug() << namespaceURI << " " << localName << " " << qName;
#else
    Q_UNUSED(namespaceURI)
    Q_UNUSED(qName)
#endif

    if (localName == "stylesheet") {
        if (!bInStyleSheet) {
            qDebug() << "Encountered a close 'stylesheet' outside a 'stylesheet'";
            return false;
        }
        bInStyleSheet = false;
    } else if (localName == "style") {
        if (!bInStyleSheet) {
            qDebug() << "Encountered a close 'style' outside a 'stylesheet'";
            return false;
        }
        if (!bInStyle) {
            qDebug() << "Encountered a close 'style' outside a 'style'";
            return false;
        }

        styleVec.push_back(currentStyle);

        currentStyle = nullptr;

        bInStyle = false;
    } else if (localName == "font") {
        if (!bInStyleSheet || !bInStyle || !bInFont) {
            qDebug() << "Encountered a close 'font' outside of a 'font' element";
            return false;
        }
        bInFont = false;
    } else if (localName == "timer") {
        if (!bInStyleSheet || !bInStyle || !bInTimer) {
            qDebug() << "Encountered a close 'timer' outside of a 'timer' element";
            return false;
        }
        bInTimer = false;
    } else if (localName == "foreground") {
        if (!bInStyleSheet || !bInStyle || !bInFg) {
            qDebug() << "Encountered a close 'foreground' outside of a 'style' element";
            return false;
        }
        bInFg = false;

    } else if (localName == "background") {
        if (!bInStyleSheet || !bInStyle || !bInBg) {
            qDebug() << "Encountered a close 'background' outside of a 'style' element";
            return false;
        }
        bInBg = false;
    } else if (localName == "color") {
        if (!bInStyleSheet || !bInStyle || !bInColor) {
            qDebug() << "Encountered a close 'background' outside of a 'style' element";
            return false;
        }

        bInColor = false;
    }
    return true;
}

bool DisplayXmlParser::characters(const QString &name)
{
#ifdef DEBUG_XML_PARSER
    qDebug() << Q_FUNC_INFO;
    qDebug() << name.trimmed();
#else
    Q_UNUSED(name)
#endif

    if (bInFont) {
        if (inFontType == "header") {
            currentStyle->SetHeadingFontName(name.trimmed());
        } else if (inFontType == "message") {
            currentStyle->SetMessageFontName(name.trimmed());
        }
    } else if (bInTimer) {
        currentStyle->SetTimerEnabled(name == "1");
    } else if (bInColor) {
        if (inColorType == "foreground") {
            currentStyle->SetFgColor(QColor(name.trimmed()));
        } else if (inColorType == "background") {
            currentStyle->SetBgColor(QColor(name.trimmed()));
        }
    }

    return true;
}

QFont::Weight DisplayXmlParser::parseWeight(QString weight)
{
    QVector<QPair<QString, QFont::Weight>> weightVec = {
        {
            { "thin", QFont::Thin },
            { "extralight", QFont::ExtraLight },
            { "light", QFont::Light },
            { "normal", QFont::Normal },
            { "medium", QFont::Medium },
            { "demibold", QFont::DemiBold },
            { "bold", QFont::Bold },
            { "extrabold", QFont::ExtraBold },
            { "black", QFont::Black },
        }
    };

    for (auto weightPair : weightVec) {
        if (weightPair.first == weight) {
            return weightPair.second;
        }
    }

    return QFont::Normal;
}
