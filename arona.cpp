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
#include <QScreen>
#include <QDateTime>
#include <QDir>
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
    
#if DEBUG_MODE
    connect(debugButton, &QPushButton::clicked, this, &arona::onDebugButtonClicked);
    connect(debugTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &arona::onDebugTypeChanged);
#endif
    
    // 初始化日志系统
    appendLog("欢迎使用Arona!");
#if DEBUG_MODE
    appendLog("调试模式已启用", "WARNING");
#endif
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
    
#if DEBUG_MODE
    // ==================== 调试功能区 ====================
    debugGroupBox = new QGroupBox("调试功能", area3);
    debugGroupBox->setStyleSheet("QGroupBox { "
                                 "border: 2px solid #66CCFF; "
                                 "border-radius: 5px; "
                                 "margin-top: 10px; "
                                 "padding-top: 10px; "
                                 "font-weight: bold; "
                                 "color: #66CCFF; "
                                 "} "
                                 "QGroupBox::title { "
                                 "subcontrol-origin: margin; "
                                 "subcontrol-position: top center; "
                                 "padding: 0 5px; "
                                 "background-color: white; "
                                 "}");
    
    QVBoxLayout *debugLayout = new QVBoxLayout(debugGroupBox);
    debugLayout->setSpacing(8);
    debugLayout->setContentsMargins(8, 15, 8, 8);
    
    // 调试类型选择
    debugTypeComboBox = new QComboBox(debugGroupBox);
    debugTypeComboBox->addItem("截图调试");
    debugTypeComboBox->addItem("点击调试");
    debugTypeComboBox->setStyleSheet("QComboBox { "
                                    "border: 2px solid #66CCFF; "
                                    "border-radius: 3px; "
                                    "padding: 3px; "
                                    "background-color: white; "
                                    "font-weight: normal; "
                                    "} "
                                    "QComboBox::drop-down { "
                                    "border: none; "
                                    "} "
                                    "QComboBox::down-arrow { "
                                    "image: none; "
                                    "border-left: 4px solid transparent; "
                                    "border-right: 4px solid transparent; "
                                    "border-top: 5px solid #66CCFF; "
                                    "width: 0; "
                                    "height: 0; "
                                    "}");
    debugLayout->addWidget(debugTypeComboBox);
    
    // 点击调试参数容器
    clickDebugWidget = new QWidget(debugGroupBox);
    clickDebugLayout = new QHBoxLayout(clickDebugWidget);
    clickDebugLayout->setSpacing(5);
    clickDebugLayout->setContentsMargins(0, 0, 0, 0);
    
    xLabel = new QLabel("X:", clickDebugWidget);
    xLabel->setStyleSheet("font-weight: normal; color: #666666;");
    clickDebugLayout->addWidget(xLabel);
    
    clickXSpinBox = new QSpinBox(clickDebugWidget);
    clickXSpinBox->setRange(0, 9999);
    clickXSpinBox->setValue(100);
    clickXSpinBox->setStyleSheet("QSpinBox { "
                                "border: 2px solid #66CCFF; "
                                "border-radius: 3px; "
                                "padding: 2px; "
                                "background-color: white; "
                                "font-weight: normal; "
                                "}");
    clickDebugLayout->addWidget(clickXSpinBox);
    
    yLabel = new QLabel("Y:", clickDebugWidget);
    yLabel->setStyleSheet("font-weight: normal; color: #666666;");
    clickDebugLayout->addWidget(yLabel);
    
    clickYSpinBox = new QSpinBox(clickDebugWidget);
    clickYSpinBox->setRange(0, 9999);
    clickYSpinBox->setValue(100);
    clickYSpinBox->setStyleSheet("QSpinBox { "
                                "border: 2px solid #66CCFF; "
                                "border-radius: 3px; "
                                "padding: 2px; "
                                "background-color: white; "
                                "font-weight: normal; "
                                "}");
    clickDebugLayout->addWidget(clickYSpinBox);
    
    clickDebugWidget->setVisible(false);  // 默认隐藏点击调试参数
    debugLayout->addWidget(clickDebugWidget);
    
    // 开始调试按钮
    debugButton = new QPushButton("开始调试", debugGroupBox);
    debugButton->setMinimumHeight(30);
    debugButton->setStyleSheet("QPushButton { "
                              "background-color: rgba(255, 152, 0, 200); "
                              "color: white; "
                              "border: 2px solid rgba(255, 152, 0, 255); "
                              "border-radius: 5px; "
                              "font-weight: bold; "
                              "padding: 5px; "
                              "} "
                              "QPushButton:hover { "
                              "background-color: #FF9800; "
                              "} "
                              "QPushButton:pressed { "
                              "background-color: #F57C00; "
                              "}");
    debugLayout->addWidget(debugButton);
    
    verticalLayout_3->addWidget(debugGroupBox);
#endif
    
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
        color = "#00A000";  // 绿色
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
}

#if DEBUG_MODE
void arona::onDebugTypeChanged(int index)
{
    // 当选择"点击调试"时显示坐标输入框，否则隐藏
    if (index == 1) {  // 点击调试
        clickDebugWidget->setVisible(true);
    } else {  // 截图调试
        clickDebugWidget->setVisible(false);
    }
}

void arona::onDebugButtonClicked()
{
    int debugType = debugTypeComboBox->currentIndex();
    
    if (debugType == 0) {
        // 截图调试
        screenshotDebug();
    } else if (debugType == 1) {
        // 点击调试
        clickDebug();
    }
}

void arona::screenshotDebug()
{
    QString handleText = handleLineEdit->text();
    if (handleText.isEmpty()) {
        appendLog("请先抓取窗口句柄", "ERROR");
        return;
    }
    
    bool ok;
    qint64 handleValue = handleText.toLongLong(&ok);
    if (!ok) {
        appendLog("句柄格式错误", "ERROR");
        return;
    }
    
    HWND hwnd = reinterpret_cast<HWND>(handleValue);
    
    // 检查窗口是否有效
    if (!IsWindow(hwnd)) {
        appendLog("窗口句柄无效", "ERROR");
        return;
    }
    
    // 获取窗口矩形区域
    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        appendLog("获取窗口矩形失败", "ERROR");
        return;
    }
    
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    // 获取窗口DC
    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);
    SelectObject(hdcMemDC, hbmScreen);
    
    // 复制窗口内容到内存DC
    PrintWindow(hwnd, hdcMemDC, PW_CLIENTONLY);
    
    // 创建QPixmap
    QPixmap pixmap = QPixmap::fromImage(QImage::fromHBITMAP(hbmScreen));
    
    // 清理
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);
    
    // 保存截图
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString screenshotPath = QString("screenshot_%1.png").arg(timestamp);
    
    // 创建screenshots目录
    QDir dir;
    if (!dir.exists("screenshots")) {
        dir.mkdir("screenshots");
    }
    
    screenshotPath = "screenshots/" + screenshotPath;
    
    if (pixmap.save(screenshotPath)) {
        appendLog(QString("截图成功保存: %1").arg(screenshotPath), "SUCCESS");
    } else {
        appendLog("保存截图失败", "ERROR");
    }
}

void arona::clickDebug()
{
    QString handleText = handleLineEdit->text();
    if (handleText.isEmpty()) {
        appendLog("请先抓取窗口句柄", "ERROR");
        return;
    }
    
    bool ok;
    qint64 handleValue = handleText.toLongLong(&ok);
    if (!ok) {
        appendLog("句柄格式错误", "ERROR");
        return;
    }
    
    HWND hwnd = reinterpret_cast<HWND>(handleValue);
    
    // 检查窗口是否有效
    if (!IsWindow(hwnd)) {
        appendLog("窗口句柄无效", "ERROR");
        return;
    }
    
    // 获取点击坐标
    int x = clickXSpinBox->value();
    int y = clickYSpinBox->value();
    
    // 构造lParam (x和y坐标)
    LPARAM lParam = MAKELPARAM(x, y);
    
    // 发送鼠标按下消息
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
    
    // 延迟一小段时间后发送鼠标抬起消息
    QTimer::singleShot(50, [hwnd, lParam, this]() {
        PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
        appendLog(QString("已发送点击消息到目标窗口，坐标: (%1, %2)")
                 .arg(clickXSpinBox->value())
                 .arg(clickYSpinBox->value()), "SUCCESS");
    });
    
    appendLog(QString("正在模拟点击，坐标: (%1, %2)").arg(x).arg(y), "INFO");
}
#endif

