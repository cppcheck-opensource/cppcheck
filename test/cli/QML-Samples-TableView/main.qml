import QtQuick 2.12
import QtQuick.Window 2.12
import Test 1.0
import '.' as Here

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Component.onCompleted: sampleModel.fillSampleData(50)
    SampleModel {
        id: sampleModel
    }

    Here.TableView {
        anchors.fill: parent
        model: sampleModel

        TableColumn {
            title: qsTr("Id")
            role: 'id'
            size: 30
        }
        TableColumn {
            title: qsTr("Name")
            role: 'name'
            fillWidth: true
        }
        TableColumn {
            title: qsTr("Grade")
            role: 'grade'
            size: 50
        }
    }
}
