import QtQuick

QtObject {
    id: scroller

    required property Flickable flickable
    property real lineStep: 0
    property real pageOverlap: 0

    function scrollBy(distance: real) {
        const area = scroller.flickable;
        const top = area.originY - area.topMargin;
        const bottom = Math.max(top, area.originY + area.contentHeight + area.bottomMargin - area.height);
        area.contentY = Math.max(top, Math.min(bottom, area.contentY + distance));
    }

    function handle(event: KeyEvent): bool {
        const area = scroller.flickable;
        const control = (event.modifiers & Qt.ControlModifier) !== 0;
        const total = area.contentHeight + area.topMargin + area.bottomMargin;
        switch (event.key) {
        case Qt.Key_Down:
            if (!control) {
                return false;
            }
            scroller.scrollBy(scroller.lineStep);
            return true;
        case Qt.Key_Up:
            if (!control) {
                return false;
            }
            scroller.scrollBy(-scroller.lineStep);
            return true;
        case Qt.Key_PageDown:
            scroller.scrollBy(area.height - scroller.pageOverlap);
            return true;
        case Qt.Key_PageUp:
            scroller.scrollBy(scroller.pageOverlap - area.height);
            return true;
        case Qt.Key_Home:
            scroller.scrollBy(-total);
            return true;
        case Qt.Key_End:
            scroller.scrollBy(total);
            return true;
        }
        return false;
    }
}
