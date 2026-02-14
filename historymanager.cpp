#include "historymanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

HistoryManager::HistoryManager(QObject *parent)
    : QObject(parent)
{
}

void HistoryManager::addEntry(const QString &operation, double result)
{
    HistoryEntry entry;
    entry.operation = operation;
    entry.result = result;

    // Добавляем представления для разных систем счисления
    int intResult = static_cast<int>(result);
    entry.resultHex = QString::number(intResult, 16).toUpper();
    entry.resultBin = QString::number(intResult, 2);

    m_entries.append(entry);

    // Ограничиваем историю 100 записями
    if (m_entries.size() > 100) {
        m_entries.removeFirst();
    }
}

void HistoryManager::clearHistory()
{
    m_entries.clear();
}

bool HistoryManager::saveToFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    for (const auto &entry : m_entries) {
        out << entry.operation << "|"
            << entry.result << "|"
            << entry.resultHex << "|"
            << entry.resultBin << "\n";
    }

    file.close();
    return true;
}

bool HistoryManager::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    m_entries.clear();
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split("|");
        if (parts.size() >= 2) {
            HistoryEntry entry;
            entry.operation = parts[0];
            entry.result = parts[1].toDouble();
            if (parts.size() >= 3) entry.resultHex = parts[2];
            if (parts.size() >= 4) entry.resultBin = parts[3];
            m_entries.append(entry);
        }
    }

    file.close();
    return true;
}

QStringList HistoryManager::getHistory() const
{
    QStringList history;
    for (const auto &entry : m_entries) {
        history.append(QString("%1 = %2").arg(entry.operation).arg(entry.result));
    }
    return history;
}

double HistoryManager::getResultFromHistory(int index) const
{
    if (index >= 0 && index < m_entries.size()) {
        return m_entries[index].result;
    }
    return 0.0;
}
