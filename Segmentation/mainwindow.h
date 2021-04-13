#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

namespace Ui
{
class MainWindow;
}

class MainWindow :
    public QMainWindow
{
  Q_OBJECT

public: explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  // Работа с директориями
  QString currentPath;         // Хранит полное имя каталога исполняемого файла
  char currentPath_char[256];  //
  QString bmp;                 // Хранит полное имя обрабатываемого изображения
  char bmp_char[256];          //
  QString gifPath;             // Хранит полное имя каталога с gif
  QString bmpPath;             // Хранит полное имя каталога со статичными изображениями
  QString savePath;            // Хранит полное имя каталога c сохранениями

  QTimer *timer;        // QTimer реализуем анимацию
  //QTimer *loadingTimer; // Реализуем подгрузку данных
  QTimer *time;         // Время анимации
  int numberImages;     // Количество статичных изображений
  int numberFrames;     // Количество кадров gif
  int numberSave;       // Количество сохранённых изображений
  int num = 0;          // Номер текущего кадра gif

  QString str;  // Строка вывода

  int xC, yC, xGC, yGC, xGCW, yGCW; // Координаты

  QImage imgGif[256], imgImg[256];
  QImage image; // Сюда загрузим изображение

  QRgb rgb;

  QString black_str;
  QString red_str;
  QString orange_str;
  QString yellow_str;
  QString green_str;
  QString heavenly_str;
  QString blue_str;
  QString purple_str;

  QPixmap ptes; // Сохраняемое изображение

  int w; // Ширина изображения
  int h; // Высота изображения

  int red[256][256];          // Матрица красных субпикселей
  int green[256][256];        // Матрица зелёных субпикселей
  int blue[256][256];         // Матрица синих субпикселей
  int powerPixel[256][256];   // Матрица яркости пикселей
  int bev[8];

  int smoKof; // Коэффициент сглаживания

  int aveRed, aveGreen, aveBlue;

  private slots:
  void matrixPixels(int par);                          // Создание матрицы пиксилей (подзабил на процедурное программирование и сделал всё в одной функции)
  void on_pushButton_clicked();                        // Действие
  void on_spinBox_valueChanged(int arg1);              //
  void on_spinBox_multiplicity_valueChanged(int arg1); //
  void timeSlot();                                     // Слот выполняется с определённой переодичностью
  void loadingData();                                  // Реализует загрузку данных в программу каждые 10 секунд
  void on_spinBox_bmpName_valueChanged();
  void on_checkBox_animation_stateChanged(int arg1);
  void on_spinBox_time_valueChanged(int arg1);
  void on_pushButton_save_clicked();                   // Сохранение изображения
  void on_spinBox_smoothing_valueChanged();            // Изменена степень сглаживания

protected:
  bool eventFilter(QObject *watched, QEvent *event);   // Ловим события

  private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
