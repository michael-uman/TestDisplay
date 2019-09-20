#ifndef TESTSCRIPTMANAGER_H
#define TESTSCRIPTMANAGER_H

#include <QObject>
#include <QVector>
#include <QSharedPointer>

class TestScript : public QObject {
    Q_OBJECT
public:
    explicit TestScript(QChar key, QString name,
                        QString desc, QString path,
                        QObject * parent = nullptr);
    virtual ~TestScript();

    QString name() const {
        return scriptName;
    }

    QString desc() const {
        return scriptDesc;
    }

    QString path() const {
        return scriptPath;
    }

    QString key() const {
        return scriptKey;
    }

protected:
    QChar   scriptKey;
    QString scriptName;
    QString scriptDesc;
    QString scriptPath;
};

typedef QSharedPointer<TestScript>  TestScriptPtr;
typedef QVector<TestScriptPtr>      TestScriptVector;

class TestScriptMgr
{
public:
    explicit TestScriptMgr();
    virtual ~TestScriptMgr();

    bool load(QString filename);
    void clear();

    TestScriptPtr       getScriptForKey(int key) const;
    TestScriptVector & getVec()  {
        return scriptVec;
    }

private:
    TestScriptVector    scriptVec;

    QString             rootPath;
};

#endif // TESTSCRIPTMANAGER_H
