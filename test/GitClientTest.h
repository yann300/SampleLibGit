// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

#pragma	 once

#include <QtTest/QtTest>
#include <Repository.h>

class GitClientTest : public QObject
{
	Q_OBJECT
private slots:
	void Clone()
	{

		Git::Repository repository;
		bool connectResult;
		connectResult = connect(&repository, SIGNAL(onError(qint32, const QString &, const QString &)), SLOT(doError(qint32, const QString &, const QString &)));
		Q_ASSERT(connectResult);
		connectResult = connect(&repository, SIGNAL(onRemoteCredential(QString&, QString &)), SLOT(doRemoteCredential(QString&, QString&)));
		Q_ASSERT(connectResult);
		connectResult = connect(&repository, SIGNAL(onRemoteTransfer(quint32, quint32, quint32, quint64)), SLOT(doRemoteTransfer(quint32, quint32, quint32, quint64)));
		Q_ASSERT(connectResult);
		connectResult = connect(&repository, SIGNAL(onRemoteFinishDownloading()), SLOT(doRemoteFinishDownloading()));
		Q_ASSERT(connectResult);
		connectResult = connect(&repository, SIGNAL(onRemoteFinishIndexing()), SLOT(doRemoteFinishIndexing()));
		Q_ASSERT(connectResult);
		connectResult = connect(&repository, SIGNAL(onRemoteUpdateTips(Git::Repository::UpdateTips, const QString &, const QString &)),
												 SLOT(doRemoteUpdateTips(Git::Repository::UpdateTips, const QString &, const QString &)));
		Q_ASSERT(connectResult);

		repository.SetRespURL(QString("https://github.com/Mashatan/CustomCombobox"));
		repository.SetAuthor("Ali Mashatan");
		repository.SetEmail("ali@ethdev.com");
		
		system("rmdir /s /q c:\\temp\\test");
		QVERIFY(repository.Clone(QString("c:\\temp\\test")));

		//system("echo \"this is a test\" >> c:\\temp\\test\\gpl01.txt");
		
		//QVERIFY(repository.AddFilenameToRepo(QString("gpl01.txt")));
		//QVERIFY(repository.Commit(QString("Adding gpl01")));
		
		//QVERIFY(repository.Push());
		//QVERIFY(gitClient.Revert());
		//QVERIFY(gitClient.Fetch());

		repository.Close();
	  
	}

private slots:
	void doError(qint32 error, const QString &message, const QString &hint)
	{
		printf("Error: [%d] %s, hint: %s\n", error, message.toStdString().c_str(), hint.toStdString().c_str());
	}
	void doRemoteCredential(QString& username, QString& password) 
	{
	}
	void doRemoteTransfer(quint32 totalObjects,
		quint32 indexedObjects,
		quint32 receivedObjects,
		quint64 receivedBytes)
   {
		printf("Progress %d/%d\n", receivedObjects, totalObjects);
	}
	void doRemoteFinishDownloading()
	{
		printf("Finished downloading.\n");
	}
	void doRemoteFinishIndexing()
	{
		printf("Finished indexing.\n");
	}
	void doRemoteUpdateTips(Git::Repository::UpdateTips updateTips, const QString &from, const QString &to)
	{
		printf("%s	%s ... %s", updateTips == Git::Repository::upNew ? "New":"Update", from.toStdString(), to.toStdString());
	}
private:
};
