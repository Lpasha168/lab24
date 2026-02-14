#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QListWidgetItem>  // ВАЖНО: добавить этот include!
#include "historymanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class CalculatorMode {
    Standard,
    Scientific,
    Programmer
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // Цифры и базовые операции
    void digitClicked();
    void binaryOperatorClicked();
    void equalsClicked();
    void clearClicked();
    void clearEntryClicked();
    void changeSignClicked();
    void percentClicked();
    void pointClicked();

    // Расширенные операции
    void squareRootClicked();
    void powerClicked();

    // Научные функции
    void sinClicked();
    void cosClicked();
    void tanClicked();
    void logClicked();

    // Программистские функции
    void toBinaryClicked();
    void toHexClicked();
    void andClicked();
    void orClicked();
    void xorClicked();

    // Управление историей
    void onHistoryItemClicked(QListWidgetItem *item);
    void clearHistoryClicked();
    void saveHistoryClicked();
    void loadHistoryClicked();

    // Смена режима
    void onModeChanged(int index);

    // Действия меню
    void onActionExit();
    void onActionAbout();

private:
    Ui::MainWindow *ui;
    HistoryManager *m_historyManager;
    CalculatorMode m_currentMode;

    double m_leftOperand;
    QString m_currentOperator;
    bool m_isNewOperation;
    bool m_isTypingNumber;

    void setupUI();
    void createConnections();
    void calculate(double rightOperand);
    void addToHistory(const QString &operation, double result);
    void updateDisplay(const QString &text);
    void switchMode(CalculatorMode mode);
    void setupStandardMode();
    void setupScientificMode();
    void setupProgrammerMode();

    static const QMap<QString, int> m_operatorPriority;
};

#endif // MAINWINDOW_H
