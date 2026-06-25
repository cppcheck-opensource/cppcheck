import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQml.Models 2.12

Item {
    id: root

    property alias model: tableView.model
    default property list<TableColumn> columns

    TableRow {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        background: null
        isHeader: true
        height: 60
    }

    ListView {
        id: tableView
        clip: true

        anchors.fill: parent
        anchors.topMargin: header.height
        delegate: TableRow {}
    }
}
