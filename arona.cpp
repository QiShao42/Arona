#include "arona.h"
#include <QTextCursor>
#include <QFileDialog>
#include <QPixmap>
#include <QPalette>
#include <QCursor>
#include <QApplication>
#include <QRect>
#include <QIcon>
#include <QSize>
#include <QSizePolicy>
#include <QPainter>
#include <windows.h>

arona::arona(QWidget *parent)
    : QMainWindow(parent)
    , isCapturingHandle(false)
    , waitingForMouseRelease(false)
    , captureTimer(nullptr)
{
    setupUi();
    
    // 设置默认背景图片
    setBackgroundImage(":/images/background.jpg");
    
    // 连接信号和槽
    connect(captureHandleButton, &QPushButton::pressed, this, &arona::onCaptureHandleButtonPressed);
    connect(selectBgButton, &QPushButton::clicked, this, &arona::onSelectBgButtonClicked);
    
    // 初始化日志系统
    appendLog("欢迎使用Arona!");
}

arona::~arona()
{
    if (captureTimer) {
        delete captureTimer;
    }
}

void arona::setupUi()
{
    // 设置主窗口属性
    this->resize(800, 600);
    this->setMinimumSize(QSize(800, 600));
    this->setMaximumSize(QSize(800, 600));
    this->setWindowTitle("ARONA");
    
    // 创建中心部件
    centralwidget = new QWidget(this);
    this->setCentralWidget(centralwidget);
    
    // 创建主水平布局
    horizontalLayout = new QHBoxLayout(centralwidget);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    
    // ==================== 区域1：左侧按钮区 ====================
    area1 = new QWidget(centralwidget);
    area1->setMinimumSize(QSize(60, 0));
    area1->setMaximumSize(QSize(60, 16777215));
    
    verticalLayout = new QVBoxLayout(area1);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    
    // 执行日志按钮
    logButton = new QPushButton(area1);
    logButton->setMinimumSize(QSize(40, 60));
    logButton->setText("执行\n日志");
    logButton->setIcon(QIcon::fromTheme("QIcon::ThemeIcon::DocumentProperties"));
    logButton->setIconSize(QSize(24, 24));
    verticalLayout->addWidget(logButton);
    
    // 垂直弹簧
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(verticalSpacer);
    
    horizontalLayout->addWidget(area1);
    
    // ==================== 区域2：中间日志区 ====================
    area2 = new QWidget(centralwidget);
    area2->setMinimumSize(QSize(540, 0));
    area2->setMaximumSize(QSize(540, 16777215));
    area2->setObjectName("area2");
    
    verticalLayout_2 = new QVBoxLayout(area2);
    verticalLayout_2->setSpacing(5);
    verticalLayout_2->setContentsMargins(0, 0, 0, 0);
    
    // 日志文本框
    logTextEdit = new QTextEdit(area2);
    logTextEdit->setReadOnly(true);
    logTextEdit->setStyleSheet("QTextEdit { "
                              "background-color: rgba(255, 255, 255, 160); "
                              "font-family: \"Microsoft YaHei\";"
                              "font-size: 10pt; "
                              "margin: 0;"
                              "border: 2px solid rgba(102, 204, 255, 200); "
                              "border-radius: 8px; "
                              "padding: 5px; "
                              "}");
    verticalLayout_2->addWidget(logTextEdit);
    
    horizontalLayout->addWidget(area2);
    
    // ==================== 区域3：右侧控制区 ====================
    area3 = new QWidget(centralwidget);
    area3->setMinimumSize(QSize(200, 0));
    area3->setMaximumSize(QSize(200, 16777215));
    
    verticalLayout_3 = new QVBoxLayout(area3);
    verticalLayout_3->setSpacing(10);
    verticalLayout_3->setContentsMargins(10, 10, 10, 10);
    
    // 抓取句柄行布局
    horizontalLayout_2 = new QHBoxLayout();
    
    // 抓取句柄按钮
    captureHandleButton = new QPushButton(area3);
    captureHandleButton->setMinimumSize(QSize(0, 35));
    captureHandleButton->setStyleSheet("QPushButton { "
                                      "background-color: rgba(102, 204, 255, 200); "
                                      "color: white; "
                                      "border: 2px solid rgba(102, 204, 255, 255); "
                                      "border-radius: 5px; "
                                      "font-weight: bold; "
                                      "padding: 5px; "
                                      "} "
                                      "QPushButton:hover { "
                                      "background-color: #55BBEE; "
                                      "} "
                                      "QPushButton:pressed { "
                                      "background-color: #44AADD; "
                                      "}");
    captureHandleButton->setText("抓取句柄");
    horizontalLayout_2->addWidget(captureHandleButton);
    
    // 句柄输入框
    handleLineEdit = new QLineEdit(area3);
    handleLineEdit->setMinimumSize(QSize(80, 35));
    handleLineEdit->setMaximumSize(QSize(80, 16777215));
    handleLineEdit->setStyleSheet("QLineEdit { "
                                 "border: 2px solid #66CCFF; "
                                 "border-radius: 5px; "
                                 "padding: 5px; "
                                 "background-color: white; "
                                 "}");
    handleLineEdit->setReadOnly(true);
    handleLineEdit->setAlignment(Qt::AlignCenter);
    horizontalLayout_2->addWidget(handleLineEdit);
    
    verticalLayout_3->addLayout(horizontalLayout_2);
    
    // 选择背景图按钮
    selectBgButton = new QPushButton(area3);
    selectBgButton->setMinimumSize(QSize(0, 35));
    selectBgButton->setStyleSheet("QPushButton { "
                                 "background-color: rgba(102, 204, 255, 200); "
                                 "color: white; "
                                 "border: 2px solid rgba(102, 204, 255, 255); "
                                 "border-radius: 5px; "
                                 "font-weight: bold; "
                                 "padding: 5px; "
                                 "} "
                                 "QPushButton:hover { "
                                 "background-color: #55BBEE; "
                                 "} "
                                 "QPushButton:pressed { "
                                 "background-color: #44AADD; "
                                 "}");
    selectBgButton->setText("选择背景图");
    verticalLayout_3->addWidget(selectBgButton);
    
    // 垂直弹簧2
    verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout_3->addItem(verticalSpacer_2);
    
    // 启动按钮
    startButton = new QPushButton(area3);
    startButton->setMinimumSize(QSize(0, 45));
    startButton->setStyleSheet("QPushButton { "
                              "background-color: rgba(102, 204, 255, 200); "
                              "color: white; "
                              "border: 2px solid rgba(102, 204, 255, 255); "
                              "border-radius: 5px; "
                              "font-weight: bold; "
                              "font-size: 14pt; "
                              "padding: 5px; "
                              "} "
                              "QPushButton:hover { "
                              "background-color: #55BBEE; "
                              "} "
                              "QPushButton:pressed { "
                              "background-color: #44AADD; "
                              "}");
    startButton->setText("启动");
    verticalLayout_3->addWidget(startButton);
    
    horizontalLayout->addWidget(area3);
    
    // 创建菜单栏
    menubar = new QMenuBar(this);
    menubar->setGeometry(QRect(0, 0, 800, 21));
    this->setMenuBar(menubar);
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
    logTextEdit->append(logMessage);
    
    // 自动滚动到底部
    logTextEdit->moveCursor(QTextCursor::End);
}

void arona::mousePressEvent(QMouseEvent *event)
{
    QMainWindow::mousePressEvent(event);
}

void arona::mouseReleaseEvent(QMouseEvent *event)
{
    QMainWindow::mouseReleaseEvent(event);
}

void arona::paintEvent(QPaintEvent *event)
{
    // 绘制背景图片
    if (!backgroundPixmap.isNull()) {
        QPainter painter(this);
        painter.drawPixmap(0, 0, backgroundPixmap);
    }
    
    QMainWindow::paintEvent(event);
}

bool arona::eventFilter(QObject *watched, QEvent *event)
{
    // 如果正在等待鼠标释放，监听全局鼠标释放事件
    if (waitingForMouseRelease && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            waitingForMouseRelease = false;
            
            // 移除事件过滤器
            QApplication::instance()->removeEventFilter(this);
            
            // 延迟一小段时间后捕获，确保鼠标事件已经处理完成
            QTimer::singleShot(100, this, &arona::captureWindowHandle);
            
            return true;  // 事件已处理
        }
    }
    
    return QMainWindow::eventFilter(watched, event);
}

void arona::onCaptureHandleButtonPressed()
{
    waitingForMouseRelease = true;
    isCapturingHandle = true;
    
    // 安装全局事件过滤器来捕获鼠标释放事件
    QApplication::instance()->installEventFilter(this);
    
    // 改变鼠标样式为准心
    QApplication::setOverrideCursor(Qt::CrossCursor);
    
    appendLog("正在抓取窗口句柄，请按住鼠标移动到目标窗口然后松开...", "INFO");
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
        handleLineEdit->setText(QString::number(handleValue));
        
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
        handleLineEdit->clear();
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
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        QPalette palette = this->palette();
        int windowWidth = this->width();
        int windowHeight = this->height();

        // 按比例缩放图片，使图片至少有一个维度与窗口尺寸一致
        QPixmap scaledPixmap = pixmap.scaled(
            windowWidth, windowHeight,
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation
            );

        // 计算裁剪区域，保证从图片中心裁剪
        int scaledWidth = scaledPixmap.width();
        int scaledHeight = scaledPixmap.height();
        int x = (scaledWidth - windowWidth) / 2;
        int y = (scaledHeight - windowHeight) / 2;

        // 裁剪图片
        QPixmap croppedPixmap = scaledPixmap.copy(x, y, windowWidth, windowHeight);

        palette.setBrush(QPalette::Window, QBrush(croppedPixmap));
        this->setPalette(palette);
    }
    /* // 加载原始图片
    QPixmap originalPixmap(imagePath);
    if (originalPixmap.isNull()) {
        appendLog("加载图片失败", "ERROR");
        return;
    }
    
    // 获取窗口尺寸
    QSize windowSize = this->size();
    int windowWidth = windowSize.width();
    int windowHeight = windowSize.height();
    
    // 获取图片尺寸
    int imgWidth = originalPixmap.width();
    int imgHeight = originalPixmap.height();
    
    // 计算缩放比例，使图片至少有一个维度与窗口尺寸一致
    // 使用较大的缩放比例，确保图片能完全覆盖窗口（类似CSS的cover效果）
    double scaleWidth = static_cast<double>(windowWidth) / imgWidth;
    double scaleHeight = static_cast<double>(windowHeight) / imgHeight;
    double scale = qMax(scaleWidth, scaleHeight);
    
    // 按比例缩放图片
    int scaledWidth = static_cast<int>(imgWidth * scale);
    int scaledHeight = static_cast<int>(imgHeight * scale);
    QPixmap scaledPixmap = originalPixmap.scaled(scaledWidth, scaledHeight, 
                                                  Qt::KeepAspectRatio, 
                                                  Qt::SmoothTransformation);
    
    // 从中心裁剪图片
    int cropX = (scaledWidth - windowWidth) / 2;
    int cropY = (scaledHeight - windowHeight) / 2;
    backgroundPixmap = scaledPixmap.copy(cropX, cropY, windowWidth, windowHeight);
    
    // 触发重绘
    this->update(); */
}
