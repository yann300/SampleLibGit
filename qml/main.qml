// Date: 2015
// Author: Ali Mashatan
// Email : ali.mashatan@gmail.com

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.2
import Git.Repository 1.0
import Git.StatusFile 1.0
//test8877

Rectangle {
	width: 900;
	height: 400;

	Repository
	{
		id: gitrepo
		onErrorMessage:
		{
			console.log("error: ", _error, "  message:", _message);
		}
		onRemoteCredential:
		{
			gitrepo.setUsername(usernameInput.text);
			gitrepo.setPassword(passwordInput.text);
		}
		onRemoteProgress:
		{
			console.log("Remote message:", _message);
		}

		onUpdateFileStatus:
		{
			var functions = gitrepo.statusFiles;
			statusModel.clear();
			for (var f = 0; f < functions.length; f++) {
				var statusStr="unknown";
				switch(functions[f].status)
				{
				case StatusFile.Current:
					statusStr="Current";
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
				//console.log("Addtoggle:", functions[f].addToggle);
				statusModel.append({ file: functions[f].newPath,
												 status: statusStr,
												 statusID: functions[f].status,
												 addToggle: functions[f].addToggle,
												 removeToggle: functions[f].removeToggle });
			}
		}
	}
	ListModel {
		id: statusModel
	}

	ColumnLayout {
		RowLayout {
			Text{
				id: urlText
				text: "URL Repo:"
			}
			TextInput{
				id: urlInput
				text: "https://github.com/yann300/SampleLibGit"
			}
		}
		RowLayout {
			Text{
				id: pathText
				text: "Path Repo:"
			}
			TextInput{
				id: pathInput
				text: "/home/yann/Ethereum/Projects/aliSampleLibGit/SampleLibGit"
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
			Text{
				id: usernameText
				text: "Username:"
			}
			TextInput{
				id: usernameInput
				text: "yann300"
			}
		}

		RowLayout {
			Text{
				id: mailText
				text: "Mail:"
			}
			TextInput{
				id: mailInput
				text: "yann.levreau@gmail.com"
			}
		}

		RowLayout {
			Text{
				id: passwordText
				text: "Password:"
			}
			TextInput{
				id: passwordInput
				text: "test"
				echoMode: TextInput.Password
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
					gitrepo.setAuthor(usernameInput.text);
					gitrepo.setEmail(mailInput.text);
					gitrepo.commit(commitInput.text);
				}
			}
			Button{
				text: "Push"
				onClicked: {
					gitrepo.setURL(urlInput.text);
					gitrepo.push();
				}
			}
		}
		TableView {
			id: filesStatusList
			height: 200
			implicitHeight: 0
			implicitWidth: 500
			width: 900
			model: statusModel

			TableViewColumn {
				role: "file"
				title: "File"
				width: 100
			}
			TableViewColumn {
				role: "status"
				title: "Status"
				width: 100
			}
			TableViewColumn {
				role: "addToggle"
				title: "Add"
				width: 50
				delegate:
					CheckBox {
						anchors.fill: parent
						checked: styleData.value
						/*Component.onCompleted: {
							 console.log( "Add Status ID: ",filesStatusList.model.get(styleData.row).statusID);
							 if (filesStatusList.model.get(styleData.row).statusID === StatusFile.Untracked)
							 {
								 addCheckboxDelegate.visible = true;
							 } else {
								 addCheckboxDelegate.visible = false;
							 }
						  }*/
						 onCheckedChanged: {
							 gitrepo.setAddToggle(filesStatusList.model.get(styleData.row).file, checked);
						 }
					 }
			}
			TableViewColumn {
				role: "removeToggle"
				title: "Remove"
				width: 50
				delegate:
					CheckBox {
						 anchors.fill: parent
						 checked: styleData.value
						 /*Component.onCompleted: {
							 console.log( "Remove Status ID: ",filesStatusList.model.get(styleData.row).statusID);
							 if (filesStatusList.model.get(styleData.row).statusID === StatusFile.Current||
								filesStatusList.model.get(styleData.row).statusID === StatusFile.Modified )
							 {
								 this.visible = true;
							 } else {
								 this.visible = false;

							 }
						 }*/
						 onCheckedChanged: {
							 gitrepo.setRemoveToggle(filesStatusList.model.get(styleData.row).file, checked);

						 }
					 }
			}
		} //Table View
	}// ColumnLayout

}
