#define APP_VERSION "1.0.0"
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QGraphicsRectItem>
#include <cstdlib>
#include <QMouseEvent>
#include <QGraphicsItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , timer(new QTimer(this))
    , gridSize(10) // Начальный размер сетки
    , lifeTime(0) // Время жизни
    , gameFinished(true) // Определение, завершена игра или нет
{
    ui->setupUi(this);
    ui->graphicsView->setScene(scene);

    // Настройка таймера
    connect(timer, &QTimer::timeout, this, &MainWindow::updateGrid);
    timer->setInterval(500);

    // Убрать скролл бары
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Установка минимального и максимального значения ползунка размера сетки
    ui->gridSize->setMinimum(10);
    ui->gridSize->setMaximum(60);

    connect(ui->gridSize, &QSlider::valueChanged, ui->gridSizeCount, QOverload<int>::of(&QLCDNumber::display));
    connect(ui->gridSize, &QSlider::valueChanged, this, &MainWindow::on_gridSizeChanged);

    //Кнопки
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::handleStartButtonClick);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::on_resetButton_clicked);
    connect(ui->randomButton, &QPushButton::clicked, this, &MainWindow::on_randomButton_clicked);

    initializeGrid();
    drawGrid();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete scene;
    delete timer;
}

void MainWindow::adjustView() {
    // Подгонка содержимого под размер окна
    QRectF sceneRect = scene->itemsBoundingRect();
    ui->graphicsView->setSceneRect(sceneRect);
    ui->graphicsView->fitInView(sceneRect, Qt::KeepAspectRatio);
}

void MainWindow::on_resetButton_clicked() {
    if (timer->isActive()) {
        timer->stop(); // Останавливаем таймер, если он запущен
        ui->startButton->setText("Старт"); // Меняем текст кнопки на "Старт"
    }

    clearGrid();
    initializeGrid();
    drawGrid();
}

void MainWindow::on_randomButton_clicked() {
    if (timer->isActive()) {
        timer->stop(); // Останавливаем таймер, если он запущен
        ui->startButton->setText("Старт"); // Меняем текст кнопки на "Старт"
    }

    clearGrid();
    initializeGrid();
    randomizeGrid();
    drawGrid();
}

void MainWindow::handleStartButtonClick() {
    qDebug() << "Start button clicked";
    qDebug() << "Timer active before:" << timer->isActive();

    if (timer->isActive()) {
        timer->stop();
        ui->startButton->setText("Старт");
        qDebug() << "Timer stopped";
    } else {
        // Проверяем, была ли игра завершена до этого
        if (gameFinished) {
            lifeTime=0;
            ui->lifeCount->display(lifeTime);
            gameFinished=false;
        }
        // Проверяем, пуста ли сетка
        if (isGridEmpty()) {
            // Если сетка пуста, эмитируем нажатие кнопки random
            on_randomButton_clicked();
        }
        timer->start();
        ui->startButton->setText("Пауза");
        qDebug() << "Timer started";
    }

    qDebug() << "Timer active after:" << timer->isActive();
}


void MainWindow::on_gridSizeChanged(int value) {
    gameFinished=true;
    timer->stop();
    ui->startButton->setText("Старт");
    clearGrid(); //Очищаем сетку
    gridSize = qBound(10,value,60);      // Обновляем размер сетки
    initializeGrid();      // Переинициализируем сетку
    drawGrid();            // Перерисовываем сетку
}

void MainWindow::initializeGrid() {
    grid.resize(gridSize);  // Правильная инициализация размера
    nextGrid.resize(gridSize);  // Инициализация второго массива
    for (int i = 0; i < gridSize; ++i) {
        grid[i].resize(gridSize);  // Каждый элемент вектора имеет нужный размер
        nextGrid[i].resize(gridSize);  // Тот же размер для следующего поколения
    }
}


void MainWindow::randomizeGrid() {
    srand(time(0)); // Рандом
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            grid[i][j] = (rand() % 2 == 0); // Случайно 0 или 1
        }
    }
}

void MainWindow::clearGrid() {
    scene->clear();
    grid.clear();
    nextGrid.clear();
    lifeTime=0;
    ui->lifeCount->display(lifeTime);
}

void MainWindow::drawGrid() {
    qDebug() << "DrawGrid called";
    scene->clear();

    int viewWidth = ui->graphicsView->width();
    int viewHeight = ui->graphicsView->height();
    qreal cellWidth = static_cast<qreal>(viewWidth) / gridSize;
    qreal cellHeight = static_cast<qreal>(viewHeight) / gridSize;

    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            QRectF rect(i * cellWidth, j * cellHeight, cellWidth, cellHeight);
            QGraphicsRectItem *cell = scene->addRect(rect);

            if (grid[i][j]) {
                cell->setBrush(Qt::black);
            } else {
                cell->setBrush(Qt::white);
            }

            cell->setPen(QPen(Qt::gray));
        }
    }

    adjustView();
}


int MainWindow::countAliveNeighbors(int x, int y) {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;

            int nx = (x + dx + gridSize) % gridSize;
            int ny = (y + dy + gridSize) % gridSize;

            if (grid[nx][ny]) {
                count++;
                qDebug() << "Alive neighbor at:" << nx << ny;
            }
        }
    }
    return count;
}

void MainWindow::updateGrid() {
    previousGrid = grid;
    qDebug() << "Timer tick";

    bool gridChanged = false;

    // Проходим по всем клеткам и обновляем их состояния
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            int aliveNeighbors = countAliveNeighbors(i, j);
            bool nextState = (grid[i][j] ? (aliveNeighbors == 2 || aliveNeighbors == 3) : (aliveNeighbors == 3));
            if (grid[i][j] != nextState) {
                nextGrid[i][j] = nextState;
                gridChanged = true; // Клетка изменила состояние
            } else {
                nextGrid[i][j] = grid[i][j]; // Клетка не изменилась
            }
        }
    }

    // Если сетка изменилась, обновляем
    if (grid != nextGrid) {
        grid = nextGrid;
        drawGrid();  // Перерисовываем сетку
        lifeTime++;  // Обновляем время жизни
        ui->lifeCount->display(lifeTime);  // Обновляем отображение времени жизни
    }

    // Проверка на стабильность или пустоту сетки
    if (isStable() or isGridEmpty()) {
        gameFinished=true;
        timer->stop();
        ui->startButton->setText("Старт");
        if (isStable()) {
            QString message = QString("Игра завершена, конфигурация стабильна\nВремя жизни: %1").arg(lifeTime);
            QMessageBox::information(this, "Stable", message);
            qDebug() << "Game finished. Stable";
        } else {
            QString message = QString("Игра завершена, нет ни одной живой клетки\nВремя жизни: %1").arg(lifeTime);
            QMessageBox::information(this, "Empty", message);
            qDebug() << "Game finished. GridEmpty";
        }
    }
}



void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event); // Вызов базового метода
    drawGrid(); // Отрисовка сетки после отображения окна
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event); // Вызов стандартного обработчика события
    drawGrid(); // Перерисовка сетки при изменении размера окна
}

bool MainWindow::isStable() {
    return previousGrid == grid;
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    // Получаем координаты клика в сцене
    QPointF scenePos = ui->graphicsView->mapToScene(event->pos());

    // Получаем размеры клетки, основываясь на актуальных размерах сцены
    qreal cellWidth = static_cast<qreal>(ui->graphicsView->width()) / gridSize;
    qreal cellHeight = static_cast<qreal>(ui->graphicsView->height()) / gridSize;

    // Определяем индексы клетки, на которую был произведен клик
    int x = static_cast<int>(scenePos.x() / cellWidth);
    int y = static_cast<int>(scenePos.y() / cellHeight);

    // Проверяем, что индексы находятся в пределах сетки
    if (x >= 0 && x < gridSize && y >= 0 && y < gridSize) {
        // Переключаем состояние клетки (живая/мертвая)
        grid[x][y] = !grid[x][y];

        // Перерисовываем клетку
        drawGrid();
    }

    // Важно вызвать метод базового класса для правильной обработки событий
    QMainWindow::mousePressEvent(event);
}

bool MainWindow::isGridEmpty() {
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            if (grid[i][j]) {
                return false; // Сетка не пуста, есть хотя бы одна живая клетка
            }
        }
    }
    return true; // Сетка пуста
}
