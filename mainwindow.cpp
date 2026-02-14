#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGridLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QListWidgetItem>  // ВАЖНО: добавить!
#include <cmath>
#include <QDebug>

const QMap<QString, int> MainWindow::m_operatorPriority = {
    {"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}, {"^", 3},
    {"AND", 1}, {"OR", 1}, {"XOR", 1}
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_historyManager(new HistoryManager(this))
    , m_currentMode(CalculatorMode::Standard)
    , m_leftOperand(0.0)
    , m_currentOperator("")
    , m_isNewOperation(true)
    , m_isTypingNumber(false)
{
    ui->setupUi(this);
    setupUI();
    createConnections();
    setupStandardMode();

    setWindowTitle("Расширенный калькулятор");
    setFixedSize(650, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // Основной дисплей
    ui->display->setAlignment(Qt::AlignRight);
    ui->display->setReadOnly(true);
    ui->display->setText("0");
    ui->display->setMaxLength(20);

    // Настройка истории
    ui->historyList->setSelectionMode(QAbstractItemView::SingleSelection);

    // Комбо-бокс для выбора режима
    ui->modeCombo->addItem("Стандартный");
    ui->modeCombo->addItem("Научный");
    ui->modeCombo->addItem("Программист");

    // Скрываем дополнительные группы кнопок
    ui->scientificGroup->hide();
    ui->programmerGroup->hide();

    // Устанавливаем подсказки для кнопок
    ui->btnSqrt->setToolTip("Квадратный корень");
    ui->btnPower->setToolTip("Возведение в степень");
    ui->btnPercent->setToolTip("Процент");
    ui->btnPlusMinus->setToolTip("Смена знака");
}

void MainWindow::createConnections()
{
    // Цифры
    connect(ui->btn0, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btn1, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btn2, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btn3, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btn4, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btn5, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btn6, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btn7, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btn8, &QPushButton::clicked, this, &MainWindow::digitClicked);
    connect(ui->btn9, &QPushButton::clicked, this, &MainWindow::digitClicked);

    // Базовые операции
    connect(ui->btnPlus, &QPushButton::clicked, this, &MainWindow::binaryOperatorClicked);
    connect(ui->btnMinus, &QPushButton::clicked, this, &MainWindow::binaryOperatorClicked);
    connect(ui->btnMultiply, &QPushButton::clicked, this, &MainWindow::binaryOperatorClicked);
    connect(ui->btnDivide, &QPushButton::clicked, this, &MainWindow::binaryOperatorClicked);

    // Унарные операции
    connect(ui->btnSqrt, &QPushButton::clicked, this, &MainWindow::squareRootClicked);
    connect(ui->btnPower, &QPushButton::clicked, this, &MainWindow::powerClicked);
    connect(ui->btnPercent, &QPushButton::clicked, this, &MainWindow::percentClicked);
    connect(ui->btnPlusMinus, &QPushButton::clicked, this, &MainWindow::changeSignClicked);

    // Управление
    connect(ui->btnEquals, &QPushButton::clicked, this, &MainWindow::equalsClicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &MainWindow::clearClicked);
    connect(ui->btnClearEntry, &QPushButton::clicked, this, &MainWindow::clearEntryClicked);
    connect(ui->btnPoint, &QPushButton::clicked, this, &MainWindow::pointClicked);
    connect(ui->btnBackspace, &QPushButton::clicked, [this]() {
        QString text = ui->display->text();
        if (text.length() > 1) {
            text.chop(1);
            updateDisplay(text);
        } else {
            updateDisplay("0");
        }
    });

    // Научные функции
    connect(ui->btnSin, &QPushButton::clicked, this, &MainWindow::sinClicked);
    connect(ui->btnCos, &QPushButton::clicked, this, &MainWindow::cosClicked);
    connect(ui->btnTan, &QPushButton::clicked, this, &MainWindow::tanClicked);
    connect(ui->btnLog, &QPushButton::clicked, this, &MainWindow::logClicked);

    // Программистские функции
    connect(ui->btnBin, &QPushButton::clicked, this, &MainWindow::toBinaryClicked);
    connect(ui->btnHex, &QPushButton::clicked, this, &MainWindow::toHexClicked);
    connect(ui->btnAnd, &QPushButton::clicked, this, &MainWindow::andClicked);
    connect(ui->btnOr, &QPushButton::clicked, this, &MainWindow::orClicked);
    connect(ui->btnXor, &QPushButton::clicked, this, &MainWindow::xorClicked);

    // История
    connect(ui->historyList, &QListWidget::itemClicked,
            this, &MainWindow::onHistoryItemClicked);
    connect(ui->btnClearHistory, &QPushButton::clicked,
            this, &MainWindow::clearHistoryClicked);
    connect(ui->btnSaveHistory, &QPushButton::clicked,
            this, &MainWindow::saveHistoryClicked);

    // Режимы
    connect(ui->modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onModeChanged);

    // Действия меню
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onActionExit);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAbout);
    connect(ui->actionSaveHistory, &QAction::triggered, this, &MainWindow::saveHistoryClicked);
    connect(ui->actionLoadHistory, &QAction::triggered, this, &MainWindow::loadHistoryClicked);
}

void MainWindow::digitClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QString digit = button->text();
    QString currentText = ui->display->text();

    // Проверка на переполнение
    if (currentText.length() >= 15) {
        QMessageBox::warning(this, "Предупреждение", "Достигнут максимум цифр!");
        return;
    }

    if (m_isNewOperation || currentText == "0") {
        currentText = digit;
        m_isNewOperation = false;
    } else {
        currentText += digit;
    }

    updateDisplay(currentText);
    m_isTypingNumber = true;
}

void MainWindow::binaryOperatorClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QString op = button->text();
    double currentValue = ui->display->text().toDouble();

    if (!m_currentOperator.isEmpty() && !m_isNewOperation) {
        calculate(currentValue);
    } else {
        m_leftOperand = currentValue;
    }

    m_currentOperator = op;
    m_isNewOperation = true;
    m_isTypingNumber = false;
}

void MainWindow::equalsClicked()
{
    if (m_currentOperator.isEmpty() || m_isNewOperation) return;

    double rightOperand = ui->display->text().toDouble();
    calculate(rightOperand);

    m_currentOperator.clear();
    m_isNewOperation = true;
}

void MainWindow::calculate(double rightOperand)
{
    double result = 0.0;
    QString operation;

    // Проверка на целые числа для побитовых операций
    if (m_currentOperator == "AND" || m_currentOperator == "OR" || m_currentOperator == "XOR") {
        int leftInt = static_cast<int>(m_leftOperand);
        int rightInt = static_cast<int>(rightOperand);
        operation = QString("%1 %2 %3").arg(leftInt).arg(m_currentOperator).arg(rightInt);

        if (m_currentOperator == "AND") {
            result = leftInt & rightInt;
        } else if (m_currentOperator == "OR") {
            result = leftInt | rightInt;
        } else if (m_currentOperator == "XOR") {
            result = leftInt ^ rightInt;
        }
    } else {
        operation = QString("%1 %2 %3").arg(m_leftOperand)
        .arg(m_currentOperator).arg(rightOperand);

        if (m_currentOperator == "+") {
            result = m_leftOperand + rightOperand;
        } else if (m_currentOperator == "-") {
            result = m_leftOperand - rightOperand;
        } else if (m_currentOperator == "*") {
            result = m_leftOperand * rightOperand;
        } else if (m_currentOperator == "/") {
            if (rightOperand == 0) {
                QMessageBox::warning(this, "Ошибка", "Деление на ноль невозможно!");
                clearClicked();
                return;
            }
            result = m_leftOperand / rightOperand;
        } else if (m_currentOperator == "^") {
            result = std::pow(m_leftOperand, rightOperand);
        }
    }

    // Округление до разумного количества знаков
    QString resultStr = QString::number(result, 'g', 12);
    updateDisplay(resultStr);
    addToHistory(operation, result);

    m_leftOperand = result;
    m_currentOperator.clear();
}

void MainWindow::clearClicked()
{
    m_leftOperand = 0.0;
    m_currentOperator.clear();
    m_isNewOperation = true;
    m_isTypingNumber = false;
    updateDisplay("0");
}

void MainWindow::clearEntryClicked()
{
    updateDisplay("0");
    m_isTypingNumber = false;
}

void MainWindow::changeSignClicked()
{
    QString currentText = ui->display->text();
    double value = currentText.toDouble();
    value = -value;
    updateDisplay(QString::number(value));
}

void MainWindow::percentClicked()
{
    double value = ui->display->text().toDouble();
    value = value / 100.0;
    updateDisplay(QString::number(value));
    addToHistory(QString("%1%").arg(value * 100), value);
}

void MainWindow::pointClicked()
{
    QString currentText = ui->display->text();
    if (!currentText.contains('.')) {
        currentText += '.';
        updateDisplay(currentText);
    }
}

void MainWindow::squareRootClicked()
{
    double value = ui->display->text().toDouble();
    if (value < 0) {
        QMessageBox::warning(this, "Ошибка",
                             "Квадратный корень из отрицательного числа!");
        return;
    }

    double result = std::sqrt(value);
    QString operation = QString("√(%1)").arg(value);
    updateDisplay(QString::number(result));
    addToHistory(operation, result);
    m_leftOperand = result;
}

void MainWindow::powerClicked()
{
    // Сохраняем левый операнд и устанавливаем оператор
    m_leftOperand = ui->display->text().toDouble();
    m_currentOperator = "^";
    m_isNewOperation = true;
}

void MainWindow::sinClicked()
{
    double value = ui->display->text().toDouble();
    double result = std::sin(value * M_PI / 180.0); // перевод в радианы
    QString operation = QString("sin(%1°)").arg(value);
    updateDisplay(QString::number(result));
    addToHistory(operation, result);
    m_leftOperand = result;
}

void MainWindow::cosClicked()
{
    double value = ui->display->text().toDouble();
    double result = std::cos(value * M_PI / 180.0);
    QString operation = QString("cos(%1°)").arg(value);
    updateDisplay(QString::number(result));
    addToHistory(operation, result);
    m_leftOperand = result;
}

void MainWindow::tanClicked()
{
    double value = ui->display->text().toDouble();
    // Проверка на углы, где тангенс не определен
    if (std::fmod(value + 90, 180) == 0) {
        QMessageBox::warning(this, "Ошибка",
                             "Тангенс не определен для этого угла!\n"
                             "Попробуйте углы: 0°, 30°, 45°, 60° и т.д.");
        return;
    }
    double result = std::tan(value * M_PI / 180.0);
    QString operation = QString("tan(%1°)").arg(value);
    updateDisplay(QString::number(result));
    addToHistory(operation, result);
    m_leftOperand = result;
}

void MainWindow::logClicked()
{
    double value = ui->display->text().toDouble();
    if (value <= 0) {
        QMessageBox::warning(this, "Ошибка",
                             "Логарифм определен только для положительных чисел!");
        return;
    }

    double result = std::log10(value);
    QString operation = QString("log(%1)").arg(value);
    updateDisplay(QString::number(result));
    addToHistory(operation, result);
    m_leftOperand = result;
}

void MainWindow::toBinaryClicked()
{
    bool ok;
    QString text = ui->display->text();
    if (text.contains('.')) {
        QMessageBox::warning(this, "Ошибка",
                             "Режим программиста работает только с целыми числами!");
        return;
    }

    int value = text.toInt(&ok);
    if (ok) {
        QString binary = QString::number(value, 2);
        updateDisplay(binary);
        addToHistory(QString("%1 в двоичной").arg(text), value);
    }
}

void MainWindow::toHexClicked()
{
    bool ok;
    QString text = ui->display->text();
    if (text.contains('.')) {
        QMessageBox::warning(this, "Ошибка",
                             "Режим программиста работает только с целыми числами!");
        return;
    }

    int value = text.toInt(&ok);
    if (ok) {
        QString hex = QString::number(value, 16).toUpper();
        updateDisplay(hex);
        addToHistory(QString("%1 в шестнадцатеричной").arg(text), value);
    }
}

void MainWindow::andClicked()
{
    if (!m_currentOperator.isEmpty()) {
        calculate(ui->display->text().toDouble());
    } else {
        m_leftOperand = ui->display->text().toDouble();
        m_currentOperator = "AND";
        m_isNewOperation = true;
    }
}

void MainWindow::orClicked()
{
    if (!m_currentOperator.isEmpty()) {
        calculate(ui->display->text().toDouble());
    } else {
        m_leftOperand = ui->display->text().toDouble();
        m_currentOperator = "OR";
        m_isNewOperation = true;
    }
}

void MainWindow::xorClicked()
{
    if (!m_currentOperator.isEmpty()) {
        calculate(ui->display->text().toDouble());
    } else {
        m_leftOperand = ui->display->text().toDouble();
        m_currentOperator = "XOR";
        m_isNewOperation = true;
    }
}

void MainWindow::addToHistory(const QString &operation, double result)
{
    QString historyEntry = QString("%1 = %2").arg(operation).arg(result);
    ui->historyList->addItem(historyEntry);
    m_historyManager->addEntry(operation, result);
}

void MainWindow::onHistoryItemClicked(QListWidgetItem *item)
{
    if (!item) return;

    QString text = item->text();
    QStringList parts = text.split(" = ");
    if (parts.size() == 2) {
        updateDisplay(parts[1]);
        m_isNewOperation = true;
    }
}

void MainWindow::clearHistoryClicked()
{
    ui->historyList->clear();
    m_historyManager->clearHistory();
}

void MainWindow::saveHistoryClicked()
{
    if (m_historyManager->saveToFile("calculator_history.txt")) {
        QMessageBox::information(this, "Успех",
                                 "История сохранена в файл calculator_history.txt");
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось сохранить историю");
    }
}

void MainWindow::loadHistoryClicked()
{
    if (m_historyManager->loadFromFile("calculator_history.txt")) {
        ui->historyList->clear();
        QStringList history = m_historyManager->getHistory();
        ui->historyList->addItems(history);
        QMessageBox::information(this, "Успех", "История загружена из файла");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить историю");
    }
}

void MainWindow::updateDisplay(const QString &text)
{
    ui->display->setText(text);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Обработка нажатий клавиатуры
    switch (event->key()) {
    case Qt::Key_0: ui->btn0->click(); break;
    case Qt::Key_1: ui->btn1->click(); break;
    case Qt::Key_2: ui->btn2->click(); break;
    case Qt::Key_3: ui->btn3->click(); break;
    case Qt::Key_4: ui->btn4->click(); break;
    case Qt::Key_5: ui->btn5->click(); break;
    case Qt::Key_6: ui->btn6->click(); break;
    case Qt::Key_7: ui->btn7->click(); break;
    case Qt::Key_8: ui->btn8->click(); break;
    case Qt::Key_9: ui->btn9->click(); break;
    case Qt::Key_Plus: ui->btnPlus->click(); break;
    case Qt::Key_Minus: ui->btnMinus->click(); break;
    case Qt::Key_Asterisk: ui->btnMultiply->click(); break;
    case Qt::Key_Slash: ui->btnDivide->click(); break;
    case Qt::Key_Enter:
    case Qt::Key_Return: ui->btnEquals->click(); break;
    case Qt::Key_Escape: ui->btnClear->click(); break;
    case Qt::Key_Backspace: ui->btnBackspace->click(); break;
    case Qt::Key_Period: ui->btnPoint->click(); break;
    case Qt::Key_Percent: ui->btnPercent->click(); break;
    case Qt::Key_S: ui->btnSin->click(); break;
    case Qt::Key_C:
        if (event->modifiers() & Qt::ControlModifier) {
            ui->btnClear->click();
        } else {
            ui->btnCos->click();
        }
        break;
    case Qt::Key_T: ui->btnTan->click(); break;
    case Qt::Key_L: ui->btnLog->click(); break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::onModeChanged(int index)
{
    switch (index) {
    case 0:
        switchMode(CalculatorMode::Standard);
        break;
    case 1:
        switchMode(CalculatorMode::Scientific);
        break;
    case 2:
        switchMode(CalculatorMode::Programmer);
        break;
    }
}

void MainWindow::switchMode(CalculatorMode mode)
{
    m_currentMode = mode;

    // Скрываем все дополнительные группы
    ui->scientificGroup->hide();
    ui->programmerGroup->hide();

    // Показываем нужную группу
    switch (mode) {
    case CalculatorMode::Standard:
        setupStandardMode();
        break;
    case CalculatorMode::Scientific:
        setupScientificMode();
        break;
    case CalculatorMode::Programmer:
        setupProgrammerMode();
        break;
    }
}

void MainWindow::setupStandardMode()
{
    setWindowTitle("Стандартный калькулятор");
    ui->scientificGroup->hide();
    ui->programmerGroup->hide();
}

void MainWindow::setupScientificMode()
{
    setWindowTitle("Научный калькулятор");
    ui->scientificGroup->show();
    ui->programmerGroup->hide();

    // Добавляем подсказки для научных функций
    ui->btnSin->setToolTip("Синус угла (в градусах)");
    ui->btnCos->setToolTip("Косинус угла (в градусах)");
    ui->btnTan->setToolTip("Тангенс угла (в градусах)");
    ui->btnLog->setToolTip("Десятичный логарифм");
}

void MainWindow::setupProgrammerMode()
{
    setWindowTitle("Программистский калькулятор");
    ui->scientificGroup->hide();
    ui->programmerGroup->show();

    // Добавляем подсказки для программистских функций
    ui->btnBin->setToolTip("Перевести в двоичную систему");
    ui->btnHex->setToolTip("Перевести в шестнадцатеричную систему");
    ui->btnAnd->setToolTip("Побитовое И (AND)");
    ui->btnOr->setToolTip("Побитовое ИЛИ (OR)");
    ui->btnXor->setToolTip("Побитовое исключающее ИЛИ (XOR)");
}

void MainWindow::onActionExit()
{
    close();
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this, "О программе",
                       "Расширенный калькулятор\n"
                       "Версия 1.0\n\n"
                       "Возможности:\n"
                       "• Стандартные операции\n"
                       "• Научный режим (sin, cos, tan, log)\n"
                       "• Режим программиста (Bin, Hex, AND, OR, XOR)\n"
                       "• История операций\n"
                       "• Поддержка клавиатуры\n\n"
                       "© 2024");
}
