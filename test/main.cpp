// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

#include <QtTest/QtTest>
#include <GitClientTest.h>

int main(int argc, char ** argv)
{
	QCoreApplication::setOrganizationName("Mashatan");
	QCoreApplication::setOrganizationDomain("github.com/mashatan");
	QCoreApplication::setApplicationName("SampleGitLib");

	QCoreApplication app(argc, argv);

	GitClientTest gitClientTest;

	QTest::qExec(&gitClientTest, argc, argv);

	app.exec();
	return 0;
}