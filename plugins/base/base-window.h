#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QWidget>

class BaseWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BaseWindow(QWidget *parent = nullptr);

    bool isValid ();
    bool isComposited ();
    void updateAndHide ();

Q_SIGNALS:

private:
    quint8              mIsComposited;
    quint8              mHideTimeoutID;
    quint8              mFadeTimeoutID;
    double              mFadeOutAlpha;

};

#endif // BASEWINDOW_H
