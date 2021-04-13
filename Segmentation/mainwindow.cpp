#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QPainter>
#include <QColor>
#include <QDir>
#include <QTextStream>
#include <omp.h>
#include <QCursor>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFileDialog>
#include <QtCore/qmath.h>

MainWindow::MainWindow(QWidget *parent) :
                                          QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // Работа с директориями и путями
  currentPath = QDir::currentPath();                    // Директория исполняемого файла
  currentPath.replace("/", "\\\\");                     // Заменить символы "/" на "\\"

  bmpPath = "\\\\bmp\\\\images\\\\";                    // Путь до папки со статичными изображениями относительно исполняемого файла
  bmpPath.prepend(currentPath);                         // Полный путь до папки со статичными изображениями
  gifPath = "\\\\bmp\\\\gif\\\\";                       // Путь до папки с gif относительно исполняемого файла
  gifPath.prepend(currentPath);                         // Полное имя папки с gif
  savePath = "\\\\bmp\\\\save\\\\";                     // Путь до папки с сохранениями относительно исполняемого файла
  savePath.prepend(currentPath);                        // Полное имя папки с сохранениями

  strcpy(currentPath_char, currentPath.toUtf8().data()); // Переписать currentPath (QString) в currentPath_char (char) (рудимент)

  loadingData();                                         // Совершим первую подгрузку

  // Реализуем анимацию с частотой 10 кадров в секунду
  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(timeSlot())); // Цепляем таймер к слоту
  timer->start(100);                                         // Запуск

  // Начальные установки
  ui->spinBox_bmpName->setValue(4);       //
  ui->spinBox->setValue(256);             //
  ui->spinBox_multiplicity->setValue(1);  //
  on_checkBox_animation_stateChanged(0);  // Анимация выключена

  qApp->installEventFilter(this); // Ловим все события
}

MainWindow::~MainWindow()
{
  delete ui;
}



// Все события приходят сюда
 #pragma warning(default:4716)
bool MainWindow::eventFilter(QObject *watched, QEvent *event){

  // Глобальные координаты окна
  xGCW = (this)->geometry().x(); // Координаты left-top главного окна приложения
  yGCW = (this)->geometry().y(); // Координаты left-top главного окна приложения

  // Отслеживаем события мыши
  if(event->type() == QEvent::MouseMove){
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    xC = mouseEvent->x();        // Координаты курсора на теле виджета
    yC = mouseEvent->y();        // Координаты курсора на теле виджета
    xGC = mouseEvent->globalX(); // Глобальные координаты курсора
    yGC = mouseEvent->globalY(); // Глобальные координаты курсора
  }

  str = "";

  // Вывод координат и цвета выбранного пикселя
  if((xGC >= xGCW+185 && xGC < xGCW+185+256 && yGC >= yGCW+10 && yGC < yGCW+10+256) ||
     (xGC > xGCW+445 && xGC <= xGCW+445+256 && yGC >= yGCW+10 && yGC < yGCW+10+256))
  {
    str = "(" + QString::number(xC+1)   + "," + QString::number(yC+1)   + ")";
  }

  // Вывод координат окна при наведении на top-left угол
  if(xGC > xGCW && xGC < xGCW+10 && yGC > yGCW && yGC < yGCW+10){
    str = "Координаты окна (" + QString::number(xGCW) + "," + QString::number(yGCW) + ") " ;
  }

  ui->label_str->setText(str); // Вывод данных

  QObject::eventFilter(watched, event);

}




// Обработка нажатия кнопки (Действие)
void MainWindow::on_pushButton_clicked(){
  loadingData(); // Загрузка данных в ОЗУ

}



// Сохранить изображение
void MainWindow::on_pushButton_save_clicked(){
  numberSave = (int)QDir(savePath).count()-2;     // Количество сохранённых изображений
  QString format = "bmp";
  QString initialPath = savePath + QString::number(numberSave+1);
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                  initialPath,
                                                  tr("%1 Files (*.%2);;All Files (*)")
                                                      .arg(format.toUpper())
                                                      .arg(format));
  if (!fileName.isEmpty())
    ptes.save(fileName);
}



// Анимация
void MainWindow::timeSlot(){
  // Если запущена анимация, но не нажата кнопка стоп
  if((ui->checkBox_animation->isChecked()) && !(ui->checkBox_stop->isChecked())){
    num = num+1;                                   // Пробегаем по кадрам gif
    ui->spinBox_bmpName->setValue(num);            // Устанавливаем новый кадр
    if(num == numberFrames) num = 0;               // Обнулить, если обработан последний кадр gif
  }
  // Если запущена анимация и нажата кнопка стоп
  if((ui->checkBox_animation->isChecked()) && (ui->checkBox_stop->isChecked())){
    num = ui->spinBox_bmpName->value();            // Номер кадра равен устанновленному в spinBox_bmpName
    ui->spinBox_bmpName->setValue(num);
  }
}





// Получаем матрицу пикселей
void MainWindow::matrixPixels(int par){

  // Выбор изображение из памяти
  //image; // Сюда загрузим изображение
  // При статическом изображении
  if(!(ui->checkBox_animation->isChecked())){
    image = imgImg[ui->spinBox_bmpName->value()-1];
  }
  // При анимации
  if(ui->checkBox_animation->isChecked()){
    image = imgGif[ui->spinBox_bmpName->value()-1];
  }

  // Установка интервалов
  bev[0] = ui->spinBox_bev_1->value();
  bev[1] = ui->spinBox_bev_2->value();
  bev[2] = ui->spinBox_bev_3->value();
  bev[3] = ui->spinBox_bev_4->value();
  bev[4] = ui->spinBox_bev_5->value();
  bev[5] = ui->spinBox_bev_6->value();
  bev[6] = ui->spinBox_bev_7->value();
  bev[7] = ui->spinBox_bev_8->value();

  // Установка делений цветов согласно выбранного скоса (тут можно не разбираться)
  black_str = ""; red_str = ""; orange_str = ""; yellow_str = ""; green_str = ""; heavenly_str = ""; blue_str = ""; purple_str = "";
  if(0      < par   ) { black_str    = QString::number(0         ) + " - " + QString::number(par         ); }
  if(bev[0] < bev[1]) { red_str      = QString::number(par+bev[0]) + " - " + QString::number(par+bev[1]-1); }
  if(bev[1] < bev[2]) { orange_str   = QString::number(par+bev[1]) + " - " + QString::number(par+bev[2]-1); }
  if(bev[2] < bev[3]) { yellow_str   = QString::number(par+bev[2]) + " - " + QString::number(par+bev[3]-1); }
  if(bev[3] < bev[4]) { green_str    = QString::number(par+bev[3]) + " - " + QString::number(par+bev[4]-1); }
  if(bev[4] < bev[5]) { heavenly_str = QString::number(par+bev[4]) + " - " + QString::number(par+bev[5]-1); }
  if(bev[5] < bev[6]) { blue_str     = QString::number(par+bev[5]) + " - " + QString::number(par+bev[6]-1); }
  if(bev[6] < bev[7]) { purple_str   = QString::number(par+bev[6]) + " - " + QString::number(par+bev[7]-1); }

  w = image.width();  // Ширина изображения
  h = image.height(); // Высота изображения

  // Отрисовка оригинала изображения
  QPixmap pix_original(w,h);          // создаём пустой QPixmap размером с исходную картинку
  QColor pxColor_original;            // Хранит цвет
  QPainter p_original(&pix_original); // Создаём объект отрисовщика

  // Отрисовка обработанного изображения
  QPixmap pix(w,h);       // создаём пустой QPixmap размером с исходную картинку
  QColor pxColor;         // Хранит цвет
  QPainter p(&pix);       // Создаём объект отрисовщика


    for(int i = 0; i < w; i++){
      for(int j = 0; j < h; j++){
        // Устанавливаем значения субпикселей
        rgb = image.pixel(i, j);      // QImage image;  // Хранит изображение формата bmp
        red[i][j] = qRed( rgb );
        green[i][j] = qGreen( rgb );
        blue[i][j] = qBlue( rgb );

        // Определение яркости каждого пикселя
        powerPixel[i][j] = (red[i][j]+green[i][j]+blue[i][j])/3;

        // Отрисовка оригинального пикселя
        pxColor_original.setRed(red[i][j]);      // Устанавливаем красный
        pxColor_original.setGreen(green[i][j]);  // Устанавливаем зелёный
        pxColor_original.setBlue(blue[i][j]);    // Устанавливаем синий
        p_original.setPen(pxColor_original);     // Устанавливаем цвет
        p_original.drawPoint(i,j);               // Закрашиваем пиксель

        // Обработка пикселя (par - значение указанное в поле Скос)
        if(powerPixel[i][j] <  par)                                         { red[i][j] = 0;   green[i][j] = 0;   blue[i][j] = 0;   } // Ч
        if(powerPixel[i][j] >= par+bev[0] && powerPixel[i][j] < par+bev[1]) { red[i][j] = 255; green[i][j] = 0;   blue[i][j] = 0;   } // К
        if(powerPixel[i][j] >= par+bev[1] && powerPixel[i][j] < par+bev[2]) { red[i][j] = 255; green[i][j] = 127; blue[i][j] = 0;   } // О
        if(powerPixel[i][j] >= par+bev[2] && powerPixel[i][j] < par+bev[3]) { red[i][j] = 255; green[i][j] = 255; blue[i][j] = 0;   } // Ж
        if(powerPixel[i][j] >= par+bev[3] && powerPixel[i][j] < par+bev[4]) { red[i][j] = 0;   green[i][j] = 255; blue[i][j] = 0;   } // З
        if(powerPixel[i][j] >= par+bev[4] && powerPixel[i][j] < par+bev[5]) { red[i][j] = 115; green[i][j] = 185; blue[i][j] = 255; } // Г
        if(powerPixel[i][j] >= par+bev[5] && powerPixel[i][j] < par+bev[6]) { red[i][j] = 0;   green[i][j] = 0;   blue[i][j] = 130; } // С
        if(powerPixel[i][j] >= par+bev[6] && powerPixel[i][j] < par+bev[7]) { red[i][j] = 127; green[i][j] = 0;   blue[i][j] = 255; } // Ф

        // Отрисовка обработанного пикселя
        pxColor.setRed(red[i][j]);      // Устанавливаем красный
        pxColor.setGreen(green[i][j]);  // Устанавливаем зелёный
        pxColor.setBlue(blue[i][j]);    // Устанавливаем синий
        p.setPen(pxColor);              // Устанавливаем цвет
        p.drawPoint(i,j);               // Закрашиваем пиксель

      }
    }

  // Сглаживание
  smoKof = qPow(2,ui->spinBox_smoothing->value()); // Коэффициент сглаживания
  if(smoKof > 1) {
    for(int i = 0; i < w; i=i+smoKof){
      for(int j = 0; j < h; j=j+smoKof){
        aveRed = 0; aveGreen = 0; aveBlue = 0; // Обнулить средние значения цвета
        for(int k = 0; k < smoKof; k++){
          for(int f = 0; f < smoKof; f++){
            aveRed = aveRed + red[i+k][j+f];
            aveGreen = aveGreen + green[i+k][j+f];
            aveBlue = aveBlue + blue[i+k][j+f];
          }
        }
        aveRed = aveRed / qPow(smoKof,2);
        aveGreen = aveGreen / qPow(smoKof,2);
        aveBlue = aveBlue / qPow(smoKof,2);
        for(int k = 0; k < smoKof; k++){
          for(int f = 0; f < smoKof; f++){
            red[i+k][j+f] = aveRed;
            green[i+k][j+f] = aveGreen;
            blue[i+k][j+f] = aveBlue;
          }
        }
      }
    }
    for(int i = 0; i < w; i++){
      for(int j = 0; j < h; j++){
        // Отрисовка пикселя
        pxColor.setRed(red[i][j]);      // Устанавливаем красный
        pxColor.setGreen(green[i][j]);  // Устанавливаем зелёный
        pxColor.setBlue(blue[i][j]);    // Устанавливаем синий
        p.setPen(pxColor);              // Устанавливаем цвет
        p.drawPoint(i,j);               // Закрашиваем пиксель
      }
    }
  }

  ptes = pix; // На случай если будем сохранять изображение



  ui->label_original->setGeometry(190+w,10,w,h); // Установка размеров и отступов label_original
  ui->label_pix->setGeometry(185,10,w,h);        // Установка размеров и отступов label_pix
  ui->label_original->setPixmap(pix_original);   // Устанавливаем в label_original изображение
  ui->label_pix->setPixmap(pix);                 // Устанавливаем в label_pix изображение

  ui->label_w->setText(QString::number(w)); // Вывод ширины изображения
  ui->label_h->setText(QString::number(h)); // Вывод высоты изображения

  // Визуализация интервалов цветов
  ui->label_black->setText(black_str);
  ui->label_red->setText(red_str);
  ui->label_orange->setText(orange_str);
  ui->label_yellow->setText(yellow_str);
  ui->label_green->setText(green_str);
  ui->label_heavenly->setText(heavenly_str);
  ui->label_blue->setText(blue_str);
  ui->label_purple->setText(purple_str);


}



// Изменение значения spinBox
void MainWindow::on_spinBox_valueChanged(int arg1){
  // if(arg1 == 17000000){ arg1 = 0; ui->spinBox->setValue(0); } // Зацикливание spinBox
  matrixPixels(arg1);
}
// Установка кратности для основного spinBox
void MainWindow::on_spinBox_multiplicity_valueChanged(int arg1){
  ui->spinBox->setSingleStep(arg1);
}

// Смена изображения
void MainWindow::on_spinBox_bmpName_valueChanged(){
  matrixPixels(ui->spinBox->value());
}




// Обработка нажатий анимации
void MainWindow::on_checkBox_animation_stateChanged(int arg1){
  // Если анимацию отключили
  if(arg1 == 0){
    ui->spinBox_bmpName->setValue(4);
    ui->spinBox_bmpName->setMaximum(numberImages); // Установить предел для spinBox_bmpName в кол-во изображений в каталоге
    ui->spinBox_bmpName->setMinimum(1);            // Установить предел для spinBox_bmpName в кол-во изображений в каталоге
    ui->checkBox_stop->setChecked(0);
  }
  // Если анимацию включили
  if(arg1 == 2){
    ui->spinBox_bmpName->setMaximum(numberFrames); // Установить предел для spinBox_bmpName в кол-во кадров gif
    ui->spinBox_bmpName->setMinimum(1);            // Установить предел для spinBox_bmpName в кол-во кадров gif
  }
}



// Загрузка данных в ОЗУ из ПЗУ
void MainWindow::loadingData(){
  numberFrames = (int)QDir(gifPath).count()-2;    // Количество кадров gif
  numberImages = (int)QDir(bmpPath).count()-2;    // Количество статичных изображений
  numberSave = (int)QDir(savePath).count()-2;     // Количество сохранённых изображений
  ui->label_numberFrames->setText(QString::number(numberFrames)); // Вывод данных
  ui->label_numberImages->setText(QString::number(numberImages)); // Вывод данных

  // Загрузка кадров гифки в ОЗУ
  QString temporary;
  for(int i = 0; i < numberFrames; i++){
    temporary = gifPath;
    temporary.append(QString::number(i+1));
    temporary.append(".bmp");
    imgGif[i].load(temporary);
  }
  // Загрузка статичных изображений в ОЗУ
  for(int i = 0; i < numberImages; i++){
    temporary = bmpPath;
    temporary.append(QString::number(i+1));
    temporary.append(".bmp");
    imgImg[i].load(temporary);
  }
}



// Изменение значения частоты кадров
void MainWindow::on_spinBox_time_valueChanged(int arg1){
  timer->start(arg1);
}



// Изменена степень сглаживания
void MainWindow::on_spinBox_smoothing_valueChanged(){
    matrixPixels(ui->spinBox->value());
}
