// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

#include <QApplication>
#include <QQuickView>
#include <RepositoryProxy.h>
#include <QQmlEngine>
#include <QtQml>

using namespace Git;

int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	qDebug()<< "Start...";
	qmlRegisterType<RepositoryProxy>("Git.Repository", 1, 0, "Repository");
	qmlRegisterType<StatusFile>("Git.StatusFile", 1, 0, "StatusFile");
	QQuickView view;
	view.setSource(QUrl("qrc:///qml/main.qml"));
	view.show();
	return app.exec();
}
