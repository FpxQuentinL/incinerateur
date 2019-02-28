#include "jeu.h"
#include <iostream>
#include <QDebug>
#include <QCoreApplication>
#include <QJsonObject>
#include <audiocontroller.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <array>
//#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>



int G_OutputDiodeV;
int G_OutputDiodeR;
bool G_DeclenchementInterupt;
int G_Interupt;
std::atomic_int G_CombiValid[9];
std::atomic_int G_CombiReset[9];
std::atomic_int G_CombiCurrent[9];

Jeu::Jeu(IPluginsReport *report) : IPlugins(report)
{

}

bool Jeu::Game_Success()
{
//    QJsonObject play_sound_victory
//    {
//        {"command", "play"},
//        {"recast", false},
//        {"path",  "file:///home/pi/Downloads/test.mp3"}
//    };


//    std::cout << "YOU WIN !!!" << std::endl;
//    Signal_RCon_AudioControler(play_sound_victory);

//    return (true);

}

bool Jeu::Game_Fail()
{
    QJsonObject play_sound
    {
        {"command", "stop"},
        {"id", "file:///home/pi/Downloads/test.mp3"}
    };

    std::cout << "YOU LOOSE !!!" << std::endl;
    Signal_RCon_AudioControler(play_sound);
    return (true);
}

void Jeu::loadConfigGameTech(QString config_file)
{
    QJsonDocument docGameTech;
    QDir dirGameTech;
    qDebug() << dirGameTech.path();
    QFile fileGameTech(config_file);
    if (!fileGameTech.open(QIODevice::ReadOnly | QIODevice::Text))
    {
          qDebug("error FFFIILLLEEEUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU");
          return;
    }
    qDebug() << "Opening config file" << config_file;
    docGameTech = QJsonDocument::fromJson(fileGameTech.readAll());
    QJsonObject object = docGameTech.object();
    qDebug("\n\n###\t start json file\t###");

    G_OutputDiodeV = object.value("ControlerLedV").toString().toInt();
    G_OutputDiodeR = object.value("ControlerLedR").toString().toInt();
    qDebug()<<G_OutputDiodeV ;
    qDebug()<<G_OutputDiodeR ;

    G_Interupt = object.value("Interupt").toString().toInt();
    qDebug("###\t end json file###\n\n");

}
void Jeu::loadConfigGameFonct(QString config_file)
{
    QJsonDocument docGame;
    QDir dirGame;
    qDebug() << dirGame.path();
    QFile fileGame(config_file);
    if (!fileGame.open(QIODevice::ReadOnly | QIODevice::Text))
    {
          qDebug("error FFFIILLLEEEUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU");
          return;
    }
    qDebug() << "Opening config file" << config_file;

    docGame = QJsonDocument::fromJson(fileGame.readAll());
    QJsonObject object = docGame.object();
    qDebug("\n\n###\t start json file\t###");


    QJsonArray array = object["List_ID"].toArray();
    qDebug("\n\n###\t start json file\t###");
    int i=0;
    for(i=0;i<array.size();i++)
    {
       G_CombiValid[i]=array.at(i).toString().toInt();
    }
    QJsonArray arrayReset=object["List_Reset"].toArray();
    for(i=0;i<arrayReset.size();i++)
    {
       G_CombiReset[i]=arrayReset.at(i).toString().toInt();
    }
    _AudioPathInterupt = object.value("AudioPathI").toString();
    _AudioPathAmbiance = object.value("AudioPathA").toString();
    qDebug()<<_AudioPathAmbiance<<_AudioPathInterupt;



    qDebug("###\t end json file###\n\n");

}
void Jeu::InteruptGeneral()
{
    QTime test=QTime::currentTime().addMSecs(250);

    while (QTime::currentTime()<test){
        if (!digitalRead(G_Interupt)){
            qDebug()<<"je suis de l'electromagnetisme";
            return;
        }
    }

    qDebug()<<"interupt";
    G_DeclenchementInterupt=true;
}


void Jeu::Setup()
{
    wiringPiSetupGpio();
    G_DeclenchementInterupt=false;

    loadConfigGameFonct("/home/pi/Dev/CodeSequenceur/test_du_soir/Code_brief/Generik/GameFonct.json");
    loadConfigGameTech("/home/pi/Dev/CodeSequenceur/test_du_soir/Code_brief/Generik/GameTech.json");

    pinMode(G_OutputDiodeV,OUTPUT);
    pinMode(G_OutputDiodeR,OUTPUT);

 qDebug()<< G_OutputDiodeV;
    digitalWrite(G_OutputDiodeR,LOW);
    digitalWrite(G_OutputDiodeV,HIGH);

    if(wiringPiISR(G_Interupt,INT_EDGE_RISING, &Jeu::InteruptGeneral)<0)
        qDebug("ERRRROOOOORRRRRR");
    QJsonObject AudioAmbiance;
    AudioAmbiance.insert("command","play");
    AudioAmbiance.insert("recast",true);
    AudioAmbiance.insert("path",_AudioPathAmbiance);
    qDebug()<< AudioAmbiance;
    Signal_RCon_AudioControler(AudioAmbiance);


}

bool Jeu::Game_Socketio()
{
    QJsonObject packet
    {
        {"io_socket", "iterate_and_pulse"},
        {"command", "{\"color\":\"0xFAFAFA\",\"delay\":\"1\",\"brightness_max\":\"255\",\"brightness_min\":\"32\",\"brightness_scale\":\"5\"pos_start\":\"0\",\"pos_end\":\"144\",\"scale\":\"i\"}"}
    };

    std::cout << "YOU Socket iO !!!" << std::endl;
    Signal_RCon_NetworkIO(packet);
    return (true);
}

void Jeu::Slot_RCon( QJsonObject packet )
{
    //qDebug()<<">IPlugins::Slot_RCon<"<< "\n" << packet;
    if (packet.contains("command"))
    {
        if (packet.value("command")=="Combi")
        {
           QJsonArray array =packet["List"].toArray();
           int i = 0;
           for (i=0;i<array.size();i++)
           {
            G_CombiCurrent[i]=array.at(i).toInt();
           // qDebug()<<G_CombiCurrent[i];
           }


        }

    }
    //GameFlow
}


void Jeu::main_game_callback()
{
 if(G_DeclenchementInterupt){
     qDebug()<<"test";
     G_DeclenchementInterupt=false;
     QJsonObject AudioInterput;
     AudioInterput.insert("command","play");
     AudioInterput.insert("path",_AudioPathInterupt);
     bool valid = true;
     bool validreset = true;
     int i = 0;
     for (i=0;i<9;i++)
     {
         if(G_CombiCurrent[i]!=G_CombiValid[i])
         {
             valid = false;
         }
         if(G_CombiCurrent[i]!=G_CombiReset[1])
         {
             validreset = false;
         }
     }

     qDebug()<<valid;
     if (valid)
     {
         Signal_RCon_AudioControler(AudioInterput);
         digitalWrite(G_OutputDiodeV, LOW);
         digitalWrite(G_OutputDiodeR, HIGH);
     }
     else if (validreset)
     {

          digitalWrite(G_OutputDiodeR,LOW);
          digitalWrite(G_OutputDiodeV,HIGH);
     }





 }
}
