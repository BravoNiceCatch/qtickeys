#include "mainwindow.h"
#include <QKeySequence>
#include <functional>
#include <future>
#include "ui_mainwindow.h"
#include "x11keyhook.h"

using namespace std;

void MainWindow::PrintPressKey(unsigned int keycode) {
    QString keycodestr = QVariant(keycode).toString();
    this->ui->txbKeycode->setText(keycodestr);
    QString wavFile = this->BuildWavFile(keycode);
    qDebug() << "sound :" << wavFile << endl;
    QString fileInfo = QFileInfo(wavFile).absoluteFilePath();
    qDebug() << "soud :" << this->ui->hsliderSound->value() << "volumn:" << player->volume() << endl;

    player->setVolume(this->ui->hsliderSound->value());
    player->setMedia(QUrl::fromLocalFile(fileInfo));
    player->play();

    CheckShowWindow(keycode);
}

QString MainWindow::BuildWavFile(unsigned int keycode) {
    SoundScheme* current = CurrentScheme();
    unsigned int index   = keycode % static_cast<unsigned int>(current->NonuniqueCount);
    QString keycodestr   = QVariant(keycode).toString();
    if (current->KeyAudioMap.contains(keycodestr)) {
        index = static_cast<unsigned int>(current->KeyAudioMap[keycodestr].toInt());
    }

    QString file = "data/" + current->Path[static_cast<int>(index)] + ".wav";
    return file;
}

void MainWindow::RunBack(KeyboardHooker* keyboardHooker) {
    printf("start thread.\n");

    keyboardHooker->Run([=](unsigned int a) { this->PrintPressKey(a); });
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      player(new QMediaPlayer),
      keyboardHooker(new KeyboardHooker),
      config(new SoundConfig) {
    ui->setupUi(this);

    int style  = this->config->QtickeysConf->Style;
    int volumn = this->config->QtickeysConf->Volumn;

    std::thread t(&MainWindow::RunBack, this, keyboardHooker);
    t.detach();

    for (auto& scheme : config->SoundSchemes) {
        ui->cboSchemes->addItem(scheme->Name);
    }

    this->ui->cboSchemes->setCurrentIndex(style);
    this->ui->hsliderSound->setValue(volumn);
}

MainWindow::~MainWindow() {
    delete ui;
    delete player;
    delete keyboardHooker;
    delete config;
}

void MainWindow::on_cboSchemes_currentIndexChanged(int index) {
    this->config->QtickeysConf->Style = index;
    this->config->SaveConfig();
}

SoundScheme* MainWindow::CurrentScheme() { return this->config->SoundSchemes[this->config->QtickeysConf->Style]; }

void MainWindow::on_hsliderSound_valueChanged(int value) {
    this->config->QtickeysConf->Volumn = value;
    this->config->SaveConfig();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (ensureExit) {
        keyboardHooker->Stop();
        event->accept();
        qDebug("exit qtickeys.");
    } else {
        this->hide();
        event->ignore();
        qDebug("prevent close & minimize.");
        this->setWindowState(Qt::WindowMinimized);
    }
}

void MainWindow::changeEvent(QEvent* event) {
    event->accept();
    if (event->type() == QEvent::WindowStateChange) {
        if (windowState() == Qt::WindowMinimized) {
            this->hide();
        } else if (windowState() == Qt::WindowNoState) {
        }
    }
}

void MainWindow::CheckShowWindow(uint keycode) {
    if (keycode == inputKeyList.last()) {
        return;
    }
    int inputKeyListLength = inputKeyList.length();
    uint nextKeyCode       = hotKeyList[inputKeyListLength];
    uint nextKeyCode2      = hotKeyList2[inputKeyListLength];
    if (keycode == nextKeyCode || keycode == nextKeyCode2) {
        inputKeyList.append(keycode);
        if (inputKeyListLength == 5) {
            ShowGui();
            inputKeyList.clear();
        }
    } else {
        inputKeyList.clear();
    }
}

void MainWindow::ShowGui() {
    this->show();
    qDebug("qaz123 to show gui.");
}

void MainWindow::on_btnExit_clicked() {
    this->ensureExit = true;

    this->close();
}
