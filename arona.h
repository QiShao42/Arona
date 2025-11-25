#ifndef ARONA_H
#define ARONA_H

#include <QMainWindow>
#include <QDateTime>
#include <QMouseEvent>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class arona;
}
QT_END_NAMESPACE

class arona : public QMainWindow
{
    Q_OBJECT

public:
    arona(QWidget *parent = nullptr);
    ~arona();

    // 日志输出系统
    void appendLog(const QString &message, const QString &level = "INFO");

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onCaptureHandleButtonPressed();
    void onCaptureHandleButtonReleased();
    void onSelectBgButtonClicked();
    void captureWindowHandle();

private:
    Ui::arona *ui;
    bool isCapturingHandle;
    QTimer *captureTimer;
    
    void setBackgroundImage(const QString &imagePath);
};
#endif // ARONA_H
