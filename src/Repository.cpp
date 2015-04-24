// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com
// Author: Ali Mashatan

#include <QDateTime>
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


Repository::Repository()
{
	m_repositoryPrivate = new RepositoryPrivate();
}

Repository::~Repository()
{
	delete m_repositoryPrivate;
}

bool Repository::Open(const QString &_repoPath)
{
	int error;
	if (error = git_repository_open(&m_repositoryPrivate->m_ptrRepo, _repoPath.toStdString().c_str()) != 0)
	{
		SignalError(error);
		return false;
	}
	m_repoPath = _repoPath;
	return true;
}

void Repository::Close()
{
	if (m_repositoryPrivate->m_ptrRepo)
	{
		git_repository_free(m_repositoryPrivate->m_ptrRepo);
		m_repositoryPrivate->m_ptrRepo = NULL;
	}
}

bool Repository::Clone(QString &_repoPath)
{
	RepositoryPrivate::CommonData commonData = { { 0 } };
	commonData.ptrRepo = this;
	commonData.ptrPrivateRepo = m_repositoryPrivate;
	Close();
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
			emit commonData->ptrRepo->onRemoteTransfer(stats->total_objects, stats->indexed_objects,
													  stats->received_objects, stats->received_bytes);
			if (stats->received_objects == stats->total_objects)
				emit commonData->ptrRepo->onRemoteFinishDownloading();

			if (stats->indexed_objects == stats->total_objects)
				emit commonData->ptrRepo->onRemoteFinishIndexing();
		}
		return 0;
	};
	cloneOptions.remote_callbacks.credentials = [](git_cred **out, const char * url, const char * username_from_url, unsigned int allowed_types, void * payload) -> int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		QString username;
		QString password;
		emit commonData->ptrRepo->onRemoteCredential(username, password);
		return git_cred_userpass_plaintext_new(out, username.toStdString().c_str(), password.toStdString().c_str());
	};
	cloneOptions.remote_callbacks.payload = &commonData;

	if (error = git_clone(&m_repositoryPrivate->m_ptrRepo, m_repoURL.toStdString().c_str(), _repoPath.toStdString().c_str(), &cloneOptions) < 0)
	{
		SignalError(error);
		return false;
	}
	m_repoPath = _repoPath;
	return true;
}

void Repository::SignalError(qint32 _error, QString &_hint /*= QString()*/)
{
	const git_error *err = giterr_last();
	if (err)
		emit onError(err->klass, QString(err->message), _hint);
	else
		emit onError(_error, QString(), _hint);
}

bool Repository::AddFilenameToRepo(const QString &_filename)
{
	git_index *index;
	int error;

	if (error = git_repository_index(&index, m_repositoryPrivate->m_ptrRepo) < 0)
	{
		SignalError(error);
		return false;
	}

	if (error = git_index_add_bypath(index, _filename.toStdString().c_str()))
	{
		SignalError(error);
		return false;
	}

	if (error = git_index_write(index) < 0)
	{
		SignalError(error);
		return false;
	}

	git_index_free(index);
	return true;
}

bool Repository::RemoveFilenameFromRepo(const QString &_filename)
{
	git_oid treeID;
	git_index *index;
	int error;

	if (error = git_repository_index(&index, m_repositoryPrivate->m_ptrRepo))
	{
		SignalError(error);
		return false;
	}

	if (error = git_index_remove_bypath(index, _filename.toStdString().c_str()))
	{
		SignalError(error);
		return false;
	}

	if (error = git_index_write_tree(&treeID, index))
	{
		SignalError(error);
		return false;
	}
	git_index_free(index);
	return true;
}

bool Repository::Commit(QString &_commitMessage)
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

	//ToDo: modification UTC
	git_signature_new((git_signature **)&author,
					  m_author.toStdString().c_str(), m_email.toStdString().c_str(), QDateTime::currentDateTime().toTime_t(), 60);

	if (error = git_repository_index(&index, m_repositoryPrivate->m_ptrRepo))
	{
		SignalError(error);
		return false;
	}

	if (error = git_index_write_tree(&treeID, index))
	{
		SignalError(error);
		return false;
	}

	git_index_free(index);

	if (error = git_tree_lookup(&tree, m_repositoryPrivate->m_ptrRepo, &treeID))
	{
		SignalError(error);
		return false;
	}
	parent = m_repositoryPrivate->GetLastCommit();

	if (error = git_commit_create_v(
				&commitID, m_repositoryPrivate->m_ptrRepo, "HEAD", author, author,
				NULL, _commitMessage.toUtf8(), tree, 1, parent))
	{
		SignalError(error);
		return false;
	}

	git_tree_free(tree);
	git_signature_free(author);

	return true;
}


bool Repository::Fetch()
{
	RepositoryPrivate::CommonData commonData = { { 0 } };
	git_remote *remote = NULL;
	const git_transfer_progress *stats;
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	int error;
	if (error = git_remote_lookup(&remote, m_repositoryPrivate->m_ptrRepo, "origin"))
	{
		SignalError(error);
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
			emit commonData->ptrRepo->onRemoteUpdateTips(Repository::upNew, QString(),QString(bStr));
		}
		else {
			git_oid_fmt(aStr, a);
			aStr[GIT_OID_HEXSZ] = '\0';
			emit commonData->ptrRepo->onRemoteUpdateTips(Repository::upUpdate, QString(bStr), QString(bStr));
		}
		emit commonData->ptrRepo->onRemoteProgress(message);
		return 0;
	};

	callbacks.sideband_progress = [](const char *str, int len, void *payload) ->int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		commonData->ptrRepo->onRemoteProgress(QString::fromUtf8(str, len));
		return 0;
	};

	callbacks.credentials = [](git_cred **out, const char * url, const char * username_from_url, unsigned int allowed_types, void * payload) -> int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		QString username;
		QString password;
		emit commonData->ptrRepo->onRemoteCredential(username, password);
		return git_cred_userpass_plaintext_new(out, username.toStdString().c_str(), password.toStdString().c_str());
	};

	callbacks.payload = &commonData;
	git_remote_set_callbacks(remote, &callbacks);

	stats = git_remote_stats(remote);

	if (error = git_remote_connect(remote, GIT_DIRECTION_FETCH))
	{
		SignalError(error);
		return false;
	}

	if (error = git_remote_download(remote, NULL))
	{
		SignalError(error);
		return false;
	}

	git_remote_disconnect(remote);

	if (error = git_remote_update_tips(remote, NULL))
	{
		SignalError(error);
		return false;
	}

	git_remote_free(remote);
	return true;
}

bool Repository::Push()
{
	RepositoryPrivate::CommonData commonData = { { 0 } };
	commonData.ptrRepo = this;
	commonData.ptrPrivateRepo = m_repositoryPrivate;
	git_remote *remote = NULL;
	const git_transfer_progress *stats;
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	int error;
	if (error = git_remote_lookup(&remote, m_repositoryPrivate->m_ptrRepo, "origin"))
	{
		SignalError(error);
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
			emit commonData->ptrRepo->onRemoteUpdateTips(Repository::upNew, QString(),QString(bStr));
		}
		else {
			git_oid_fmt(aStr, a);
			aStr[GIT_OID_HEXSZ] = '\0';
			emit commonData->ptrRepo->onRemoteUpdateTips(Repository::upUpdate, QString(bStr), QString(bStr));
		}
		emit commonData->ptrRepo->onRemoteProgress(message);
		return 0;
	};
	callbacks.sideband_progress = [](const char *str, int len, void *payload) ->int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		commonData->ptrRepo->onRemoteProgress(QString::fromUtf8(str, len));
		return 0;
	};
	callbacks.credentials = [](git_cred **out, const char * url, const char * username_from_url, unsigned int allowed_types, void * payload) -> int
	{
		RepositoryPrivate::CommonData *commonData = (RepositoryPrivate::CommonData*)payload;
		QString username;
		QString password;
		emit commonData->ptrRepo->onRemoteCredential(username, password);
		return git_cred_userpass_plaintext_new(out, username.toStdString().c_str(), password.toStdString().c_str());
	};
	callbacks.payload = &commonData;
	git_remote_set_callbacks(remote, &callbacks);

	stats = git_remote_stats(remote);

	if (error = git_remote_connect(remote, GIT_DIRECTION_PUSH))
	{
		SignalError(error);
		return false;
	}

	git_push_options options;
	git_push_init_options(&options, GIT_PUSH_OPTIONS_VERSION);

	git_remote_add_push(remote, "refs/heads/master:refs/heads/master");

	if (error = git_remote_upload(remote, NULL, &options))
	{
		SignalError(error);
		return false;
	}

	git_remote_disconnect(remote);

	if (error = git_remote_update_tips(remote, NULL))
	{
		SignalError(error);
		return false;
	}

	git_remote_free(remote);
	return true;
}

bool Repository::Merge()
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
	return true;
}

bool Repository::Revert()
{
	git_commit *commit = m_repositoryPrivate->GetLastCommit();
	int error;
	if (error = git_revert(m_repositoryPrivate->m_ptrRepo, commit, NULL))
	{
		SignalError(error);
		return false;
	}
	return true;
}

QString Repository::GetEmail()
{
	return m_email;
}

QString Repository::GetAuthor()
{
	return m_author;
}

QString Repository::GetRespPath()
{
	return m_repoPath;
}

QString Repository::GetRespURL()
{
	return m_repoURL;
}

void Repository::SetRespURL(const QString &_value)
{
	m_repoURL = _value;
}

void Repository::SetAuthor(const QString &_value)
{
	m_author = _value;
}

void Repository::SetEmail(const QString &_value)
{
	m_email = _value;
}

}
