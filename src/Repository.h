// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

#pragma  once

#include <QString>
#include <QObject>
#include <QStringList>

#include <StatusFile.h>

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

		Repository(QObject* _parent = NULL);
		virtual ~Repository();
		/// Return email of the user
		QString getUsername();
		/// Return email of the user
		QString getPassword();
		/// Return email of the user
		QString getEmail();
		/// Return author
		QString getAuthor();
		/// Return repository path
		QString getRespPath();
		/// Return repository URL
		QString getRespURL();

		void setUsername(const QString &_value);
		void setPassword(const QString &_value);
		void setEmail(const QString &_value);
		void setAuthor(const QString &_value);
		void setRepoURL(const QString &_value);

		/// Open repository by path
		bool open(const QString &_repoPath);
		/// Clone repository in a new path
		bool clone(const QString &_repoPath);
		/// Close current repository
		void close();

		/// Adding a filename to current branch
		bool addFilenameToRepo(const QString &_filename);
		/// Remove a filename to current branch
		bool removeFilenameFromRepo(const QString &_filename);
		/// Commit the files with a message
		bool commit(const QString &_commitMessage);
		/// Receiving latest the changes from git server (whitout merge)
		bool fetch();
		/// Update remote refs
		bool push();
		/// Merge the retrieved branch heads into current branch
		bool merge();
#if 0
		bool revert();
#endif
		bool getStausFiles();

	signals:
		void errorMessage(qint32 _error, const QString &_message, const QString &_hint);
		void remoteCredential();
		void remoteTransfer(quint32 _totalObjects,
			quint32 _indexedObjects,
			quint32 _receivedObjects,
			quint64 _receivedBytes);
		void remoteProgress(const QString &_message);
		void remoteFinishDownloading();
		void remoteFinishIndexing();
		void remoteUpdateTips(Git::Repository::UpdateTips _updatetips, const QString &_from, const QString &_to);
		void updateFileStatus();

	protected:
		QList<StatusFile*> m_statusFiles;

	private:

		class RepositoryPrivate;
		RepositoryPrivate *m_repositoryPrivate;

		QString m_username;
		QString m_password;
		QString m_email;
		QString m_author;
		QString m_repoPath;
		QString m_repoURL;

		void signalError(qint32 _error, const QString &_hint = QString());
	};
}
Q_DECLARE_METATYPE(Git::Repository::UpdateTips)
