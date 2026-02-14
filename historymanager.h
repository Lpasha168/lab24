#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QObject>
#include <QStringList>
#include <QVector>

class HistoryManager : public QObject
{
    Q_OBJECT

public:
    explicit HistoryManager(QObject *parent = nullptr);

    void addEntry(const QString &operation, double result);
    void clearHistory();
    bool saveToFile(const QString &filename);
    bool loadFromFile(const QString &filename);
    QStringList getHistory() const;
    double getResultFromHistory(int index) const;

private:
    struct HistoryEntry {
        QString operation;
        double result;
        QString resultHex;
        QString resultBin;
    };

    QVector<HistoryEntry> m_entries;
};

#endif // HISTORYMANAGER_H
