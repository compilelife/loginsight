.pragma library

let curIndex = 0

const colorTable = [Qt.rgba(1,0,0,1), Qt.rgba(0,1,0,1), Qt.rgba(0,0,1,1)]

function next() {
  return colorTable[(curIndex++) % colorTable.length]
}
