import QtQuick 2.0
import QtQuick.Layouts 1.15

RowLayout {
  spacing: 10
  property string hint: ''
  Layout.preferredWidth: parent.width

  Text {
    text: hint
    height: parent.height
    verticalAlignment: Qt.AlignVCenter
  }
}
