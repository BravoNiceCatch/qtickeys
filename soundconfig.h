#ifndef SOUNDCONFIG_H
#define SOUNDCONFIG_H

#include <QFile>
#include <QJsonObject>
#include <QVector>

class StyleConf {
 public:
    int Style  = 3;
    int Volumn = 70;
};

class SoundScheme {
 public:
    QString Name;
    int NonuniqueCount;
    QVector<QString> Path;
    QJsonObject KeyAudioMap;
};

class SoundConfig {
 public:
    SoundConfig();
    ~SoundConfig();
    QVector<SoundScheme *> SoundSchemes;
    StyleConf *QtickeysConf;
    void SaveConfig();

 private:
    QFile *schemeConfig;
    QFile *conf;
    void ReadSchemeConfig();
    void ReadSchemeConfig(QJsonObject &object);
    void ReadConfig();
};

#endif  // SOUNDCONFIG_H
