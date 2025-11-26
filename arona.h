#ifndef ARONA_H
#define ARONA_H

#include <QMainWindow>
#include <QDateTime>
#include <QMouseEvent>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QMenuBar>
#include <QEvent>
#include <QPaintEvent>
#include <QPixmap>

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
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onCaptureHandleButtonPressed();
    void onSelectBgButtonClicked();

private:
    // UI控件
    QWidget *centralwidget;
    QWidget *area1;
    QWidget *area2;
    QWidget *area3;
    QPushButton *logButton;
    QPushButton *captureHandleButton;
    QPushButton *selectBgButton;
    QPushButton *startButton;
    QLineEdit *handleLineEdit;
    QTextEdit *logTextEdit;
    QMenuBar *menubar;
    
    // 布局
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *verticalSpacer;
    QSpacerItem *verticalSpacer_2;
    
    // 其他成员变量
    bool isCapturingHandle;
    bool waitingForMouseRelease;  // 等待鼠标释放状态
    QTimer *captureTimer;
    QPixmap backgroundPixmap;  // 背景图片
    
    // 辅助函数
    void setupUi();
    void setBackgroundImage(const QString &imagePath);
    void captureWindowHandle();
};
#endif // ARONA_H
