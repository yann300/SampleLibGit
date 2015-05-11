// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.2
import Git.Repository 1.0
import Git.StatusFile 1.0

Rectangle {
	width: 600;
	height: 400;

	Component.onCompleted:
	{
		console.log("Complete");
	}

	Repository
	{
		id: gitrepo
		onErrorMessage:
		{
			console.log("error: ", _error, "  message:", _message);
		}
		onChangeRepository:
		{
			gitrepo.getStausFiles();
		}
		onUpdateFileStatus:
		{
			var functions = gitrepo.statusFiles;
			filesStatusList.model.clear();
			for (var f = 0; f < functions.length; f++) {
				var statusStr="unknown";
				switch(functions[f].type)
				{
				case StatusFile.Current:
					statusStr="Current";
					break;
				case StatusFile.WTNew:
					statusStr="WT New";
					break;
				case StatusFile.WTDeleted:
					statusStr="WT Deleted";
					break;
				case StatusFile.New:
					statusStr="New";
					break;
				case StatusFile.Modified:
					statusStr="Modified";
					break;
				case StatusFile.Deleted:
					statusStr="Deleted";
					break;
				case StatusFile.Renamed:
					statusStr="Renamed";
					break;
				case StatusFile.TypeChange:
					statusStr="TypeChange";
					break;
				case StatusFile.Untracked:
					statusStr="Untracked";
					break;
				case StatusFile.Unknown:
					statusStr="Unknown";
					break;
				}
				filesStatusList.model.append({ file: functions[f].newPath, type: statusStr });
			}
		}
	}
	ColumnLayout {
		RowLayout {
			Text{
				id: urlText
				text: "URL Repo:"
			}
			TextInput{
				id: urlInput
				text: "https://github.com/Mashatan/CustomCombobox"
			}
		}
		RowLayout {
			Text{
				id: pathText
				text: "Path Repo:"
			}
			TextInput{
				id: pathInput
				text: "c:/temp/test"
			}
		}
		RowLayout {
			Text{
				id: commitText
				text: "Commit Message:"
			}
			TextInput{
				id: commitInput
				text: "test"
			}
		}
		RowLayout {
			Button{
				text: "Open"
				onClicked: {
					gitrepo.open(pathInput.text);
				}
			}
			Button{
				text: "Close"
				onClicked: {
					gitrepo.close();
				}
			}
			Button{
				text: "Refresh"
				onClicked: {
					gitrepo.getStausFiles();
				}
			}
			Button{
				text: "Clone"
				onClicked: {
					gitrepo.setURL(urlInput.text);
					gitrepo.clone(pathInput.text);
				}
			}
			Button{
				text: "Commit"
				onClicked: {
					gitrepo.setAuthor("Ali");
					gitrepo.setEmail("Ali@ethdev.com");
					gitrepo.commit(commitInput.text);
				}
			}
		}
		TableView {
			id: filesStatusList
			height: 200
			implicitHeight: 0
			width: 400
			model: ListModel{}

			TableViewColumn {
				role: "file"
				title: "file"
				width: 100
			}
			TableViewColumn {
				role: "type"
				title: "type"
				width: 100
			}
		} //Table View
	}



}
