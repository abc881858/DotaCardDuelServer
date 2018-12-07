#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QWebSocketServer>
#include <QWebSocket>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    server = new QWebSocketServer(QString("Card Server"), QWebSocketServer::NonSecureMode, this);
    server->listen(QHostAddress::Any, 7720);
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    //  connect(server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::write(QWebSocket* socket, QJsonObject jsonObject)
{
    QJsonDocument jsonDoucment(jsonObject);
    QByteArray json = jsonDoucment.toJson(QJsonDocument::Compact);
    socket->sendBinaryMessage(json);
}

void MainWindow::newConnection()
{
    qDebug() << "a client socket connected...";
    QWebSocket* newSock = server->nextPendingConnection();

    if (clients.count() == 0)
    {
        newSock->setObjectName("firstPlayer");
        connect(newSock, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(readFromFirstClient(QByteArray)));

        connect(newSock, &QWebSocket::disconnected, [=]() {
            clients.removeOne(newSock);
            if(clients.isEmpty())
            {
                connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
            }
        });

        clients.append(newSock);
        qDebug() << "current clients count: " << clients.count();
    }
    else if (clients.count() == 1)
    {
        newSock->setObjectName("secondPlayer");
        connect(newSock, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(readFromSecondClient(QByteArray)));

        connect(newSock, &QWebSocket::disconnected, [=]() {
            clients.removeOne(newSock);
            if(clients.isEmpty())
            {
                connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
            }
        });

        clients.append(newSock);
        qDebug() << "current clients count: " << clients.count();
        qDebug() << "room is full, now we disconnect newConnection slot";
        disconnect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
        qDebug() << "the two players are ready, we start the game. First in, first go!";
    }
}

void MainWindow::readFromFirstClient(QByteArray byteArray)
{
    QJsonDocument jsonDoucment = QJsonDocument::fromJson(byteArray);
    QJsonObject json = jsonDoucment.object();
    qDebug() << "readFromFirstClient: " << json;

    if (clients.count() == 1)
    {
        qDebug() << "waiting for second player...";
        return;
    }

    //toInt默认值是0，所以就算没有command，也会返回0
    if (json["command"].toInt() == 2000)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemySetupDeck");
        QJsonObject parameter;
        parameter.insert("array", json["array"].toArray());
        jsonObject.insert("parameter", parameter);
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 3000)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyStartGame");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 10001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyDrawPhase");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 20001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyStandbyPhase");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 30001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyMain1Phase");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 40001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyBattlePhase");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 50001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyMain2Phase");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 60001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyEndPhase");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 70001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "drawPhase");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 10002)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "standbyPhase");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 20002)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "main1Phase");
        write(clients[1], jsonObject);
    }
    else if (json["command"].toInt() == 60002)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "endPhase");
        write(clients[1], jsonObject);
    }
    else
    {
        clients[1]->sendBinaryMessage(byteArray);
    }
}

void MainWindow::readFromSecondClient(QByteArray byteArray)
{
    QJsonDocument jsonDoucment = QJsonDocument::fromJson(byteArray);
    QJsonObject json = jsonDoucment.object();
    qDebug() << "readFromSecondClient: " << json;

    //只有 client2 才会处理 command 1000
    //表示 client 全部连接完毕，请求服务器指示
    if (json["command"].toInt() == 1000)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "setupDeck");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 2001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "startGame");
        QJsonObject parameter;
        parameter.insert("array", json["array"].toArray());
        jsonObject.insert("parameter", parameter);
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 3000)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "drawPhase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 10001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyDrawPhase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 20001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyStandbyPhase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 30001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyMain1Phase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 40001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyBattlePhase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 50001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyMain2Phase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 60001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "enemyEndPhase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 70001)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "drawPhase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 10002)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "standbyPhase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 20002)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "main1Phase");
        write(clients[0], jsonObject);
    }
    else if (json["command"].toInt() == 60002)
    {
        QJsonObject jsonObject;
        jsonObject.insert("request", "endPhase");
        write(clients[0], jsonObject);
    }
    else
    {
        clients[0]->sendBinaryMessage(byteArray);
    }
}
