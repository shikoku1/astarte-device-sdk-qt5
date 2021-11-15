/*
 * This file is part of Astarte.
 *
 * Copyright 2017 Ispirata Srl
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QTextStream>
 #include <QDateTime>
#include <QtCore/QDir>
 #include <QObject>
#include <HemeraCore/Operation>
#include <AstarteDeviceSDK.h>

#include <ValidateInterfaceOperation.h>
#include <unistd.h>


AstarteDeviceSDK *sdk;

void checkInitResult(Hemera::Operation *op) {
  qDebug() << "-------------- ok";

  sdk->sendData("com.test.Everything", "/double", 3.3, QDateTime::currentDateTime());
  sdk->sendData("com.test.Everything", "/integer", 3, QDateTime::currentDateTime());
  sdk->sendData("com.test.Everything", "/boolean", false, QDateTime::currentDateTime());



  //sdk->sendData("com.test.Everything", "/boolean", var, QDateTime::currentDateTime());
  qDebug() << "-------------- ok2";

  while (1) {
      qDebug() << "-------------- loop";

      sdk->sendData("com.test.Everything", "/double", 3.3, QDateTime::currentDateTime());

      {
        QList<int> list = { 1, 2, 3};
        sdk->sendData("com.test.Everything", "/integerarray", list, QDateTime::currentDateTime());
      }
      {
        QList<bool> list = { true, false, true};
        sdk->sendData("com.test.Everything", "/booleanarray", list, QDateTime::currentDateTime());
      }
      {
        QList<double> list = { 4.5, 4.6, 4.7};
        sdk->sendData("com.test.Everything", "/doublearray", list, QDateTime::currentDateTime());
      }

      {
        qDebug() << "expect an error";
        QList<double> list = { 4.5, 4.6, 4.7};
        sdk->sendData("com.test.Everything", "/double", list, QDateTime::currentDateTime());
      }
      {
        qDebug() << "expect an error";
        QList<QString> list = { QStringLiteral("fsd"), QStringLiteral("gfdgfd"), QStringLiteral("hbgf")};
        sdk->sendData("com.test.Everything", "/datetimearray", list, QDateTime::currentDateTime());
      }
      sleep(1);
  }
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);


    sdk = new AstarteDeviceSDK(QDir::currentPath() + QStringLiteral("/transport-astarte.conf"), QDir::currentPath()+ QStringLiteral("/interfaces"),
                               "u-WraCwtT_G_fjJf63TiAw");


    QObject::connect(sdk->init(), &Hemera::Operation::finished, checkInitResult);



    return app.exec();
}
