// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

#pragma  once

namespace Git
{
	class StatusFile: public QObject
	{
		Q_OBJECT
		Q_ENUMS(StausType)
		Q_PROPERTY(QString oldPath READ oldPath CONSTANT)
		Q_PROPERTY(QString newPath READ newPath CONSTANT)
		Q_PROPERTY(StausType type READ statusType CONSTANT)
	public:
		enum StausType
		{
			Current=0,
			WTDeleted,
			New,
			Modified,
			Deleted,
			Renamed,
			TypeChange,
			Untracked,
			Unknown,
		};

		StatusFile(QObject* _parent=nullptr): QObject(_parent) {}
		StatusFile(QObject* _parent,
				   QString _oldPath,
				   QString _newPath,
				   StausType _statusType ): QObject(_parent),
								 m_oldPath(_oldPath),
								 m_newPath(_newPath),
								 m_statusType(_statusType)
		{}
		~StatusFile() {}
		QString oldPath() const { return m_oldPath; }
		QString newPath() const { return m_newPath; }
		StausType statusType() const { return m_statusType; }

	private:
		QString m_oldPath;
		QString m_newPath;
		StausType m_statusType;
	};
}
