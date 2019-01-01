#include "soundconfig.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

SoundConfig::SoundConfig()
    : QtickeysConf(new StyleConf()),
      schemeConfig(new QFile("data/sound.json")),
      conf(new QFile(QDir::homePath() + "/.qtickeys/qtickeys.conf")) {
    ReadSchemeConfig();
    ReadConfig();
}

SoundConfig::~SoundConfig() {
    delete QtickeysConf;
    delete conf;
    delete schemeConfig;
    for (auto *p : SoundSchemes) delete p;
    SoundSchemes.clear();
}

void SoundConfig::ReadSchemeConfig() {
    if (!schemeConfig->exists() || !schemeConfig->open(QIODevice::ReadOnly)) {
        qWarning("can't open sound.json.");
        return;
    }
    QByteArray configByteArray = schemeConfig->readAll();
    QJsonDocument doc(QJsonDocument::fromJson(configByteArray));

    QJsonObject configObject = doc.object();

    QJsonObject typewriterObject = configObject["typewriter1"].toArray().at(0).toObject();
    QJsonObject bubbleObject     = configObject["bubbles2"].toArray().at(0).toObject();
    QJsonObject mechanicalObject = configObject["mechanical4"].toArray().at(0).toObject();
    QJsonObject swordObject      = configObject["sword1"].toArray().at(0).toObject();
    QJsonObject pianoObject      = configObject["piano3"].toArray().at(0).toObject();

    ReadSchemeConfig(typewriterObject);
    ReadSchemeConfig(bubbleObject);
    ReadSchemeConfig(mechanicalObject);
    ReadSchemeConfig(swordObject);
    ReadSchemeConfig(pianoObject);

    schemeConfig->close();
}

void SoundConfig::ReadConfig() {
    if (!QDir(QDir::homePath() + "/.qtickeys").exists()) {
        QDir().mkdir(QDir::homePath() + "/.qtickeys");
    }

    if (!conf->exists() || !conf->open(QIODevice::ReadOnly)) {
        qWarning("can't open tickey.conf");
        return;
    }

    QByteArray confByteArray = conf->readAll();
    QJsonDocument doc(QJsonDocument(QJsonDocument::fromJson(confByteArray)));
    QJsonObject confObject = doc.object();
    QtickeysConf->Style    = confObject["style"].toInt();
    QtickeysConf->Volumn   = confObject["volumn"].toInt();

    conf->close();
}

void SoundConfig::SaveConfig() {
    if (!conf->open(QIODevice::WriteOnly)) {
        qWarning("can't open(write)  tickey.conf");
        return;
    }
    QJsonObject styleConfObject;
    styleConfObject["style"]  = QtickeysConf->Style;
    styleConfObject["volumn"] = QtickeysConf->Volumn;
    QJsonDocument saveDoc(styleConfObject);
    conf->write(saveDoc.toJson());
    conf->close();
}

void SoundConfig::ReadSchemeConfig(QJsonObject &scheme) {
    SoundScheme *s    = new SoundScheme();
    s->Name           = scheme["name"].toString();
    s->NonuniqueCount = scheme["non_unique_count"].toInt();
    for (const auto p : scheme["path"].toArray()) {
        s->Path.push_back(p.toString().replace("\\", "/"));
    }
    s->KeyAudioMap = scheme["key_audio_map"].toObject();
    SoundSchemes.push_back(s);
}
