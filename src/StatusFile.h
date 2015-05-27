// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

#pragma  once
#include <QDebug>

namespace Git
{
	class StatusFile: public QObject
	{
		Q_OBJECT
		Q_ENUMS(StausType)

		Q_PROPERTY(QString oldPath READ oldPath CONSTANT)
		Q_PROPERTY(QString newPath READ newPath CONSTANT)
		Q_PROPERTY(StausType status READ statusType CONSTANT)
		Q_PROPERTY(bool addToggle READ addToggle CONSTANT)
		Q_PROPERTY(bool removeToggle READ removeToggle CONSTANT)
	public:

		enum StausType
		{
			Current=0,
			New,
			Modified,
			Deleted,
			Renamed,
			TypeChange,
			Untracked,
			Unknown,
		};

		StatusFile(	QObject* _parent=nullptr): QObject(_parent) {}
		StatusFile(	QObject* _parent,
					QString _oldPath,
					QString _newPath,
					StausType _statusType):
								QObject(_parent),
								m_oldPath(_oldPath),
								m_newPath(_newPath),
								m_statusType(_statusType),
								m_addToggle(false),
								m_removeToggle(false)
		{}
		~StatusFile() {}
		QString oldPath() const { return m_oldPath; }
		QString newPath() const { return m_newPath; }
		StausType statusType() const { return m_statusType; }
		bool addToggle() const { return m_addToggle; }
		void setAddToggle(bool _toggle) { m_addToggle = _toggle;}
		bool removeToggle() const { return m_removeToggle; }
		void setRemoveToggle(bool _toggle) {
			m_removeToggle = _toggle;
			qDebug() << "Remove Toggle";
		}
	private:
		QString m_oldPath;
		QString m_newPath;
		bool m_addToggle;
		bool m_removeToggle;
		StausType m_statusType;
	};
}
