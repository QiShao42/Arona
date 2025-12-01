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
#include <QDebug>
#include <QRegularExpression>
#include <QEventLoop>
#include <windows.h>
#include <winuser.h>

const QPoint arona::BUTTON_HALL_TO_CAFE1 = QPoint(118, 960);
const QPoint arona::BUTTON_CAFE1_TO_CAFE2 = QPoint(230, 134);
const QPoint arona::BUTTON_CAFE2_TO_CAFE1 = QPoint(230, 134);
const QPoint arona::BUTTON_INVITATION_TICKET = QPoint(1310, 953);

arona::arona(QWidget *parent)
    : QMainWindow(parent)
    , isCapturingHandle(false)
    , capturingHandleIndex(1)
    , waitingForMouseRelease(false)
    , isRunning(false)
    , shouldStop(false)
    , captureTimer(nullptr)
    , schedulerTimer(nullptr)
    , currentHandleIndex(0)
    , timerEnabled(false)
{
    setupUi();
    
    // 初始化多窗口句柄列表
    gameHandles.resize(3);
    for (int i = 0; i < 3; i++) {
        gameHandles[i] = NULL;
    }
    
    // 创建定时设置对话框
    timerDialog = new TimerDialog(this);
    
    // 设置默认背景图片
    setBackgroundImage(":/images/background.jpg");
    
    // 连接信号和槽
    connect(captureHandleButton, &QPushButton::pressed, this, &arona::onCaptureHandleButtonPressed);
    connect(captureHandleButton2, &QPushButton::pressed, this, &arona::onCaptureHandle2ButtonPressed);
    connect(captureHandleButton3, &QPushButton::pressed, this, &arona::onCaptureHandle3ButtonPressed);
    connect(selectBgButton, &QPushButton::clicked, this, &arona::onSelectBgButtonClicked);
    connect(startButton, &QPushButton::clicked, this, &arona::onstartButtonClicked);
    connect(timerSettingsButton, &QPushButton::clicked, this, &arona::onTimerSettingsButtonClicked);

    // 加载位置模板
    loadPositionTemplates();

    // 加载邀请券模板
    loadpositionReadyTemplates();
    
    // 加载学生头像模板（二值化）
    loadStudentAvatarTemplates();
    
    // 创建定时器用于检查定时任务
    schedulerTimer = new QTimer(this);
    connect(schedulerTimer, &QTimer::timeout, this, &arona::onSchedulerTimerTimeout);
    schedulerTimer->start(30000);  // 每30秒检查一次
    
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
    
    // ==================== 多窗口句柄管理 ====================
    QString buttonStyle = "QPushButton { "
                         "background-color: rgba(102, 204, 255, 200); "
                         "color: white; "
                         "border: 2px solid rgba(102, 204, 255, 255); "
                         "border-radius: 5px; "
                         "font-weight: bold; "
                         "padding: 5px; "
                         "font-size: 9pt; "
                         "} "
                         "QPushButton:hover { "
                         "background-color: #55BBEE; "
                         "} "
                         "QPushButton:pressed { "
                         "background-color: #44AADD; "
                         "}";
    
    QString lineEditStyle = "QLineEdit { "
                           "border: 2px solid #66CCFF; "
                           "border-radius: 5px; "
                           "padding: 5px; "
                           "background-color: white; "
                           "}";
    
    // 句柄1
    horizontalLayout_2 = new QHBoxLayout();
    captureHandleButton = new QPushButton("句柄1", area3);
    captureHandleButton->setMinimumSize(QSize(0, 30));
    captureHandleButton->setStyleSheet(buttonStyle);
    horizontalLayout_2->addWidget(captureHandleButton);
    
    handleLineEdit = new QLineEdit(area3);
    handleLineEdit->setMinimumSize(QSize(80, 30));
    handleLineEdit->setMaximumSize(QSize(80, 16777215));
    handleLineEdit->setStyleSheet(lineEditStyle);
    handleLineEdit->setReadOnly(true);
    handleLineEdit->setAlignment(Qt::AlignCenter);
    handleLineEdit->setPlaceholderText("空");
    horizontalLayout_2->addWidget(handleLineEdit);
    verticalLayout_3->addLayout(horizontalLayout_2);
    
    // 句柄2
    QHBoxLayout *horizontalLayout_handle2 = new QHBoxLayout();
    captureHandleButton2 = new QPushButton("句柄2", area3);
    captureHandleButton2->setMinimumSize(QSize(0, 30));
    captureHandleButton2->setStyleSheet(buttonStyle);
    horizontalLayout_handle2->addWidget(captureHandleButton2);
    
    handleLineEdit2 = new QLineEdit(area3);
    handleLineEdit2->setMinimumSize(QSize(80, 30));
    handleLineEdit2->setMaximumSize(QSize(80, 16777215));
    handleLineEdit2->setStyleSheet(lineEditStyle);
    handleLineEdit2->setReadOnly(true);
    handleLineEdit2->setAlignment(Qt::AlignCenter);
    handleLineEdit2->setPlaceholderText("空");
    horizontalLayout_handle2->addWidget(handleLineEdit2);
    verticalLayout_3->addLayout(horizontalLayout_handle2);
    
    // 句柄3
    QHBoxLayout *horizontalLayout_handle3 = new QHBoxLayout();
    captureHandleButton3 = new QPushButton("句柄3", area3);
    captureHandleButton3->setMinimumSize(QSize(0, 30));
    captureHandleButton3->setStyleSheet(buttonStyle);
    horizontalLayout_handle3->addWidget(captureHandleButton3);
    
    handleLineEdit3 = new QLineEdit(area3);
    handleLineEdit3->setMinimumSize(QSize(80, 30));
    handleLineEdit3->setMaximumSize(QSize(80, 16777215));
    handleLineEdit3->setStyleSheet(lineEditStyle);
    handleLineEdit3->setReadOnly(true);
    handleLineEdit3->setAlignment(Qt::AlignCenter);
    handleLineEdit3->setPlaceholderText("空");
    horizontalLayout_handle3->addWidget(handleLineEdit3);
    verticalLayout_3->addLayout(horizontalLayout_handle3);
    
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
    
    // ==================== 定时功能按钮 ====================
    timerSettingsButton = new QPushButton("定时功能", area3);
    timerSettingsButton->setMinimumSize(QSize(0, 35));
    timerSettingsButton->setStyleSheet("QPushButton { "
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
    verticalLayout_3->addWidget(timerSettingsButton);
    
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
    debugTypeComboBox->addItem("按键调试");
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
    
    // 按键调试参数容器
    keyDebugWidget = new QWidget(debugGroupBox);
    keyDebugLayout = new QHBoxLayout(keyDebugWidget);
    keyDebugLayout->setSpacing(5);
    keyDebugLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *keyLabel = new QLabel("按键:", keyDebugWidget);
    keyLabel->setStyleSheet("font-weight: normal; color: #666666;");
    keyDebugLayout->addWidget(keyLabel);
    
    keySelectComboBox = new QComboBox(keyDebugWidget);
    keySelectComboBox->addItem("F", 0x46);          // F键
    keySelectComboBox->addItem("Ctrl", VK_CONTROL); // Ctrl键
    keySelectComboBox->addItem("CapsLock", VK_CAPITAL); // CapsLock键
    keySelectComboBox->addItem("Shift", VK_SHIFT); // Shift键
    keySelectComboBox->setStyleSheet("QComboBox { "
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
    keyDebugLayout->addWidget(keySelectComboBox);
    
    keyDebugWidget->setVisible(false);  // 默认隐藏按键调试参数
    debugLayout->addWidget(keyDebugWidget);
    
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
            int index = capturingHandleIndex;
            QTimer::singleShot(100, this, [this, index]() {
                captureWindowHandle(index);
            });
            
            return true;  // 事件已处理
        }
    }
    
    return QMainWindow::eventFilter(watched, event);
}

void arona::onstartButtonClicked()
{
    if (isRunning) {
        // 当前正在运行，点击按钮则停止
        stopScript();
    } else {
        // 当前未运行，点击按钮则启动
        startScript();
    }
}

void arona::onCaptureHandleButtonPressed()
{
    capturingHandleIndex = 1;
    waitingForMouseRelease = true;
    isCapturingHandle = true;
    
    // 安装全局事件过滤器来捕获鼠标释放事件
    QApplication::instance()->installEventFilter(this);
    
    // 改变鼠标样式为准心
    QApplication::setOverrideCursor(Qt::CrossCursor);

    appendLog("正在抓取窗口句柄1，请按住鼠标移动到目标窗口然后松开...", "INFO");
}

void arona::onCaptureHandle2ButtonPressed()
{
    capturingHandleIndex = 2;
    waitingForMouseRelease = true;
    isCapturingHandle = true;
    
    QApplication::instance()->installEventFilter(this);
    QApplication::setOverrideCursor(Qt::CrossCursor);
    
    appendLog("正在抓取窗口句柄2，请按住鼠标移动到目标窗口然后松开...", "INFO");
}

void arona::onCaptureHandle3ButtonPressed()
{
    capturingHandleIndex = 3;
    waitingForMouseRelease = true;
    isCapturingHandle = true;
    
    QApplication::instance()->installEventFilter(this);
    QApplication::setOverrideCursor(Qt::CrossCursor);
    
    appendLog("正在抓取窗口句柄3，请按住鼠标移动到目标窗口然后松开...", "INFO");
}

void arona::captureWindowHandle(int handleIndex)
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
        
        // 存储到对应的句柄列表中
        gameHandles[handleIndex - 1] = hwnd;
        
        // 更新对应的输入框
        QLineEdit *targetEdit = nullptr;
        if (handleIndex == 1) targetEdit = handleLineEdit;
        else if (handleIndex == 2) targetEdit = handleLineEdit2;
        else if (handleIndex == 3) targetEdit = handleLineEdit3;
        
        if (targetEdit) {
            targetEdit->setText(QString::number(handleValue));
        }
        
        // 获取窗口标题
        wchar_t windowTitle[256];
        GetWindowTextW(hwnd, windowTitle, 256);
        QString title = QString::fromWCharArray(windowTitle);
        
        if (title.isEmpty()) {
            title = "(无标题窗口)";
        }
        
        // 打印窗口句柄，标题和父窗口名
        wchar_t parentTitle[256];
        GetWindowTextW(GetParent(hwnd), parentTitle, 256);
        QString parentTitleStr = QString::fromWCharArray(parentTitle);
        appendLog(QString("成功抓取窗口句柄%1: %2, 窗口标题: %3, 父窗口名: %4")
                 .arg(handleIndex).arg(handleValue).arg(title).arg(parentTitleStr), "SUCCESS");
    } else {
        appendLog(QString("抓取窗口句柄%1失败").arg(handleIndex), "ERROR");
        if (handleIndex == 1) handleLineEdit->clear();
        else if (handleIndex == 2) handleLineEdit2->clear();
        else if (handleIndex == 3) handleLineEdit3->clear();
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

QImage arona::captureWindow(HWND hwnd)
{
    // 检查窗口是否有效
    if (!IsWindow(hwnd)) {
        appendLog("窗口句柄无效", "ERROR");
        return QImage();
    }
    
    // 获取窗口矩形区域
    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        appendLog("获取窗口矩形失败", "ERROR");
        return QImage();
    }
    
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    // 获取窗口DC
    HDC hdcWindow = GetDC(hwnd);
    if (!hdcWindow) {
        appendLog("获取窗口DC失败", "ERROR");
        return QImage();
    }
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    if (!hdcMemDC) {
        appendLog("创建内存DC失败", "ERROR");
        ReleaseDC(hwnd, hdcWindow);
        return QImage();
    }
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);
    if (!hbmScreen) {
        appendLog("创建兼容位图失败", "ERROR");
        DeleteDC(hdcMemDC);
        ReleaseDC(hwnd, hdcWindow);
        return QImage();
    }
    SelectObject(hdcMemDC, hbmScreen);
    // 复制窗口内容到内存DC
    PrintWindow(hwnd, hdcMemDC, PW_RENDERFULLCONTENT);
    
    // 创建QPixmap
    QPixmap pixmap = QPixmap::fromImage(QImage::fromHBITMAP(hbmScreen));
    
    // 清理
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);

    return pixmap.toImage();
}

QString arona::calculateImageHash(const QImage& image, const QRect& roi)
{
    QImage targetImage = image;
    
    // 如果指定了ROI区域，则裁剪图像
    if (!roi.isNull() && roi.isValid()) {
        targetImage = image.copy(roi);
    }
    
    // 转换为灰度并缩放为8x8像素进行哈希计算
    QImage grayImage = targetImage.convertToFormat(QImage::Format_Grayscale8);
    QImage hashImage = grayImage.scaled(8, 8, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    
    // 计算平均像素值
    qint64 totalValue = 0;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            totalValue += qGray(hashImage.pixel(x, y));
        }
    }
    qint64 avgValue = totalValue / 64;
    
    // 生成64位哈希值
    QString hash;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            int pixelValue = qGray(hashImage.pixel(x, y));
            hash += (pixelValue >= avgValue) ? "1" : "0";
        }
    }
    
    return hash;
}

void arona::loadPositionTemplates()
{
    positionTemplates.clear();

    // 加载位置模板
    QDir dir(":/images/position_templates");
    if (!dir.exists()) {
        appendLog("位置模板目录不存在", "ERROR");
        return;
    }
    QStringList files = dir.entryList(QDir::Files);
    foreach (QString file, files) {
        QString filePath = dir.filePath(file);
        QImage image = QImage(filePath);
        QString hash = calculateImageHash(image);
        QString fileName = file.split(".").first();
        positionTemplates.insert(fileName, hash);
    }

    appendLog(QString("位置模板加载完成，共加载%1个模板").arg(positionTemplates.size()), "SUCCESS");
    foreach (QString filePath, positionTemplates.keys()) {
        qDebug() << "位置模板: " << filePath << ", 哈希值: " << positionTemplates[filePath];
    }
}

void arona::loadpositionReadyTemplates()
{
    positionReadyTemplates.clear();

    // 加载位置就绪模板
    QDir dir(":/images/position_ready");
    if (!dir.exists()) {
        appendLog("位置就绪模板目录不存在", "ERROR");
        return;
    }
    QStringList files = dir.entryList(QDir::Files);
    foreach (QString file, files) {
        QString filePath = dir.filePath(file);
        QImage image = QImage(filePath);
        QString hash = calculateImageHash(image);
        QString fileName = file.split(".").first();
        positionReadyTemplates.insert(fileName, hash);
    }

    appendLog(QString("位置就绪模板加载完成，共加载%1个模板").arg(positionReadyTemplates.size()), "SUCCESS");
}

void arona::loadStudentAvatarTemplates()
{
    binarizedStudentTemplates.clear();
    
    // 背景色 #F3F7F8
    QRgb backgroundColor = qRgb(243, 247, 248);
    
    // 加载学生头像模板
    QDir dir(":/images/student_avatar");
    if (!dir.exists()) {
        appendLog("学生头像模板目录不存在", "ERROR");
        return;
    }
    
    QStringList files = dir.entryList(QDir::Files);
    int loadedCount = 0;
    
    foreach (QString file, files) {
        if (file.endsWith(".png", Qt::CaseInsensitive)) {
            QString filePath = dir.filePath(file);
            QImage image(filePath);
            
            if (!image.isNull()) {
                // 二值化图像
                QVector<bool> binaryData = binarizeImage(image, backgroundColor);
                
                // 去掉文件扩展名作为学生名称
                QString studentName = file.left(file.lastIndexOf('.'));
                
                // 存储二值化数据及尺寸信息
                StudentTemplate tmpl;
                tmpl.binaryData = binaryData;
                tmpl.width = image.width();
                tmpl.height = image.height();
                binarizedStudentTemplates.insert(studentName, tmpl);
                loadedCount++;
            } else {
                appendLog(QString("无法加载学生头像模板: %1").arg(file), "WARNING");
            }
        }
    }
    
    appendLog(QString("学生头像模板加载完成，共加载%1个模板").arg(loadedCount), "SUCCESS");
}

QVector<bool> arona::binarizeImage(const QImage &image, const QRgb &backgroundColor)
{
    // 将图像二值化：背景色为0，其他颜色为1
    // 允许一定的颜色容差（RGB各分量±10）
    const int tolerance = 10;
    
    int width = image.width();
    int height = image.height();
    QVector<bool> binaryData(width * height);
    
    int bgR = qRed(backgroundColor);
    int bgG = qGreen(backgroundColor);
    int bgB = qBlue(backgroundColor);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QRgb pixel = image.pixel(x, y);
            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);
            
            // 检查是否接近背景色
            bool isBackground = (qAbs(r - bgR) <= tolerance) &&
                               (qAbs(g - bgG) <= tolerance) &&
                               (qAbs(b - bgB) <= tolerance);
            
            binaryData[y * width + x] = !isBackground;  // 背景为0，其他为1
        }
    }
    
    return binaryData;
}

int arona::calculateHammingDistance(const QVector<bool> &binary1, const QVector<bool> &binary2)
{
    // 计算两个二值化数据的汉明距离（不同位的数量）
    if (binary1.size() != binary2.size()) {
        return -1;  // 大小不匹配
    }
    
    int distance = 0;
    for (int i = 0; i < binary1.size(); i++) {
        if (binary1[i] != binary2[i]) {
            distance++;
        }
    }
    
    return distance;
}

bool arona::compareImagesByHamming(const QImage &image, const QVector<bool> &templateBinary, 
                                   int width, int height, const QRgb &backgroundColor, double threshold)
{
    // 检查尺寸
    if (image.width() != width || image.height() != height) {
        return false;
    }
    
    // 二值化待识别图像
    QVector<bool> imageBinary = binarizeImage(image, backgroundColor);
    
    // 计算汉明距离
    int distance = calculateHammingDistance(imageBinary, templateBinary);
    
    if (distance < 0) {
        return false;
    }
    
    // 计算相似度 (1 - 汉明距离/总像素数)
    int totalPixels = width * height;
    double similarity = 1.0 - (double)distance / totalPixels;
    
    appendLog(QString("汉明距离: %1, 总像素: %2, 相似度: %3")
             .arg(distance)
             .arg(totalPixels)
             .arg(similarity, 0, 'f', 4), "INFO");
    
    return similarity >= threshold;
}

QString arona::recognizeCurrentPosition(QImage screenshot)
{
    // 循环遍历所有的位置模板
    for (auto it = positionTemplates.begin(); it != positionTemplates.end(); ++it) {
        QString key = it.key();
        QString templateHash = it.value();
        
        // 解析键中的坐标信息 - 格式: "(x,y)描述"
        static const QRegularExpression regex("\\((\\d+),(\\d+)\\)(.*)");
        QRegularExpressionMatch match = regex.match(key);

        if (match.hasMatch()) {
            int x = match.captured(1).toInt();
            int y = match.captured(2).toInt();
            QString description = match.captured(3);

            // 从截图中截取对应的区域 (36x36像素)
            QRect region(x, y, 36, 36);
            qDebug() << "截取区域: " << region;
            
            // 检查区域是否在截图范围内
            if (!screenshot.rect().contains(region)) {
                qDebug() << "区域超出游戏窗口范围，跳过:" << key;
                continue;
            }
            
            // 截取指定区域
            QImage regionImage = screenshot.copy(region);
            if (regionImage.isNull()) {
                qDebug() << "截取区域失败:" << key;
                continue;
            }
            
            // 计算该区域的哈希值
            QString currentHash = calculateImageHash(regionImage);

            // 与模板哈希值进行比较
            if (currentHash == templateHash) {
                qDebug() << "找到匹配的位置模板:" << key;
                qDebug() << "返回描述:" << description;
                return description; // 返回位置信息
            }
            else {
                // 保存截图
                // QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
                // QString screenshotPath = "screenshots/" + QString("position_%1_%2.png").arg(description).arg(timestamp);
                // if (regionImage.save(screenshotPath))
                // {
                //     qDebug() << "截图保存成功:" << screenshotPath;
                // }
                // else
                // {
                //     qDebug() << "截图保存失败:" << screenshotPath;
                // }
            }
        }
    }
    
    // 没有找到匹配的位置
    qDebug() << "未找到匹配的位置模板";
    return QString();
}

void arona::enterCafe1FromHall(HWND hwnd)
{
    // 从大厅进入咖啡厅1
    click(hwnd, BUTTON_HALL_TO_CAFE1.x(), BUTTON_HALL_TO_CAFE1.y());
}

void arona::enterCafe2FromCafe1(HWND hwnd)
{
    // 从咖啡厅1进入咖啡厅2
    click(hwnd, BUTTON_CAFE1_TO_CAFE2.x(), BUTTON_CAFE1_TO_CAFE2.y());
}

void arona::enterCafe1FromCafe2(HWND hwnd)
{
    // 从咖啡厅2进入大厅
    click(hwnd, BUTTON_CAFE2_TO_CAFE1.x(), BUTTON_CAFE2_TO_CAFE1.y());
}

void arona::inviteStudentByName(HWND hwnd, QStringList studentNames)
{
    // 邀请学生进入咖啡厅
    // 点击邀请券，打开邀请界面
    click(hwnd, BUTTON_INVITATION_TICKET.x(), BUTTON_INVITATION_TICKET.y());
    
    // 等待邀请界面就绪
    int retries = 50;
    while (retries > 0) {
        delayMs(100);
        QImage screenshot = captureWindow(hwnd);
        if (isPositionReady(screenshot, INVITATION_INTERFACE_ROI)) {
            break;
        }
        if (shouldStop) {
            appendLog("========== 脚本已停止 ==========917", "WARNING");
            isRunning = false;
            updateStartButtonState();
            return;
        }
        retries--;
    }

    if (retries <= 0) {
        appendLog("邀请界面未就绪，超时退出", "WARNING");
        return;
    }

    QImage screenshot = captureWindow(hwnd);
    if (screenshot.isNull()) {
        appendLog("截图失败", "ERROR");
        return;
    }
    
    // 查找对应学生
    for (int i = 0; i < studentNames.size(); i++) {
        int studentIndex = findStudentInInvitationInterface(screenshot, studentNames[i]);
        if (studentIndex == 0) {
            appendLog(QString("未找到学生: %1").arg(studentNames[i]), "WARNING");
        }

        appendLog(QString("找到学生: %1, 位置: %2").arg(studentNames[i]).arg(studentIndex), "INFO");

        // 点击学生
        click(hwnd, 1150, studentIndex);
        delayMs(1500);

        QImage inviteImage = captureWindow(hwnd);
        QString notice = checkNotice(inviteImage, INVITATION_NOTICE_ROI);
        if (notice == "(921,223)ChangeClothes") {
            appendLog(QString("%1正穿着另一件衣服").arg(studentNames[i]), "SUCCESS");
            click(hwnd, 1317, 260);
        }
        else if (notice == "(921,223)NextRoom") {
            appendLog(QString("%1正在另一个咖啡厅").arg(studentNames[i]), "SUCCESS");
            click(hwnd, 1317, 260);
        }
        else if (notice == "(921,223)Notice") {
            appendLog(QString("邀请%1前来咖啡厅").arg(studentNames[i]), "SUCCESS");
            click(hwnd, 1150, 775);
            delayMs(2500);
            break;
        }
        else {
            appendLog(QString("未识别到邀请通知: %1").arg(notice), "WARNING");
        }
        delayMs(1000);
    }
    
    return;
}

int arona::findStudentInInvitationInterface(QImage image, QString studentName)
{
    // 查找对应学生
    // 从(1100, 280)开始向下搜索#77DEFF颜色
    int studentIndex = 1;
    const int searchX = 1100;
    const int captureX = 732;
    const int startY = 280;
    const int maxY = 830;
    const int studentHeight = 36;
    const int studentWidth = 128;
    const int verticalSpacing = 110; // 从当前y位置向下110像素继续搜索
    const int yOffset = 7;           // 找到标记后，向下偏移7像素截取头像

    // 目标颜色 #77DEFF (RGB: 119, 222, 255)
    QRgb targetColor = qRgb(119, 222, 255);

    int currentY = startY;
    int foundCount = 0;

    appendLog("开始搜索学生头像标记...", "INFO");
    
    // 从预加载的二值化模板中获取学生模板
    if (!binarizedStudentTemplates.contains(studentName)) {
        appendLog(QString("未找到学生的二值化模板: %1").arg(studentName), "ERROR");
        return 0;
    }
    
    StudentTemplate templateData = binarizedStudentTemplates[studentName];
    
    appendLog(QString("已找到学生模板: %1, 尺寸: %2x%3, 二值化数据大小: %4")
             .arg(studentName)
             .arg(templateData.width)
             .arg(templateData.height)
             .arg(templateData.binaryData.size()), "INFO");
    
    // 背景色 #F3F7F8
    QRgb backgroundColor = qRgb(243, 247, 248);

    while (currentY <= maxY)
    {
        // 检查图像边界
        if (currentY < 0 || currentY >= image.height() || searchX >= image.width())
        {
            break;
        }

        // 检查(searchX, currentY)位置的颜色
        QRgb pixelColor = image.pixel(searchX, currentY);

        // 如果找到目标颜色
        if (pixelColor == targetColor)
        {
            // 向下检查连续67个像素是否都是#77DEFF
            bool allMatch = true;
            for (int i = 1; i <= 67; i++)
            {
                if (currentY + i >= image.height())
                {
                    allMatch = false;
                    break;
                }
                QRgb checkColor = image.pixel(searchX, currentY + i);
                if (checkColor != targetColor)
                {
                    allMatch = false;
                    break;
                }
            }

            // 如果连续67个像素都匹配
            if (allMatch)
            {
                foundCount++;

                // 计算截取位置
                int captureY = currentY - yOffset;

                // 检查截取区域是否在图像范围内
                if (captureY >= 0 && captureY + studentHeight <= image.height() &&
                    captureX >= 0 && captureX + studentWidth <= image.width())
                {

                    // 截取128x36像素的学生头像
                    QImage studentImg = image.copy(captureX, captureY, studentWidth, studentHeight);
                    QString studentPath = QString("screenshots/student_avatar%1.png").arg(studentIndex);

                    // 保存截图用于调试
                    if (studentImg.save(studentPath))
                    {
                        appendLog(QString("已保存student_avatar_%1: 标记位置(%2, %3), 截取位置(%2, %4)")
                                      .arg(studentIndex)
                                      .arg(searchX)
                                      .arg(currentY)
                                      .arg(captureY),
                                  "INFO");
                        studentIndex++;
                    }

                    // 使用汉明距离进行比较（二值化 + 汉明距离）
                    // 相似度阈值设为0.90，即允许10%的像素不同
                    if (compareImagesByHamming(studentImg, templateData.binaryData, templateData.width, templateData.height, backgroundColor, 0.90)) {
                        appendLog(QString("找到匹配的学生: %1").arg(studentName), "SUCCESS");
                        return currentY;
                    } else {
                        appendLog(QString("学生头像不匹配: %1").arg(studentName), "INFO");
                    }
                }

                // 从当前位置向下110像素继续搜索
                currentY += verticalSpacing;
            }
            else
            {
                // 如果不是连续67个像素都匹配，继续向下搜索
                currentY++;
            }
        }
        else
        {
            // 颜色不匹配，继续向下搜索
            currentY++;
        }
    }

    return 0;
}

bool arona::compareImagesByOddRows(const QImage &image1, const QImage &image2)
{
    // 逐像素对比两张图片的奇数行
    // 只对比奇数行的所有像素，允许RGB分量各有±5的误差
    
    // 检查图片尺寸是否相同
    if (image1.width() != image2.width() || image1.height() != image2.height()) {
        appendLog(QString("图片尺寸不匹配: (%1x%2) vs (%3x%4)")
                 .arg(image1.width()).arg(image1.height())
                 .arg(image2.width()).arg(image2.height()), "WARNING");
        return false;
    }
    
    int width = image1.width();
    int height = image1.height();
    int comparedPixels = 0;
    int mismatchedPixels = 0;
    const int tolerance = 50;  // RGB分量允许的最大误差
    
    // 遍历所有奇数行（第1, 3, 5, 7...行，索引为0, 2, 4, 6...）
    for (int y = 0; y < height; y += 2) {
        // 遍历该行的所有像素
        for (int x = 0; x < width; x++) {
            QRgb pixel1 = image1.pixel(x, y);
            QRgb pixel2 = image2.pixel(x, y);
            
            comparedPixels++;
            
            // 提取RGB分量
            int r1 = qRed(pixel1);
            int g1 = qGreen(pixel1);
            int b1 = qBlue(pixel1);
            
            int r2 = qRed(pixel2);
            int g2 = qGreen(pixel2);
            int b2 = qBlue(pixel2);
            
            // 计算RGB分量的差异
            int rDiff = qAbs(r1 - r2);
            int gDiff = qAbs(g1 - g2);
            int bDiff = qAbs(b1 - b2);
            
            // 检查每个分量的差异是否在容差范围内
            if (rDiff > tolerance || gDiff > tolerance || bDiff > tolerance) {
                mismatchedPixels++;
                // 如果有任何一个像素的RGB分量超出容差范围，立即返回false
                appendLog(QString("像素不匹配: 位置(%1,%2), RGB1=(%3,%4,%5), RGB2=(%6,%7,%8), 差异=(%9,%10,%11)")
                         .arg(x).arg(y)
                         .arg(r1).arg(g1).arg(b1)
                         .arg(r2).arg(g2).arg(b2)
                         .arg(rDiff).arg(gDiff).arg(bDiff), "DEBUG");
                return false;
            }
        }
    }
    
    appendLog(QString("图片匹配成功: 对比了%1个像素（奇数行），全部在容差范围内（±%2）").arg(comparedPixels).arg(tolerance), "SUCCESS");
    return true;
}

QString arona::checkNotice(QImage screenshot, QRect roi)
{
    QImage noticeImage = screenshot.copy(roi);

    QString hash = calculateImageHash(noticeImage);
    if (positionReadyTemplates.values().contains(hash)) {
        qDebug() << "识别到邀请通知,键: " << positionReadyTemplates.key(hash);
        return positionReadyTemplates.key(hash);
    }

    return "";
}

bool arona::isPositionReady(QImage screenshot, QRect roi)
{
    // 检查位置是否就绪
    QImage positionReady = screenshot.copy(roi);

    // 保存截图
    // QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    // QString screenshotPath = "screenshots/position_ready_" + timestamp + ".png";
    // if (positionReady.save(screenshotPath)) {
    //     appendLog(QString("已保存位置就绪截图: %1").arg(screenshotPath), "SUCCESS");
    // }
    // else {
    //     appendLog("保存位置就绪截图失败", "ERROR");
    // }
    
    QString hash = calculateImageHash(positionReady);
    if (positionReadyTemplates.values().contains(hash)) {
        qDebug() << "位置就绪,键: " << positionReadyTemplates.key(hash);
        return true;
    }

    // 打印哈希值
    // qDebug() << "位置就绪哈希值: " << hash;
    // for (auto it = positionReadyTemplates.begin(); it != positionReadyTemplates.end(); ++it) {
    //     qDebug() << "位置就绪模板键: " << it.key() << " 哈希值: " << it.value();
    // }
    return false;
}

// ==================== 辅助逻辑函数实现（封装重复逻辑） ====================

bool arona::waitForPosition(HWND hwnd, const QString &targetPosition, int maxRetries, int delayMs, int clickX, int clickY)
{
    appendLog(QString("等待进入位置: %1 ").arg(targetPosition).arg(maxRetries), "INFO");
    
    int retries = maxRetries;
    while (retries > 0)
    {
        // 检查是否需要停止
        if (shouldStop) {
            appendLog("========== 脚本已停止 ==========1159", "WARNING");
            isRunning = false;
            updateStartButtonState();
            return false;
        }

        // 点击游戏窗口边缘（防止超时）
        click(hwnd, clickX, clickY);

        // 延时并检查停止信号
        if (!delayMsWithCheck(delayMs)) {
            appendLog("========== 脚本已停止 ==========1179", "WARNING");
            isRunning = false;
            updateStartButtonState();
            return false;
        }
        
        // 截图并识别当前位置
        QImage screenshot = captureWindow(hwnd);
        QString currentPosition = recognizeCurrentPosition(screenshot);
        
        if (currentPosition == targetPosition)
        {
            appendLog(QString("进入%1成功").arg(targetPosition), "SUCCESS");
            return true;
        }
        
        retries--;
    }
    
    // 超时失败
    appendLog(QString("进入%1失败（超时）").arg(targetPosition), "ERROR");
    isRunning = false;
    updateStartButtonState();
    return false;
}

bool arona::adjustCafeView(HWND hwnd, int scrollX, int scrollY, int scrollCount)
{
    // appendLog(QString("开始调整咖啡厅视角（滚动%1次）").arg(scrollCount), "INFO");
    
    // 唤醒游戏窗口
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    delayMs(200);
    
    // 按下CTRL键
    pressKeyGlobal(VK_CONTROL, 1);
    
    if (!delayMsWithCheck(500)) {
        // pressKeyGlobal(VK_CONTROL, 0);
        appendLog("========== 脚本已停止 ==========1210", "WARNING");
        isRunning = false;
        updateStartButtonState();
        return false;
    }
    
    // 循环滚动鼠标滚轮
    for (int i = 0; i < scrollCount; i++)
    {
        if (shouldStop) {
            // pressKeyGlobal(VK_CONTROL, 0);
            appendLog("========== 脚本已停止 ==========1221", "WARNING");
            isRunning = false;
            updateStartButtonState();
            return false;
        }
        
        scroll(hwnd, scrollX, scrollY, -3);
        
        if (!delayMsWithCheck(200)) {
            // pressKeyGlobal(VK_CONTROL, 0);
            appendLog("========== 脚本已停止 ==========1231", "WARNING");
            isRunning = false;
            updateStartButtonState();
            return false;
        }
    }
    
    // 释放CTRL键
    pressKeyGlobal(VK_CONTROL, 0);
    
    // appendLog("咖啡厅视角调整完成", "SUCCESS");
    return true;
}

void arona::adjustCafePosition(HWND hwnd)
{
    // appendLog("开始调整咖啡厅位置（3次拖拽）", "INFO");
    
    drag(hwnd, 1680, 240, 130, 1040, 1000);
    drag(hwnd, 400, 400, 1900, 900, 1000);
    drag(hwnd, 1080, 1040, 1680, 240, 1000);
    
    // appendLog("咖啡厅位置调整完成", "SUCCESS");
}

void arona::patStudents(HWND hwnd, int rounds)
{
    for (int i = 0; i < rounds; i++)
    {
        if (shouldStop) {
            appendLog("========== 脚本已停止 ==========1263", "WARNING");
            isRunning = false;
            updateStartButtonState();
            return;
        }
        
        // appendLog(QString("摸头第%1/%2轮").arg(i + 1).arg(rounds), "INFO");
        clickGrid(hwnd, 200, 240, 1900, 880, 65, 20);
        delayMs(1000);
    }
}

void arona::closeGameWindow(HWND hwnd)
{
    // 找到父窗口，关闭游戏
    HWND parentHwnd = GetParent(hwnd);
    if (parentHwnd != NULL)
    {
        // 获取父窗口标题
        wchar_t title[256];
        GetWindowTextW(parentHwnd, title, 256);
        QString titleStr = QString::fromWCharArray(title);
        appendLog(QString("父窗口标题: %1").arg(titleStr), "INFO");
        
        if (titleStr == "大号" || titleStr == "最大号" || titleStr == "小七")
        {
            // 点击关闭按钮
            click(parentHwnd, 703, 30);
            appendLog(QString("已关闭窗口: %1").arg(titleStr), "SUCCESS");
        }
        else
        {
            appendLog(QString("父窗口标题: %1，不在关闭列表中").arg(titleStr), "WARNING");
        }
    }
    else
    {
        appendLog("未找到父窗口", "WARNING");
    }
}

#if DEBUG_MODE
void arona::onDebugTypeChanged(int index)
{
    // 根据调试类型显示/隐藏对应的参数控件
    clickDebugWidget->setVisible(false);
    keyDebugWidget->setVisible(false);
    
    if (index == 1) {  // 点击调试
        clickDebugWidget->setVisible(true);
    } else if (index == 2) {  // 按键调试
        keyDebugWidget->setVisible(true);
    }
    // index == 0 (截图调试) 不需要额外参数，都隐藏
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
    } else if (debugType == 2) {
        // 按键调试
        keyDebug();
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

    QImage image = captureWindow(hwnd);
    if (image.isNull()) {
        appendLog("截图失败", "ERROR");
        return;
    }
    
    // 创建screenshots目录
    QDir dir;
    if (!dir.exists("screenshots")) {
        dir.mkdir("screenshots");
    }
    
    // ==================== 新增功能：自动截取学生头像 ====================
    
    // 从(1100, 280)开始向下搜索#77DEFF颜色
    int studentIndex = 1;
    const int searchX = 1100;
    const int captureX = 732;
    const int startY = 280;
    const int maxY = 830;
    const int studentHeight = 36;
    const int studentWidth = 128;
    const int verticalSpacing = 110;  // 从当前y位置向下110像素继续搜索
    const int yOffset = 7;  // 找到标记后，向上偏移7像素截取头像
    
    // 目标颜色 #77DEFF (RGB: 119, 222, 255)
    QRgb targetColor = qRgb(119, 222, 255);
    
    int currentY = startY;
    int foundCount = 0;
    
    appendLog("开始搜索学生头像标记...", "INFO");
    
    while (currentY <= maxY) {
        // 检查图像边界
        if (currentY < 0 || currentY >= image.height() || searchX >= image.width()) {
            break;
        }
        
        // 检查(searchX, currentY)位置的颜色
        QRgb pixelColor = image.pixel(searchX, currentY);
        
        // 如果找到目标颜色
        if (pixelColor == targetColor) {
            // 向下检查连续67个像素是否都是#77DEFF
            bool allMatch = true;
            for (int i = 1; i <= 67; i++) {
                if (currentY + i >= image.height()) {
                    allMatch = false;
                    break;
                }
                QRgb checkColor = image.pixel(searchX, currentY + i);
                if (checkColor != targetColor) {
                    allMatch = false;
                    break;
                }
            }
            
            // 如果连续67个像素都匹配
            if (allMatch) {
                foundCount++;
                
                // 计算截取位置
                int captureY = currentY - yOffset;
                
                // 检查截取区域是否在图像范围内
                if (captureY >= 0 && captureY + studentHeight <= image.height() &&
                    captureX >= 0 && captureX + studentWidth <= image.width()) {
                    
                    // 截取128x40像素的学生头像
                    QImage studentImg = image.copy(captureX, captureY, studentWidth, studentHeight);
                    QString studentPath = QString("screenshots/student%1.png").arg(studentIndex);
                    
                    if (studentImg.save(studentPath)) {
                        appendLog(QString("已保存student%1: 标记位置(650, %2), 截取位置(650, %3)")
                                 .arg(studentIndex).arg(currentY).arg(captureY), "SUCCESS");
                        studentIndex++;
                    } else {
                        appendLog(QString("保存student%1失败").arg(studentIndex), "ERROR");
                    }
                }
                
                // 从当前位置向下110像素继续搜索
                currentY += verticalSpacing;
            } else {
                // 如果不是连续67个像素都匹配，继续向下搜索
                currentY++;
            }
        } else {
            // 颜色不匹配，继续向下搜索
            currentY++;
        }
    }
    
    appendLog(QString("学生头像搜索完成，共找到%1个标记，保存了%2张头像")
             .arg(foundCount).arg(studentIndex - 1), "SUCCESS");
    
    // ==================== 保存完整截图（可选） ====================
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString screenshotPath = QString("screenshots/fullscreen_%1.png").arg(timestamp);
    
    if (image.save(screenshotPath)) {
        appendLog(QString("完整截图已保存: %1").arg(screenshotPath), "INFO");
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

    QImage image = captureWindow(hwnd);
    QImage clickArea = image.copy(x, y, 36, 36);
    // 保存点击区域图像
    QString position = QString("(%1,%2)").arg(x).arg(y);
    QString clickAreaPath = "screenshots/" + QString("%1clickArea.png").arg(position);
    if (clickArea.save(clickAreaPath)) {
        appendLog(QString("点击区域图像保存: %1").arg(clickAreaPath), "SUCCESS");
    } else {
        appendLog("保存点击区域图像失败", "ERROR");
    }
    
    // 构造lParam (x和y坐标)
    LPARAM lParam = MAKELPARAM(x, y);
    
    // 发送鼠标按下消息
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
    
    // 延迟一小段时间后发送鼠标抬起消息
    QTimer::singleShot(50, [hwnd, lParam, this]() {
        PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
        // appendLog(QString("已发送点击消息到目标窗口，坐标: (%1, %2)")
        //          .arg(clickXSpinBox->value())
        //          .arg(clickYSpinBox->value()), "SUCCESS");
    });
    
    appendLog(QString("正在模拟点击，坐标: (%1, %2)").arg(x).arg(y), "INFO");
}

void arona::keyDebug()
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
    
    // 获取选中的按键
    int vkCode = keySelectComboBox->currentData().toInt();
    QString keyName = keySelectComboBox->currentText();
    
    appendLog(QString("正在模拟按键: %1 (VK=0x%2)")
             .arg(keyName)
             .arg(vkCode, 2, 16, QChar('0')), "INFO");

    // 唤醒游戏窗口
    click(hwnd, 820, 800);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    
    delayMs(200);
    
    // 按下按键
    // pressKey(hwnd, vkCode, 1);
    pressKeyGlobal(vkCode, 1);
    delayMs(100);
    pressKeyGlobal(vkCode, 0);
    delayMs(100);
    pressKeyGlobal(vkCode, 1);
    delayMs(100);
    pressKeyGlobal(vkCode, 0);
    delayMs(100);
    pressKeyGlobal(vkCode, 1);
    delayMs(100);
    pressKeyGlobal(vkCode, 0);
    delayMs(500);
    pressKeyGlobal(vkCode, 1);
    
    // 延迟1000ms后抬起按键
    QTimer::singleShot(500, [hwnd, vkCode, keyName, this]() {
        // pressKey(hwnd, vkCode, 0);
        pressKeyGlobal(vkCode, 0);
        appendLog(QString("已完成按键模拟: %1").arg(keyName), "SUCCESS");
    });
}
#endif

// ==================== 脚本控制函数实现 ====================

void arona::startScript()
{
    // 检查是否已经在运行
    if (isRunning) {
        appendLog("脚本已经在运行中", "WARNING");
        return;
    }
    
    // 统计有效窗口数量
    int validWindowCount = 0;
    for (int i = 0; i < 3; i++) {
        if (gameHandles[i] != NULL && IsWindow(gameHandles[i])) {
            validWindowCount++;
        }
    }
    
    if (validWindowCount == 0) {
        appendLog("请先抓取至少一个窗口句柄", "ERROR");
        return;
    }
    
    // 设置运行状态
    isRunning = true;
    shouldStop = false;
    updateStartButtonState();
    
    appendLog("========== 脚本启动 ==========", "SUCCESS");
    appendLog(QString("检测到%1个有效窗口，将依次执行").arg(validWindowCount), "INFO");
    
    // 使用QTimer异步执行脚本，避免阻塞UI
    QTimer::singleShot(100, this, &arona::executeAllWindows);
}

void arona::stopScript()
{
    if (!isRunning) {
        appendLog("脚本未在运行", "WARNING");
        return;
    }
    
    shouldStop = true;
    appendLog("正在停止脚本...", "WARNING");
}

void arona::updateStartButtonState()
{
    if (isRunning) {
        // 运行中 - 显示为停止按钮（红色）
        startButton->setText("停止");
        startButton->setStyleSheet("QPushButton { "
                                  "background-color: rgba(244, 67, 54, 200); "
                                  "color: white; "
                                  "border: 2px solid rgba(244, 67, 54, 255); "
                                  "border-radius: 5px; "
                                  "font-weight: bold; "
                                  "font-size: 14pt; "
                                  "padding: 5px; "
                                  "} "
                                  "QPushButton:hover { "
                                  "background-color: #E53935; "
                                  "} "
                                  "QPushButton:pressed { "
                                  "background-color: #C62828; "
                                  "}");
    } else {
        // 停止状态 - 显示为启动按钮（蓝色）
        startButton->setText("启动");
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
    }
}

void arona::executeScript(HWND hwnd, QString titleStr)
{
    // ==================== 初始化 ====================
    
    
    // 检查停止信号
    if (shouldStop) {
        appendLog("========== 脚本已停止 ==========1682", "WARNING");
        isRunning = false;
        updateStartButtonState();
        return;
    }

    // 置顶游戏窗口
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    // ==================== 等待进入大厅 ====================
    if (!waitForPosition(hwnd, "Hall", 20, 4000, 120, 640)) {
        return;  // 进入失败，已在函数内处理
    }

    // 检测当前时间，精确到小时
    QTime currentTime = QTime::currentTime();
    int currentHour = currentTime.hour();
    // 在凌晨4点到6点之间进行扫荡
    if(currentHour >= 4 && currentHour <= 6)
    {
        // 扫荡
        if (!waitForPosition(hwnd, "Opration", 10, 1500, 1880, 890))
        {
            return;
        }
        appendLog("阿罗娜，进入操作界面", "INFO");
        delayMs(1500);
        if (!waitForPosition(hwnd, "Task", 10, 1500, 1230, 370))
        {
            return;
        }
        delayMs(1500);
        appendLog("阿罗娜，进入任务界面", "INFO");
        // 前往最后一关
        int waitCount = 0;
        while (waitCount < 30)
        {
            click(hwnd, 1840, 520);
            delayMs(300);
            QImage screenshot = captureWindow(hwnd);
            if (isPositionReady(screenshot, TASK_END_ROI))
            {
                click(hwnd, 1595, 215);
                delayMs(300);
                break;
            }
            waitCount++;
        }
        // 确保进入困难关卡
        QImage screenshot = captureWindow(hwnd);
        if (isPositionReady(screenshot, HARD_TASK_ROI))
        {
            delayMs(300);
            click(hwnd, 1595, 215);
        }
        if (titleStr == "大号")
        {
            doTask(hwnd, 4, 2);
        }
        else if (titleStr == "最大号")
        {
            doTask(hwnd, 2, 1);
        }
        else if (titleStr == "小七")
        {
            doTask(hwnd, 18, 2);
        }
        else
        {
            appendLog("阿罗娜，不支持的窗口", "ERROR");
        }
        appendLog("阿罗娜，完成任务", "INFO");
        delayMs(1000);

        // 返回大厅
        if (!waitForPosition(hwnd, "Hall", 30, 1000, 1855, 10))
        {
            return;
        }
    }

    // 调试暂停
    // shouldStop = true;
    // appendLog("========== 脚本已停止 ==========1741", "WARNING");
    // isRunning = false;
    // updateStartButtonState();
    // return;

    // ==================== 咖啡厅1 ====================
    appendLog("阿罗娜，前往咖啡厅1", "INFO");
    
    // 进入咖啡厅1
    enterCafe1FromHall(hwnd);
    if (!waitForPosition(hwnd, "Cafe1", 10, 1500, 150, 1045)) {
        return;
    }

    // 如果是在4点到12点之间，在咖啡厅1邀请学生
    if (currentHour >= 4 && currentHour <= 12)
    {
        inviteStudentToCafe(hwnd, titleStr);
    }

    if (!isRunning) {
        appendLog("========== 脚本已停止 ==========1716", "WARNING");
        updateStartButtonState();
        return;
    }
    
    // 调整视角和位置
    if (!adjustCafeView(hwnd, 1500, 600, 12)) {
        return;
    }
    adjustCafePosition(hwnd);
    
    // 摸头
    appendLog(QString("阿罗娜，开始摸头（循环3轮）"), "INFO");
    patStudents(hwnd, 3);
    
    if (!delayMsWithCheck(500)) {
        appendLog("========== 脚本已停止 ==========1731", "WARNING");
        isRunning = false;
        updateStartButtonState();
        return;
    }

    // ==================== 咖啡厅2 ====================
    appendLog("阿罗娜，前往咖啡厅2", "INFO");
    
    // 进入咖啡厅2
    enterCafe2FromCafe1(hwnd);
    if (!waitForPosition(hwnd, "Cafe2", 10, 1500, 150, 1045)) {
        return;
    }

    // 如果是16点到0点之间，在咖啡厅2邀请学生
    if (currentHour >= 16 && currentHour <= 23)
    {
        inviteStudentToCafe(hwnd, titleStr);
    }
    
    // 调整视角和位置
    if (!adjustCafeView(hwnd, 1500, 600, 12)) {
        return;
    }
    adjustCafePosition(hwnd);
    
    // 摸头
    appendLog(QString("阿罗娜，开始摸头（循环3轮）"), "INFO");
    patStudents(hwnd, 3);

    // ==================== 关闭游戏 ====================
    closeGameWindow(hwnd);
    
    // ==================== 完成 ====================
    appendLog("========== 脚本执行完成 ==========1758", "SUCCESS");
    if (shouldStop)
    {
        isRunning = false;
        updateStartButtonState();
    }
}

// ==================== 工具函数实现 ====================
void arona::inviteStudentToCafe(HWND hwnd, QString titleStr)
{
    // 检查邀请券是否就绪
    QImage screenshot = captureWindow(hwnd);
    if (isPositionReady(screenshot, INVITATION_TICKET_ROI))
    {
        appendLog("邀请券就绪，准备邀请学生", "SUCCESS");

        QStringList studentNames;
        if (titleStr == "最大号")
        {
            studentNames << "星野（泳装）" << "日奈（礼服）" << "优香（体操服）" << "宫子" << "晴（露营）";
        }
        else if (titleStr == "大号")
        {
            studentNames << "未花" << "花子（泳装）" << "星野（泳装）" << "玲纱" << "爱露";
        }
        else if (titleStr == "小七")
        {
            studentNames << "日奈（礼服）" << "未花" << "花子（泳装）" << "星野（泳装）" << "爱丽丝（女仆）";
        }
        // 邀请学生
        inviteStudentByName(hwnd, studentNames);
    }
    else
    {
        appendLog("邀请券未就绪", "WARNING");
    }
    return;
}

// 关闭声音函数
void arona::muteSound(HWND hwnd)
{
    click(GetParent(hwnd), 1531, 30);
    delayMs(1000);
    click(hwnd, 715, 136);
    delayMs(1000);
    click(GetParent(hwnd), 1120, 30);
    delayMs(1000);
}

void arona::doTask(HWND hwnd, int taskIndex, int subTaskIndex)
{
    // 点击固定次数，进入指定关卡
    for (int i = 0; i < taskIndex; i++)
    {
        click(hwnd, 70, 540);
        delayMs(300);
    }
    // 进入指定关卡
    click(hwnd, 1680, 370 + subTaskIndex * 170);
    delayMs(1000);

    // 设定最大挑战次数
    for (int i = 0; i < 3; i++)
    {
        click(hwnd, 1525, 500);
        delayMs(300);
    }

    // 点击开始扫荡
    click(hwnd, 1400, 630);
    delayMs(1000);

    // 点击确认
    click(hwnd, 1140, 750);
}

void arona::delayMs(int milliseconds)
{
    // 使用QEventLoop实现无阻塞延时
    // 这样可以保持UI响应，不会冻结界面
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

bool arona::delayMsWithCheck(int milliseconds)
{
    // 带停止检查的延时函数
    // 每100ms检查一次是否需要停止
    // 返回false表示需要停止，返回true表示延时完成
    
    int elapsed = 0;
    int checkInterval = 100;  // 每100ms检查一次
    
    while (elapsed < milliseconds) {
        // 检查是否需要停止
        if (shouldStop) {
            return false;
        }
        
        // 计算本次需要延时的时间
        int delayTime = qMin(checkInterval, milliseconds - elapsed);
        
        // 延时
        QEventLoop loop;
        QTimer::singleShot(delayTime, &loop, &QEventLoop::quit);
        loop.exec();
        
        elapsed += delayTime;
    }
    
    return true;
}

void arona::click(HWND hwnd, int x, int y)
{
    // 检查窗口是否有效
    if (!IsWindow(hwnd)) {
        appendLog("窗口句柄无效", "ERROR");
        return;
    }
    
    // 构造lParam (x和y坐标)
    LPARAM lParam = MAKELPARAM(x, y);
    
    // 发送鼠标按下消息
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
    
    // 延迟50ms后发送鼠标抬起消息，模拟真实点击
    delayMs(50);
    PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
    
    // appendLog(QString("已点击坐标: (%1, %2)").arg(x).arg(y), "INFO");
}

void arona::clickGrid(HWND hwnd, int x1, int y1, int x2, int y2, int spacing, int delay)
{
    // 检查窗口是否有效
    if (!IsWindow(hwnd)) {
        appendLog("窗口句柄无效", "ERROR");
        return;
    }
    
    // 确保x1 <= x2, y1 <= y2
    if (x1 > x2) qSwap(x1, x2);
    if (y1 > y2) qSwap(y1, y2);
    
    // 计算网格点数量
    int gridWidth = (x2 - x1) / spacing + 1;
    int gridHeight = (y2 - y1) / spacing + 1;
    int totalPoints = gridWidth * gridHeight;
    
    int clickedCount = 0;
    
    // 遍历Y坐标（从上到下）
    for (int y = y1; y <= y2; y += spacing) {
        // 检查是否需要停止
        if (shouldStop) {
            appendLog(QString("摸头已中断，已完成%1轮").arg(clickedCount), "WARNING");
            return;
        }
        
        // 遍历X坐标（从左到右）
        for (int x = x1; x <= x2; x += spacing) {
            // 检查是否需要停止
            if (shouldStop) {
                appendLog(QString("摸头已中断，已完成%1轮").arg(clickedCount), "WARNING");
                return;
            }
            
            // 构造lParam (x和y坐标)
            LPARAM lParam = MAKELPARAM(x, y);
            
            // 发送鼠标按下和抬起消息
            PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
            delayMs(5);  // 按下和抬起之间的短暂延时
            PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
            
            clickedCount++;
            
            // 点击间隔延时（带停止检查）
            if (delay > 0) {
                if (!delayMsWithCheck(delay)) {
                    // 延时期间收到停止信号
                    appendLog(QString("地毯式点击已中断，已完成%1/%2个点").arg(clickedCount).arg(totalPoints), "WARNING");
                    return;
                }
            }
        }
    }
}

void arona::moveMouse(int x, int y)
{
    // 移动真实鼠标到屏幕坐标
    SetCursorPos(x, y);
    appendLog(QString("鼠标已移动到屏幕坐标: (%1, %2)").arg(x).arg(y), "INFO");
}

void arona::moveMouseToWindow(HWND hwnd, int x, int y)
{
    // 检查窗口是否有效
    if (!IsWindow(hwnd)) {
        appendLog("窗口句柄无效", "ERROR");
        return;
    }
    
    // 获取窗口位置
    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        appendLog("获取窗口位置失败", "ERROR");
        return;
    }
    
    // 计算屏幕坐标 = 窗口左上角 + 相对坐标
    int screenX = rect.left + x;
    int screenY = rect.top + y;
    
    // 移动鼠标
    SetCursorPos(screenX, screenY);
    appendLog(QString("鼠标已移动到窗口坐标: (%1, %2)").arg(x).arg(y), "INFO");
}

void arona::drag(HWND hwnd, int startX, int startY, int endX, int endY, int duration)
{
    // 检查窗口是否有效
    if (!IsWindow(hwnd)) {
        appendLog("窗口句柄无效", "ERROR");
        return;
    }
    
    // 计算拖动步数（每10ms移动一次）
    int steps = duration / 10;
    if (steps < 1) steps = 1;
    
    // 计算每步的移动距离
    double deltaX = static_cast<double>(endX - startX) / steps;
    double deltaY = static_cast<double>(endY - startY) / steps;
    
    // 发送鼠标按下消息到起始位置
    LPARAM startLParam = MAKELPARAM(startX, startY);
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, startLParam);
    
    // appendLog(QString("开始拖动: (%1, %2) -> (%3, %4)")
    //          .arg(startX).arg(startY).arg(endX).arg(endY), "INFO");
    
    delayMs(50);  // 等待按下消息处理
    
    // 逐步移动鼠标
    for (int i = 1; i <= steps; ++i) {
        int currentX = startX + static_cast<int>(deltaX * i);
        int currentY = startY + static_cast<int>(deltaY * i);
        
        LPARAM currentLParam = MAKELPARAM(currentX, currentY);
        PostMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, currentLParam);
        
        delayMs(10);  // 每步延时10ms
    }
    
    // 发送鼠标抬起消息到终点位置
    LPARAM endLParam = MAKELPARAM(endX, endY);
    PostMessage(hwnd, WM_LBUTTONUP, 0, endLParam);
    
    // appendLog(QString("拖动完成: (%1, %2) -> (%3, %4)")
    //          .arg(startX).arg(startY).arg(endX).arg(endY), "SUCCESS");
}

void arona::scroll(HWND hwnd, int x, int y, int delta)
{
    // 检查窗口是否有效
    if (!IsWindow(hwnd)) {
        appendLog("窗口句柄无效", "ERROR");
        return;
    }
    
    // Windows滚轮标准单位为120
    // delta > 0: 向上滚动
    // delta < 0: 向下滚动
    // 通常传入的delta是滚动的"格数"，需要乘以WHEEL_DELTA(120)
    int wheelDelta = delta * WHEEL_DELTA;
    
    // 构造WPARAM
    // 高位字包含滚动量，低位字包含按键状态（通常为0）
    WPARAM wParam = MAKEWPARAM(0, wheelDelta);
    
    // 构造LPARAM (x和y坐标)
    LPARAM lParam = MAKELPARAM(x, y);
    
    // 发送滚轮消息
    PostMessage(hwnd, WM_MOUSEWHEEL, wParam, lParam);
    
    QString direction = (delta > 0) ? "向上" : "向下";
    // appendLog(QString("已发送滚轮消息: %1滚动%2格，坐标: (%3, %4)")
    //          .arg(direction).arg(qAbs(delta)).arg(x).arg(y), "INFO");
}

void arona::pressKey(HWND hwnd, int vkCode, bool press)
{
    // 检查窗口是否有效
    if (!IsWindow(hwnd)) {
        appendLog("窗口句柄无效", "ERROR");
        return;
    }
    
    // 使用PostMessage发送键盘消息到指定窗口
    if (press) {
        // 按下键 - 发送WM_KEYDOWN消息
        // lParam包含扫描码、重复次数等信息
        // 这里简化处理，重复次数为1，扫描码为0
        LPARAM lParam = 0x00000001;  // 重复次数=1, 扫描码=0, 扩展键=0, 上下文码=0, 之前状态=0, 转换状态=0
        PostMessage(hwnd, WM_KEYDOWN, vkCode, lParam);
        appendLog(QString("向窗口发送按键按下: VK_CODE=0x%1(%2)").arg(vkCode, 2, 16, QChar('0')).arg(vkCode), "INFO");
    } else {
        // 抬起键 - 发送WM_KEYUP消息
        // lParam的bit30=1(之前按下), bit31=1(正在释放)
        LPARAM lParam = 0xC0000001;  // 重复次数=1, 之前状态=1, 转换状态=1
        PostMessage(hwnd, WM_KEYUP, vkCode, lParam);
        appendLog(QString("向窗口发送按键抬起: VK_CODE=0x%1(%2)").arg(vkCode, 2, 16, QChar('0')).arg(vkCode), "INFO");
    }
}

void arona::pressKeyGlobal(int vkCode, bool press)
{
    // 使用keybd_event发送全局键盘事件（不指定窗口）
    // 注意：这会发送到当前有焦点的窗口
    if (press) {
        // 按下键
        keybd_event(vkCode, 0, 0, 0);
        // appendLog(QString("全局按键按下: VK_CODE=0x%1(%2)").arg(vkCode, 2, 16, QChar('0')).arg(vkCode), "INFO");
    } else {
        // 抬起键
        keybd_event(vkCode, 0, KEYEVENTF_KEYUP, 0);
        // appendLog(QString("全局按键抬起: VK_CODE=0x%1(%2)").arg(vkCode, 2, 16, QChar('0')).arg(vkCode), "INFO");
    }
}

// ==================== 多窗口和定时功能实现 ====================

void arona::onTimerSettingsButtonClicked()
{
    // 打开定时设置对话框
    // 先将当前设置加载到dialog
    timerDialog->setTimerEnabled(timerEnabled);
    timerDialog->setScheduledTimes(scheduledTimes);
    
    // 显示对话框
    if (timerDialog->exec() == QDialog::Accepted) {
        // 用户点击了保存按钮，更新设置
        timerEnabled = timerDialog->isTimerEnabled();
        scheduledTimes = timerDialog->getScheduledTimes();
        
        appendLog(QString("定时设置已更新: %1启用, 共%2个定时点")
                 .arg(timerEnabled ? "已" : "未")
                 .arg(scheduledTimes.size()), "INFO");
                 
        // 如果启用了定时功能，显示所有定时点
        if (timerEnabled && !scheduledTimes.isEmpty()) {
            QString timesStr;
            for (const QTime &time : scheduledTimes) {
                if (!timesStr.isEmpty()) timesStr += ", ";
                timesStr += time.toString("HH:mm");
            }
            appendLog(QString("定时时间: %1").arg(timesStr), "INFO");
        }
    } else {
        // 用户点击了取消按钮
        appendLog("定时设置未更改", "INFO");
    }
}

void arona::onSchedulerTimerTimeout()
{
    // 每30秒检查一次是否需要执行定时任务
    if (!timerEnabled) {
        return;  // 定时功能未启用
    }
    
    if (isRunning) {
        return;  // 脚本正在运行，不重复启动
    }

    // 检查当前时间
    checkAndExecuteScheduledTasks();
}

void arona::checkAndExecuteScheduledTasks()
{
    QTime currentTime = QTime::currentTime();
    QString currentTimeKey = currentTime.toString("HH:mm");
    
    // 检查是否需要执行
    for (const QTime &scheduledTime : scheduledTimes) {
        QString scheduledKey = scheduledTime.toString("HH:mm");
        
        // 检查时间是否匹配（精确到分钟）
        if (scheduledKey == currentTimeKey) {
            // 检查今天是否已经执行过
            if (executedToday.contains(scheduledKey)) {
                continue;  // 今天已经执行过这个时间点的任务
            }
            
            // 记录已执行
            executedToday.insert(scheduledKey);
            
            // 执行脚本
            appendLog(QString("========== 定时任务触发: %1 ==========").arg(scheduledKey), "WARNING");
            startScript();
            
            return;  // 一次只执行一个任务
        }
    }
    
    // 如果是新的一天（00:00），清空执行记录
    if (currentTimeKey == "00:00") {
        executedToday.clear();
        appendLog("新的一天开始，定时任务记录已重置", "INFO");
    }
}

void arona::executeAllWindows()
{
    // 依次对所有窗口执行脚本
    appendLog("========== 开始多窗口执行 ==========", "INFO");

    int validHandleCount = 0;
    for (int i = 0; i < 3; i++) {
        if (gameHandles[i] != NULL && IsWindow(gameHandles[i])) {
            validHandleCount++;
            // 如果窗口已最小化，则恢复
            if (IsIconic(GetParent(gameHandles[i]))) {
                appendLog(QString("窗口%1已最小化，正在恢复").arg(i), "INFO");
                ShowWindow(GetParent(gameHandles[i]), SW_RESTORE);
                delayMs(200);
                appendLog(QString("窗口%1恢复成功").arg(i), "INFO");
            }
            // 启动游戏
            click(gameHandles[i], 1450, 200);
        }
    }

    // 获取当前时间，在凌晨0点到7点之间关闭声音
    QTime currentTime = QTime::currentTime();
    int currentHour = currentTime.hour();
    if (currentHour >= 0 && currentHour <= 7)
    {
        appendLog("在凌晨0点到6点之间关闭声音", "INFO");
        for (int i = 0; i < 3; i++) {
            if (gameHandles[i] != NULL && IsWindow(gameHandles[i])) {
                // 关闭声音
                muteSound(gameHandles[i]);
            }
        }
    }
    
    if (validHandleCount == 0) {
        appendLog("没有有效的游戏窗口句柄", "ERROR");
        isRunning = false;
        updateStartButtonState();
        return;
    }
    
    appendLog(QString("找到%1个有效窗口，开始依次执行").arg(validHandleCount), "INFO");
    
    // 依次执行每个窗口
    for (int i = 0; i < 3; i++) {
        if (shouldStop) {
            appendLog("========== 脚本已停止 ==========2139", "WARNING");
            isRunning = false;
            updateStartButtonState();
            return;
        }
        
        HWND hwnd = gameHandles[i];
        if (hwnd == NULL || !IsWindow(hwnd)) {
            continue;  // 跳过无效句柄
        }
        
        // 获取父窗口标题
        wchar_t title[256];
        GetWindowTextW(GetParent(hwnd), title, 256);
        QString titleStr = QString::fromWCharArray(title);
        currentHandleIndex = i;
        appendLog(QString("---------- 正在处理窗口：%1 ----------").arg(titleStr), "INFO");
        
        // 执行脚本主逻辑
        executeScript(hwnd, titleStr);
        
        if (shouldStop) {
            break;
        }
        
        // 窗口之间延时
        if (i < 2 && !shouldStop) {
            appendLog("等待进入下一个窗口...", "INFO");
            if (!delayMsWithCheck(5000)) {
                break;
            }
        }
    }
    
    appendLog("========== 多窗口执行完成 ==========", "SUCCESS");
    isRunning = false;
    updateStartButtonState();
}

