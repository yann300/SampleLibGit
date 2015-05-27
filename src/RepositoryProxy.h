// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

#pragma  once

#include <QQmlListProperty>
#include <Repository.h>
#include <QDebug>

namespace Git
{
	class RepositoryProxy : public Repository
	{
		Q_OBJECT
		Q_PROPERTY(QQmlListProperty<Git::StatusFile> statusFiles READ statusFiles CONSTANT)

	public:
		RepositoryProxy(QObject* _parent = NULL);
		virtual ~RepositoryProxy();
		Q_INVOKABLE void setEmail(QString const& _email)
		{
			Repository::setEmail(_email);
		}
		Q_INVOKABLE void setAuthor(QString const& _author)
		{
			Repository::setAuthor(_author);
		}
		Q_INVOKABLE void setURL(QString const& _url)
		{
			Repository::setRepoURL(_url);
		}
		Q_INVOKABLE bool open(QString const& _path)
		{
			bool result = Repository::open(_path);
			return result;
		}
		Q_INVOKABLE void close()
		{
			Repository::close();
		}
		Q_INVOKABLE bool clone(QString const& _path)
		{
			return Repository::clone(_path);
		}
		Q_INVOKABLE bool commit(QString const& _commitMessage)
		{
			return Repository::commit(_commitMessage);
		}
		Q_INVOKABLE void fetch()
		{
			Repository::fetch();
		}
		Q_INVOKABLE void push()
		{
			Repository::push();
		}
		Q_INVOKABLE void merge()
		{
			Repository::merge();
		}
		Q_INVOKABLE bool getStausFiles()
		{
			return Repository::getStausFiles();
		}
		Q_INVOKABLE void setAddToggle(QString _filename, bool _toggle)
		{
			foreach(StatusFile *statusFile, m_statusFiles)
			{
				if (statusFile->newPath() == _filename)
				{
					statusFile->setAddToggle(_toggle);
				}
			}
		}
		Q_INVOKABLE void setRemoveToggle(QString _filename, bool _toggle)
		{
			foreach(StatusFile *statusFile, m_statusFiles)
			{
				if (statusFile->oldPath() == _filename)
				{
					statusFile->setRemoveToggle(_toggle);
				}
			}
		}
		QQmlListProperty<StatusFile> statusFiles() const
		{
			//qDebug() << m_statusFiles.count();
			return QQmlListProperty<StatusFile>(const_cast<RepositoryProxy*>(this), const_cast<RepositoryProxy*>(this)->m_statusFiles);
		}

	private:
	};
}
