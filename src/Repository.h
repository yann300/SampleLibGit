// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

#pragma  once

#include <git2.h>
#include <QString>
#include <QObject>
#include <QStringList>

namespace Git
{
	class Repository : public QObject
	{
		Q_OBJECT
	public:
		enum UpdateTips
		{
			upNew, 
			upUpdate,
		};

		Repository();
		virtual ~Repository();
		/// Return email of the user
		QString GetEmail();
		/// Return author 
		QString GetAuthor();
		/// Return repository path
		QString GetRespPath();
		/// Return repository URL
		QString GetRespURL();

		void SetEmail(const QString &_value);
		void SetAuthor(const QString &_value);
		void SetRespURL(const QString &_value);

		/// Open repository by path
		bool Open(const QString &_repoPath);
		/// Clone repository in a new path
		bool Clone(QString &_repoPath);
		/// Close current repository
		void Close();

		/// Adding a filename to current branch
		bool AddFilenameToRepo(const QString &_filename);
		/// Remove a filename to current branch
		bool RemoveFilenameFromRepo(const QString &_filename);
		/// Commit the files with a message
		bool Commit(QString &_commitMessage);
		/// Receiving latest the changes from git server (whitout merge)
		bool Fetch();
		/// Update remote refs
		bool Push();
		/// Merge the retrieved branch heads into current branch
		bool Merge();
		bool Revert();

	signals:
		void onError(qint32 error, const QString &message, const QString &hint);
		void onRemoteCredential(QString& username, QString& password);
		void onRemoteTransfer(quint32 totalObjects,
			quint32 indexedObjects,
			quint32 receivedObjects,
			quint64 receivedBytes);
		void onRemoteProgress(const QString &message);
		void onRemoteFinishDownloading();
		void onRemoteFinishIndexing();
		void onRemoteUpdateTips(Git::Repository::UpdateTips updatetips, const QString &from, const QString &to);

	private:
		
		class RepositoryPrivate;
		RepositoryPrivate *m_repositoryPrivate;
		
		QString m_email;
		QString m_author;
		QString m_repoPath;
		QString m_repoURL;

		void SignalError(qint32 _error, QString &_hint = QString());
	};
}
Q_DECLARE_METATYPE(Git::Repository::UpdateTips)
