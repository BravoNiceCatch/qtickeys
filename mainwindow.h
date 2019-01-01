#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QObject>
#include <QTextStream>
#include "x11keyhook.h"

#include "soundconfig.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

 public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void PrintPressKey(unsigned int keycode);

 private slots:
    void on_cboSchemes_currentIndexChanged(int index);
    void on_hsliderSound_valueChanged(int value);

    void on_btnExit_clicked();

 private:
    bool ensureExit = false;
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    KeyboardHooker *keyboardHooker;
    void RunBack(KeyboardHooker *keyboardHooker);
    QString BuildWavFile(unsigned int keycode);
    SoundConfig *config;
    SoundScheme *CurrentScheme();

    void CheckShowWindow(unsigned int keycode);
    uint hotKeyList[6]  = {24, 38, 52, 10, 11, 12};
    uint hotKeyList2[6] = {24, 38, 52, 87, 88, 89};

    QVector<uint> inputKeyList;
    void ShowGui();

 protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
};

#endif  // MAINWINDOW_H
