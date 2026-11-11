#pragma once

#include <QImage>
#include <QQuickWindow>
#include <QTest>

#include <cstdlib>

inline double differingFraction(const QImage &actual, const QImage &expected, int channelTolerance = 40)
{
    if (actual.size() != expected.size() || actual.isNull()) {
        return 1.0;
    }

    const QImage a = actual.convertToFormat(QImage::Format_ARGB32);
    const QImage b = expected.convertToFormat(QImage::Format_ARGB32);
    qint64 differing = 0;
    for (int y = 0; y < a.height(); ++y) {
        const QRgb *lineA = reinterpret_cast<const QRgb *>(a.constScanLine(y));
        const QRgb *lineB = reinterpret_cast<const QRgb *>(b.constScanLine(y));
        for (int x = 0; x < a.width(); ++x) {
            const QRgb p = lineA[x];
            const QRgb q = lineB[x];
            if (std::abs(qRed(p) - qRed(q)) > channelTolerance || std::abs(qGreen(p) - qGreen(q)) > channelTolerance
                || std::abs(qBlue(p) - qBlue(q)) > channelTolerance) {
                ++differing;
            }
        }
    }
    return static_cast<double>(differing) / (static_cast<double>(a.width()) * a.height());
}

inline QImage stableGrab(QQuickWindow *window)
{
    QImage previous = window->grabWindow();
    for (int attempt = 0; attempt < 40; ++attempt) {
        QTest::qWait(25);
        const QImage current = window->grabWindow();
        if (current == previous) {
            return current;
        }
        previous = current;
    }
    return previous;
}
