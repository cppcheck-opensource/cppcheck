import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQml.Models 2.12

ItemDelegate {
    height: 40
    width: parent.width
    property bool isHeader: false
    RowLayout {
        property var _d: model
        anchors.fill: parent
        anchors.leftMargin: 9

        Repeater {
            model: root.columns
            Label {
                text: parent.parent.isHeader ? modelData.title : parent._d[modelData.role]
                Layout.fillWidth: modelData.fillWidth
                Layout.preferredWidth: modelData.size !== '*'
                                       ? modelData.size : 20
            }
        }
    }
}
