#include <QDebug>
#include <QFile>
#include <QDomDocument>
#include <QDir>

#include "testscriptmanager.h"

TestScript::TestScript(QChar key, QString name, QString desc,
                       QString path, QObject * parent)
    :   QObject(parent),
        scriptKey(key),
        scriptName(name),
        scriptDesc(desc),
        scriptPath(path)
{
//#ifdef DEBUG
//    qDebug() << Q_FUNC_INFO;
//    qDebug() << "Script Name : " << name << " Desc : " << desc << " Path : " << path;
//#endif
}

TestScript::~TestScript()
{
#ifdef DEBUG
    qDebug() << Q_FUNC_INFO;
#endif
}


TestScriptMgr::TestScriptMgr()
{

}

TestScriptMgr::~TestScriptMgr()
{

}

bool TestScriptMgr::load(QString filename)
{
    QDomDocument        scriptDom("scripts");
    QFile               xmlFile(filename);

    if (!xmlFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open script xml " << filename;
        return false;
    }

    if (!scriptDom.setContent(&xmlFile)) {
        qWarning() << "Unable to parse xml file";
        xmlFile.close();
        return false;
    }

    xmlFile.close();

    QDomElement root = scriptDom.documentElement();
    if (!root.tagName().compare("scripts", Qt::CaseInsensitive) == 0) {
        qWarning() << "Invalid XML file";
        return false;
    }

    QDomNode child = root.firstChild();
    while (!child.isNull()) {
        QString tag = child.toElement().tagName();

        if (tag.compare("rootpath", Qt::CaseInsensitive) == 0) {
            rootPath = child.toElement().text();
            qDebug() << "Root path = " << rootPath;
        } else if (tag.compare("script", Qt::CaseInsensitive) == 0) {
            QDomNode    scriptChild = child.firstChild();
            QChar       scriptKey;
            QString     scriptName, scriptDesc, scriptPath;

            while (!scriptChild.isNull()) {
                tag = scriptChild.toElement().tagName();

#ifdef DEBUG
                qDebug() << "tag " << tag;
#endif
                if (tag.compare("key", Qt::CaseInsensitive) == 0) {
                    scriptKey = scriptChild.toElement().text()[0];
                } else if (tag.compare("name", Qt::CaseInsensitive) == 0) {
                    scriptName = scriptChild.toElement().text();
                } else if (tag.compare("description", Qt::CaseInsensitive) == 0) {
                    scriptDesc = scriptChild.toElement().text();
                } else if (tag.compare("path", Qt::CaseInsensitive) == 0) {
                    scriptPath = scriptChild.toElement().text();
                }
                scriptChild = scriptChild.nextSibling();
            }

            // If the script path is not absolute, append the root path...
            if (!scriptPath.startsWith("/")) {
                scriptPath = QDir::cleanPath(rootPath + QDir::separator() + scriptPath);
            }

            // Check if the script file exists...
            if (QFile::exists(scriptPath)) {
                qInfo() << "Adding script " << scriptName << " (" << scriptPath << ")";
                TestScriptPtr newScript(new TestScript(scriptKey, scriptName,
                                                       scriptDesc, scriptPath));
                scriptVec.push_back(newScript);
            } else {
                qWarning() << "Script " << scriptName << " not found";
            }
        } else {
            qWarning() << "Invalid tag in xml";
        }

        child = child.nextSibling();
    }

    return true;
}

void TestScriptMgr::clear()
{
    scriptVec.clear();
}

TestScriptPtr TestScriptMgr::getScriptForKey(int key) const
{
    for (auto script : scriptVec) {
        if (script->key() == key) {
            return script;
        }
    }

    return TestScriptPtr();
}

