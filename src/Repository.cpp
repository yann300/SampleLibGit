// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com
// Author: Ali Mashatan

#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <git2.h>

#include <Repository.h>
#include <Private/RepositoryPrivate.h>

namespace Git
{
//********************** Start private
Repository::RepositoryPrivate::RepositoryPrivate()
{
	m_ptrRepo = NULL;
	// initialize libgit2
	git_libgit2_init();
}

Repository::RepositoryPrivate::~RepositoryPrivate()
{
	// finalize libgit2
	git_libgit2_shutdown();
}

git_commit * Repository::RepositoryPrivate::GetLastCommit()
{
	int error;
	git_commit * commit = NULL;
	git_oid oidParentCommit;
	error = git_reference_name_to_id(&oidParentCommit, m_ptrRepo, "HEAD");
	if (error == 0)
	{
		error = git_commit_lookup(&commit, m_ptrRepo, &oidParentCommit);
		if (error == 0)
			return commit;
	}
	return NULL;
}
//********************** End private


Repository::Repository(QObject* _parent /*= NULL*/)
	:QObject(_parent)
{
	m_repositoryPrivate = new RepositoryPrivate();
}

Repository::~Repository()
{
	delete m_repositoryPrivate;
}

QString Repository::getUsername()
{
	return m_username;
}

QString Repository::getPassword()
{
	return m_password;
}

bool Repository::open(const QString &_repoPath)
{
	int error;
	if (error = git_repository_open(&m_repositoryPrivate->m_ptrRepo, _repoPath.toStdString().c_str()) != 0)
	{
		signalError(error);
		getStausFiles();
		return false;
	}
	m_repoPath = _repoPath;
	getStausFiles();
	return true;
}

void Repository::close()
{
	if (m_repositoryPrivate->m_ptrRepo)
	{
		git_repository_free(m_repositoryPrivate->m_ptrRepo);
		m_repositoryPrivate->m_ptrRepo = NULL;
		m_statusFiles.clear();
		emit updateFileStatus();
	}
}

bool Repository::clone(const QString &_repoPath)
{
	RepositoryPrivate::CommonData commonData;// = { { 0 } };
	commonData.ptrRepo = this;
	commonData.ptrPrivateRepo = m_repositoryPrivate;
	git_clone_options cloneOptions = GIT_CLONE_OPTIONS_INIT;
	git_checkout_options checkoutOptions = GIT_CHECKOUT_OPTIONS_INIT;
	int error;

	// Set up options
	checkoutOptions.checkout_strategy = GIT_CHECKOUT_SAFE;
	checkoutOptions.progress_payload = &commonData;
	cloneOptions.checkout_opts = checkoutOptions;
	cloneOptions.remote_callbacks.transfer_progress = [](const git_transfer_progress *stats, void *payload) -> int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		if (commonData)
		{
			emit commonData->ptrRepo->remoteTransfer(stats->total_objects, stats->indexed_objects,
													  stats->received_objects, stats->received_bytes);
			if (stats->received_objects == stats->total_objects)
				emit commonData->ptrRepo->remoteFinishDownloading();

			if (stats->indexed_objects == stats->total_objects)
				emit commonData->ptrRepo->remoteFinishIndexing();
		}
		return 0;
	};
	cloneOptions.remote_callbacks.credentials = [](git_cred **out, const char * url, const char * username_from_url, unsigned int allowed_types, void * payload) -> int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		emit commonData->ptrRepo->remoteCredential();
		return git_cred_userpass_plaintext_new(out, commonData->ptrRepo->getUsername().toStdString().c_str(), commonData->ptrRepo->getPassword().toStdString().c_str());
	};
	cloneOptions.remote_callbacks.payload = &commonData;
	git_repository * ptrRepo;
	if (error = git_clone(&ptrRepo, m_repoURL.toStdString().c_str(), _repoPath.toStdString().c_str(), &cloneOptions) < 0)
	{
		signalError(error);
		return false;
	}
	close();
	m_repositoryPrivate->m_ptrRepo = ptrRepo;
	m_repoPath = _repoPath;
	getStausFiles();
	return true;
}

void Repository::signalError(qint32 _error, const QString &_hint /*= QString()*/)
{
	const git_error *err = giterr_last();
	if (err)
		emit errorMessage(err->klass, QString(err->message), _hint);
	else
		emit errorMessage(_error, QString(), _hint);
}

bool Repository::addFilenameToRepo(const QString &_filename)
{
	git_index *index;
	int error;

	if (error = git_repository_index(&index, m_repositoryPrivate->m_ptrRepo) < 0)
	{
		signalError(error);
		return false;
	}

	if (error = git_index_add_bypath(index, _filename.toStdString().c_str()))
	{
		signalError(error);
		return false;
	}

	if (error = git_index_write(index) < 0)
	{
		signalError(error);
		return false;
	}

	git_index_free(index);
	return true;
}

bool Repository::removeFilenameFromRepo(const QString &_filename)
{
	git_oid treeID;
	git_index *index;
	int error;

	if (error = git_repository_index(&index, m_repositoryPrivate->m_ptrRepo))
	{
		signalError(error);
		return false;
	}

	if (error = git_index_remove_bypath(index, _filename.toStdString().c_str()))
	{
		signalError(error);
		return false;
	}

	if (error = git_index_write_tree(&treeID, index))
	{
		signalError(error);
		return false;
	}
	git_index_free(index);
	return true;
}

bool Repository::commit(const QString &_commitMessage)
{
	git_index *index;
	git_oid treeID, commitID;
	git_tree *tree;
	git_commit *parent = NULL;
	git_signature *author;
	int error;
	Q_ASSERT(m_author.isEmpty() == false);
	Q_ASSERT(m_email.isEmpty() == false);
	Q_ASSERT(_commitMessage.isEmpty() == false);

	if (!m_repositoryPrivate->m_ptrRepo)
	{
		emit errorMessage(-1, tr("Reposetry is not opened"), QString());
		return false;
	}
	foreach(StatusFile * statusFile, m_statusFiles)
	{
		if (statusFile->addToggle() && statusFile->statusType() == StatusFile::Untracked  )
		{
			addFilenameToRepo(statusFile->newPath());
		}
		if (statusFile->statusType() == StatusFile::Modified )
		{
			addFilenameToRepo(statusFile->oldPath());
		}
		if ((statusFile->removeToggle() && (statusFile->statusType() == StatusFile::Current ||
											statusFile->statusType() == StatusFile::Modified))
			|| statusFile->statusType() == StatusFile::Deleted)
		{
			removeFilenameFromRepo(statusFile->oldPath());
		}
	}

	//ToDo: modification UTC
	git_signature_new((git_signature **)&author,
					  m_author.toStdString().c_str(), m_email.toStdString().c_str(), QDateTime::currentDateTime().toTime_t(), 60);

	if (error = git_repository_index(&index, m_repositoryPrivate->m_ptrRepo))
	{
		signalError(error);
		return false;
	}

	if (error = git_index_write_tree(&treeID, index))
	{
		signalError(error);
		return false;
	}

	git_index_free(index);

	if (error = git_tree_lookup(&tree, m_repositoryPrivate->m_ptrRepo, &treeID))
	{
		signalError(error);
		return false;
	}
	parent = m_repositoryPrivate->GetLastCommit();

	if (error = git_commit_create_v(
				&commitID, m_repositoryPrivate->m_ptrRepo, "HEAD", author, author,
				NULL, _commitMessage.toUtf8(), tree, 1, parent))
	{
		signalError(error);
		return false;
	}

	git_tree_free(tree);
	git_signature_free(author);
	//qDebug() << "finish commit";
	getStausFiles();
	return true;
}


bool Repository::fetch()
{
	RepositoryPrivate::CommonData commonData;// = { { 0 } };
	git_remote *remote = NULL;
	const git_transfer_progress *stats;
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	int error;
	if (error = git_remote_lookup(&remote, m_repositoryPrivate->m_ptrRepo, "origin"))
	{
		signalError(error);
		return false;
	}
	commonData.ptrRepo = this;
	commonData.ptrPrivateRepo = m_repositoryPrivate;
	callbacks.update_tips = [](const char *refname, const git_oid *a, const git_oid *b, void *payload)->int
	{
		Q_UNUSED(refname)
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		char aStr[GIT_OID_HEXSZ + 1], bStr[GIT_OID_HEXSZ + 1];

		git_oid_fmt(bStr, b);
		bStr[GIT_OID_HEXSZ] = '\0';
		QString message;
		if (git_oid_iszero(a))
		{
			emit commonData->ptrRepo->remoteUpdateTips(Repository::upNew, QString(),QString(bStr));
		}
		else {
			git_oid_fmt(aStr, a);
			aStr[GIT_OID_HEXSZ] = '\0';
			emit commonData->ptrRepo->remoteUpdateTips(Repository::upUpdate, QString(bStr), QString(bStr));
		}
		emit commonData->ptrRepo->remoteProgress(message);
		return 0;
	};

	callbacks.sideband_progress = [](const char *str, int len, void *payload) ->int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		commonData->ptrRepo->remoteProgress(QString::fromUtf8(str, len));
		return 0;
	};

	callbacks.credentials = [](git_cred **out, const char * url, const char * username_from_url, unsigned int allowed_types, void * payload) -> int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		emit commonData->ptrRepo->remoteCredential();
		return git_cred_userpass_plaintext_new(out, commonData->ptrRepo->getUsername().toStdString().c_str(), commonData->ptrRepo->getPassword().toStdString().c_str());
	};

	callbacks.payload = &commonData;
	git_remote_set_callbacks(remote, &callbacks);

	stats = git_remote_stats(remote);

	if (error = git_remote_connect(remote, GIT_DIRECTION_FETCH))
	{
		signalError(error);
		return false;
	}

	if (error = git_remote_download(remote, NULL))
	{
		signalError(error);
		return false;
	}

	git_remote_disconnect(remote);

	if (error = git_remote_update_tips(remote, NULL))
	{
		signalError(error);
		return false;
	}

	git_remote_free(remote);
	getStausFiles();
	return true;
}

bool Repository::push()
{
	if (!m_repositoryPrivate->m_ptrRepo)
	{
		emit errorMessage(-1, tr("Reposetry is not opened"), QString());
		return false;
	}

	RepositoryPrivate::CommonData commonData;// = { { 0 } };
	commonData.ptrRepo = this;
	commonData.ptrPrivateRepo = m_repositoryPrivate;
	git_remote *remote = NULL;
	const git_transfer_progress *stats;
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	int error;
	if (error = git_remote_lookup(&remote, m_repositoryPrivate->m_ptrRepo, "origin"))
	{
		signalError(error);
		return false;
	}

	callbacks.update_tips = [](const char *refname, const git_oid *a, const git_oid *b, void *payload)->int
	{
		Q_UNUSED(refname)
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		char aStr[GIT_OID_HEXSZ + 1], bStr[GIT_OID_HEXSZ + 1];

		git_oid_fmt(bStr, b);
		bStr[GIT_OID_HEXSZ] = '\0';
		QString message;
		if (git_oid_iszero(a))
		{
			emit commonData->ptrRepo->remoteUpdateTips(Repository::upNew, QString(),QString(bStr));
		}
		else {
			git_oid_fmt(aStr, a);
			aStr[GIT_OID_HEXSZ] = '\0';
			emit commonData->ptrRepo->remoteUpdateTips(Repository::upUpdate, QString(bStr), QString(bStr));
		}
		emit commonData->ptrRepo->remoteProgress(message);
		return 0;
	};
	callbacks.sideband_progress = [](const char *str, int len, void *payload) ->int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		commonData->ptrRepo->remoteProgress(QString::fromUtf8(str, len));
		return 0;
	};
	callbacks.credentials = [](git_cred **out, const char * url, const char * username_from_url, unsigned int allowed_types, void * payload) -> int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		emit commonData->ptrRepo->remoteCredential();
		return git_cred_userpass_plaintext_new(out, commonData->ptrRepo->getUsername().toStdString().c_str(), commonData->ptrRepo->getPassword().toStdString().c_str());
	};
	callbacks.payload = &commonData;
	git_remote_set_callbacks(remote, &callbacks);

	stats = git_remote_stats(remote);

	if (error = git_remote_connect(remote, GIT_DIRECTION_PUSH))
	{
		signalError(error);
		return false;
	}

	git_push_options options;
	git_push_init_options(&options, GIT_PUSH_OPTIONS_VERSION);

	git_remote_add_push(remote, "refs/heads/master:refs/heads/master");

	if (error = git_remote_upload(remote, NULL, &options))
	{
		signalError(error);
		return false;
	}

	git_remote_disconnect(remote);

	if (error = git_remote_update_tips(remote, NULL))
	{
		signalError(error);
		return false;
	}

	git_remote_free(remote);
	return true;
}

bool Repository::merge()
{
	int error;
	std::vector<git_annotated_commit*> commits;
	RepositoryPrivate::MergeData mergeData;
	mergeData.commits = &commits;
	mergeData.ptrRepo = m_repositoryPrivate->m_ptrRepo;

	git_repository_fetchhead_foreach(m_repositoryPrivate->m_ptrRepo, []
									 (const char *ref_name, const char *remote_url, const git_oid *oid, unsigned int is_merge, void *payload) -> int
	{
		RepositoryPrivate::MergeData *mergeData = (RepositoryPrivate::MergeData*)payload;
		git_annotated_commit* commit;
		int error = git_annotated_commit_from_fetchhead(&commit, mergeData->ptrRepo, "master", remote_url, oid);
		mergeData->commits->push_back(commit);
		return error;
	}, (void*)&mergeData);

	git_merge_options mergopts = GIT_MERGE_OPTIONS_INIT;
	// Favor theirs
	mergopts.file_favor = GIT_MERGE_FILE_FAVOR_THEIRS;
	if (commits.size() > 0)
	{
		error = git_merge(m_repositoryPrivate->m_ptrRepo, (const git_annotated_commit**)&commits[0], commits.size(), NULL, NULL);
	}
	for (auto commit : commits)
	{
		git_annotated_commit_free(commit);
	}
	getStausFiles();
	return true;
}

#if 0
bool Repository::revert()
{
	git_commit *commit = m_repositoryPrivate->GetLastCommit();
	int error;
	if (error = git_revert(m_repositoryPrivate->m_ptrRepo, commit, NULL))
	{
		signalError(error);
		return false;
	}
	return true;
}
#endif

bool Repository::getStausFiles()
{
	//clear list of status
	foreach(StatusFile * statusFile, m_statusFiles)
		delete statusFile;
	m_statusFiles.clear();

	git_status_options statusopt = GIT_STATUS_OPTIONS_INIT;
	statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
			GIT_STATUS_OPT_INCLUDE_UNMODIFIED |
			GIT_STATUS_OPT_UPDATE_INDEX |
			GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS |
			GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
			GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

	int error;
	if (!m_repositoryPrivate->m_ptrRepo)
	{
		emit errorMessage(-1, tr("Reposetry is not opened"), QString());
		return false;
	}
	git_status_list *status;
	if (error = git_status_list_new(&status, m_repositoryPrivate->m_ptrRepo, &statusopt))
	{
		signalError(error);
		return false;
	}
	size_t statusCount = git_status_list_entrycount(status);
	//qDebug() << "C counter : " << statusCount;
	for (int i = 0; i < statusCount; i++)
	{
		const git_status_entry *s = git_status_byindex(status, i);
		StatusFile::StausType statusType = StatusFile::Unknown;
		switch (s->status)
		{
		case GIT_STATUS_CURRENT:
			statusType = StatusFile::Current;
		break;
		case GIT_STATUS_WT_NEW:
			statusType = StatusFile::Untracked;
		break;
		case GIT_STATUS_WT_DELETED:
		case GIT_STATUS_INDEX_DELETED:
			statusType = StatusFile::Deleted;
		break;
		case GIT_STATUS_INDEX_NEW:
			statusType = StatusFile::New;
		break;
		case GIT_STATUS_INDEX_MODIFIED:
		case GIT_STATUS_WT_MODIFIED:
			statusType = StatusFile::Modified;
		break;
		case GIT_STATUS_WT_RENAMED:
		case GIT_STATUS_INDEX_RENAMED:
			statusType = StatusFile::Renamed;
		break;
		case GIT_STATUS_INDEX_TYPECHANGE:
			statusType = StatusFile::TypeChange;
		break;
		default:
			statusType = StatusFile::Unknown;
		}

		if (s->head_to_index)
		{
			const char * newPath = s->head_to_index->new_file.path;
			if ( newPath )
			{
				const char * oldPath = s->head_to_index->old_file.path;
				QString qnewPath = QString(newPath);
				if (oldPath )
				{
					QString qoldPath = QString(oldPath);
					m_statusFiles << new StatusFile(this,
												qoldPath,
												qnewPath,
												statusType);
				}
				else
					m_statusFiles << new StatusFile(this,
													qnewPath,
													qnewPath,
													statusType);
			}
		} else if (	s->index_to_workdir )
		{
			m_statusFiles << new StatusFile(this,
										QString(s->index_to_workdir->old_file.path),
										QString(s->index_to_workdir->old_file.path),
										statusType);
		}
	}//for

	git_status_list_free(status);
	emit updateFileStatus();
	return true;
}

QString Repository::getEmail()
{
	return m_email;
}

QString Repository::getAuthor()
{
	return m_author;
}

QString Repository::getRespPath()
{
	return m_repoPath;
}

QString Repository::getRespURL()
{
	return m_repoURL;
}

void Repository::setUsername(const QString &_value)
{
	m_username = _value;
}

void Repository::setPassword(const QString &_value)
{
	m_password = _value;
}

void Repository::setRepoURL(const QString &_value)
{
	m_repoURL = _value;
}

void Repository::setAuthor(const QString &_value)
{
	m_author = _value;
}

void Repository::setEmail(const QString &_value)
{
	m_email = _value;
}

}
