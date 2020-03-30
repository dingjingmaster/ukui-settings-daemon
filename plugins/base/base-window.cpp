#include "base-window.h"

#include <QDesktopWidget>
#include <QGuiApplication>

BaseWindow::BaseWindow(QWidget *parent) : QWidget(parent)
{
    // FIXME://
//    QScreen* screen = QGuiApplication::primaryScreen();
    QRect wh = QDesktopWidget().screenGeometry();

    // FIXME://
    mIsComposited = true;

    if (mIsComposited) {
        double      scalew, scaleh, scale;
        int         size;
        scalew = wh.width() / 640.0;
        scaleh = wh.height() / 480.0;

        scale = qMin(scalew, scaleh);
        size = 130 * qMax(1., scale);
        setFixedSize(QSize(size, size));
        mFadeOutAlpha = 1.0;
    } else {
        setStyleSheet("border:2px;");
    }

}

// FIXME://
bool BaseWindow::isValid()
{
    return mIsComposited;
}

bool BaseWindow::isComposited()
{
    return mIsComposited;
}

void BaseWindow::updateAndHide()
{

}
