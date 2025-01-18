#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleStartButtonClick(); //Кнопка запуска/остановки симуляции
    void on_resetButton_clicked(); //Кнопка сброса
    void on_gridSizeChanged(int value); // Изменяет размер сетки через слайдер
    void on_randomButton_clicked();  // Случайные клетки

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override; // Добавляем обработчик события показа
    void mousePressEvent(QMouseEvent *event) override; // Обработчик кликов

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;  // Графическая сцена для отображения игры
    QTimer *timer;          // Таймер для обновления игры
    int gridSize;           // Текущий размер сетки
    QVector<QVector<int>> grid; // Состояние
    QVector<QVector<int>> previousGrid; // Прошлое поколение сетки
    QVector<QVector<int>> nextGrid; // Следующее поколение сетки

    void initializeGrid();  // Инициализация сетки
    void drawGrid();        // Отрисовка сетки в graphicsView
    void clearGrid();       // Очистка сетки
    void updateGrid();      // Переход к следующему поколению
    int countAliveNeighbors(int x, int y); // Подсчет соседей клетки
    void randomizeGrid();   // Случайная инициализация клеток
    void adjustView();
    int lifeTime;
    bool isStable();        // Проверка на стабильность
    bool isGridEmpty();
    bool hasStabilized;
    bool gameFinished;
};
#endif // MAINWINDOW_H
