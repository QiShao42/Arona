#include "arona.h"
#include "./ui_arona.h"
#include <QTextCursor>
#include <QFileDialog>
#include <QPixmap>
#include <QPalette>
#include <QCursor>
#include <QApplication>
#include <windows.h>

arona::arona(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::arona)
    , isCapturingHandle(false)
    , captureTimer(nullptr)
{
    ui->setupUi(this);
    
    // 连接信号和槽
    connect(ui->captureHandleButton, &QPushButton::pressed, this, &arona::onCaptureHandleButtonPressed);
    connect(ui->captureHandleButton, &QPushButton::released, this, &arona::onCaptureHandleButtonReleased);
    connect(ui->selectBgButton, &QPushButton::clicked, this, &arona::onSelectBgButtonClicked);
    
    // 初始化日志系统
    appendLog("欢迎使用Arona!");
}

arona::~arona()
{
    if (captureTimer) {
        delete captureTimer;
    }
    delete ui;
}

void arona::appendLog(const QString &message, const QString &level)
{
    // 获取当前时间
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    
    // 根据日志级别设置颜色
    QString color;
    if (level == "INFO") {
        color = "#000000";  // 黑色
    } else if (level == "SUCCESS") {
        color = "#00AA00";  // 绿色
    } else if (level == "WARNING") {
        color = "#FF8800";  // 橙色
    } else if (level == "ERROR") {
        color = "#FF0000";  // 红色
    } else {
        color = "#000000";  // 默认黑色
    }
    
    // 格式化日志消息
    QString logMessage = QString("<span style='color: #666666;'>[%1]</span> "
                                 "<span style='color: %2; font-weight: bold;'>[%3]</span> "
                                 "<span style='color: %2;'>%4</span>")
                             .arg(currentTime)
                             .arg(color)
                             .arg(level)
                             .arg(message);
    
    // 追加到日志文本框
    ui->logTextEdit->append(logMessage);
    
    // 自动滚动到底部
    ui->logTextEdit->moveCursor(QTextCursor::End);
}

void arona::mousePressEvent(QMouseEvent *event)
{
    QMainWindow::mousePressEvent(event);
}

void arona::mouseReleaseEvent(QMouseEvent *event)
{
    QMainWindow::mouseReleaseEvent(event);
}

void arona::onCaptureHandleButtonPressed()
{
    isCapturingHandle = true;
    
    // 改变鼠标样式为准心
    QApplication::setOverrideCursor(Qt::CrossCursor);
    
    appendLog("正在抓取窗口句柄，请移动鼠标到目标窗口并松开鼠标...", "INFO");
}

void arona::onCaptureHandleButtonReleased()
{
    if (isCapturingHandle) {
        // 延迟一小段时间后捕获，确保鼠标已经移动到目标窗口
        QTimer::singleShot(100, this, &arona::captureWindowHandle);
    }
}

void arona::captureWindowHandle()
{
    isCapturingHandle = false;
    
    // 恢复鼠标样式
    QApplication::restoreOverrideCursor();
    
    // 获取当前鼠标位置
    POINT pt;
    GetCursorPos(&pt);
    
    // 获取鼠标位置的窗口句柄
    HWND hwnd = WindowFromPoint(pt);
    
    if (hwnd != NULL) {
        // 将句柄转换为10进制整数并显示
        qint64 handleValue = reinterpret_cast<qint64>(hwnd);
        ui->handleLineEdit->setText(QString::number(handleValue));
        
        // 获取窗口标题
        wchar_t windowTitle[256];
        GetWindowTextW(hwnd, windowTitle, 256);
        QString title = QString::fromWCharArray(windowTitle);
        
        if (title.isEmpty()) {
            title = "(无标题窗口)";
        }
        
        appendLog(QString("成功抓取窗口句柄: %1, 窗口标题: %2").arg(handleValue).arg(title), "SUCCESS");
    } else {
        appendLog("抓取窗口句柄失败", "ERROR");
        ui->handleLineEdit->clear();
    }
}

void arona::onSelectBgButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("选择背景图片"),
                                                    "",
                                                    tr("图片文件 (*.png *.jpg *.jpeg *.bmp)"));
    
    if (!fileName.isEmpty()) {
        setBackgroundImage(fileName);
        appendLog(QString("已设置背景图片: %1").arg(fileName), "SUCCESS");
    }
}

void arona::setBackgroundImage(const QString &imagePath)
{
    // 设置背景图片
    QString styleSheet = QString("QMainWindow { "
                                 "background-image: url(%1); "
                                 "background-position: center; "
                                 "}").arg(imagePath);
    this->setStyleSheet(styleSheet);
}
