#include "studentinvitedialog.h"
#include <QDir>
#include <QFileInfo>

StudentInviteDialog::StudentInviteDialog(QWidget *parent)
    : QDialog(parent)
{
    // 定义窗口标题列表
    windowTitles << "最大号" << "大号" << "小七";
    
    // 加载可用学生列表
    loadAvailableStudents();
    
    setupUi();
}

StudentInviteDialog::~StudentInviteDialog()
{
}

void StudentInviteDialog::loadAvailableStudents()
{
    // 从 images/student_avatar 文件夹读取所有学生名字
    availableStudents.clear();
    
    // 使用资源路径
    QDir avatarDir(":/images/student_avatar");
    if (avatarDir.exists()) {
        QStringList filters;
        filters << "*.png";
        QFileInfoList fileList = avatarDir.entryInfoList(filters, QDir::Files);
        
        for (const QFileInfo &fileInfo : fileList) {
            // 文件名（不含扩展名）就是学生名字
            availableStudents.append(fileInfo.baseName());
        }
    }
    
    // 排序
    availableStudents.sort();
}

void StudentInviteDialog::setupUi()
{
    this->setWindowTitle("邀请学生设置");
    this->setFixedSize(650, 580);
    this->setModal(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 说明标签
    QLabel *infoLabel = new QLabel("为每个窗口设置要邀请的学生列表（按优先级排序）", this);
    infoLabel->setStyleSheet("font-size: 10pt; color: #666;");
    mainLayout->addWidget(infoLabel);
    
    // 创建标签页控件
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet("QTabWidget::pane { "
                            "border: 2px solid #FF9800; "
                            "border-radius: 5px; "
                            "} "
                            "QTabBar::tab { "
                            "background-color: rgba(255, 152, 0, 100); "
                            "color: #333; "
                            "padding: 8px 20px; "
                            "margin-right: 2px; "
                            "border-top-left-radius: 5px; "
                            "border-top-right-radius: 5px; "
                            "} "
                            "QTabBar::tab:selected { "
                            "background-color: #FF9800; "
                            "color: white; "
                            "font-weight: bold; "
                            "}");
    
    // 为每个窗口标题创建标签页
    for (const QString &title : windowTitles) {
        QWidget *tabPage = createTabPage(title);
        tabWidget->addTab(tabPage, title);
    }
    
    mainLayout->addWidget(tabWidget);
    
    // 底部按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    saveButton = new QPushButton("保存", this);
    saveButton->setMinimumHeight(35);
    saveButton->setStyleSheet("QPushButton { "
                             "background-color: rgba(76, 175, 80, 200); "
                             "color: white; "
                             "border: 2px solid rgba(76, 175, 80, 255); "
                             "border-radius: 5px; "
                             "font-weight: bold; "
                             "padding: 5px 20px; "
                             "} "
                             "QPushButton:hover { "
                             "background-color: #4CAF50; "
                             "} "
                             "QPushButton:pressed { "
                             "background-color: #388E3C; "
                             "}");
    connect(saveButton, &QPushButton::clicked, this, &StudentInviteDialog::onSaveButtonClicked);
    buttonLayout->addWidget(saveButton);
    
    cancelButton = new QPushButton("取消", this);
    cancelButton->setMinimumHeight(35);
    cancelButton->setStyleSheet("QPushButton { "
                               "background-color: rgba(158, 158, 158, 200); "
                               "color: white; "
                               "border: 2px solid rgba(158, 158, 158, 255); "
                               "border-radius: 5px; "
                               "font-weight: bold; "
                               "padding: 5px 20px; "
                               "} "
                               "QPushButton:hover { "
                               "background-color: #9E9E9E; "
                               "} "
                               "QPushButton:pressed { "
                               "background-color: #757575; "
                               "}");
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

QWidget* StudentInviteDialog::createTabPage(const QString &windowTitle)
{
    QWidget *page = new QWidget();
    QVBoxLayout *mainPageLayout = new QVBoxLayout(page);
    mainPageLayout->setSpacing(10);
    mainPageLayout->setContentsMargins(10, 10, 10, 10);
    
    // 上半部分：邀请学生设置（水平布局）
    QWidget *studentWidget = new QWidget(page);
    QHBoxLayout *pageLayout = new QHBoxLayout(studentWidget);
    pageLayout->setSpacing(10);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    
    TabWidgets widgets;
    
    // 左侧：可选学生列表
    QGroupBox *availableGroup = new QGroupBox("可选学生", page);
    availableGroup->setStyleSheet("QGroupBox { "
                                 "border: 1px solid #ccc; "
                                 "border-radius: 5px; "
                                 "margin-top: 10px; "
                                 "font-weight: bold; "
                                 "} "
                                 "QGroupBox::title { "
                                 "subcontrol-origin: margin; "
                                 "subcontrol-position: top center; "
                                 "padding: 0 5px; "
                                 "}");
    QVBoxLayout *availableLayout = new QVBoxLayout(availableGroup);
    
    widgets.availableList = new QListWidget(availableGroup);
    widgets.availableList->setStyleSheet("QListWidget { "
                                        "border: 1px solid #ddd; "
                                        "border-radius: 3px; "
                                        "} "
                                        "QListWidget::item { "
                                        "padding: 5px; "
                                        "} "
                                        "QListWidget::item:selected { "
                                        "background-color: #FF9800; "
                                        "color: white; "
                                        "}");
    // 添加所有可用学生
    for (const QString &student : availableStudents) {
        widgets.availableList->addItem(student);
    }
    connect(widgets.availableList, &QListWidget::itemDoubleClicked, 
            this, &StudentInviteDialog::onAvailableListDoubleClicked);
    availableLayout->addWidget(widgets.availableList);
    
    pageLayout->addWidget(availableGroup);
    
    // 中间：操作按钮
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(5);
    buttonLayout->addStretch();
    
    widgets.addButton = new QPushButton("添加 >>", page);
    widgets.addButton->setMinimumHeight(30);
    widgets.addButton->setStyleSheet("QPushButton { "
                                    "background-color: rgba(255, 152, 0, 150); "
                                    "color: white; "
                                    "border: 1px solid #FF9800; "
                                    "border-radius: 3px; "
                                    "padding: 5px 10px; "
                                    "} "
                                    "QPushButton:hover { "
                                    "background-color: #FF9800; "
                                    "}");
    connect(widgets.addButton, &QPushButton::clicked, this, &StudentInviteDialog::onAddButtonClicked);
    buttonLayout->addWidget(widgets.addButton);
    
    widgets.removeButton = new QPushButton("<< 移除", page);
    widgets.removeButton->setMinimumHeight(30);
    widgets.removeButton->setStyleSheet("QPushButton { "
                                       "background-color: rgba(244, 67, 54, 150); "
                                       "color: white; "
                                       "border: 1px solid #F44336; "
                                       "border-radius: 3px; "
                                       "padding: 5px 10px; "
                                       "} "
                                       "QPushButton:hover { "
                                       "background-color: #F44336; "
                                       "}");
    connect(widgets.removeButton, &QPushButton::clicked, this, &StudentInviteDialog::onRemoveButtonClicked);
    buttonLayout->addWidget(widgets.removeButton);
    
    buttonLayout->addSpacing(20);
    
    widgets.moveUpButton = new QPushButton("上移 ↑", page);
    widgets.moveUpButton->setMinimumHeight(30);
    widgets.moveUpButton->setStyleSheet("QPushButton { "
                                       "background-color: rgba(33, 150, 243, 150); "
                                       "color: white; "
                                       "border: 1px solid #2196F3; "
                                       "border-radius: 3px; "
                                       "padding: 5px 10px; "
                                       "} "
                                       "QPushButton:hover { "
                                       "background-color: #2196F3; "
                                       "}");
    connect(widgets.moveUpButton, &QPushButton::clicked, this, &StudentInviteDialog::onMoveUpButtonClicked);
    buttonLayout->addWidget(widgets.moveUpButton);
    
    widgets.moveDownButton = new QPushButton("下移 ↓", page);
    widgets.moveDownButton->setMinimumHeight(30);
    widgets.moveDownButton->setStyleSheet("QPushButton { "
                                         "background-color: rgba(33, 150, 243, 150); "
                                         "color: white; "
                                         "border: 1px solid #2196F3; "
                                         "border-radius: 3px; "
                                         "padding: 5px 10px; "
                                         "} "
                                         "QPushButton:hover { "
                                         "background-color: #2196F3; "
                                         "}");
    connect(widgets.moveDownButton, &QPushButton::clicked, this, &StudentInviteDialog::onMoveDownButtonClicked);
    buttonLayout->addWidget(widgets.moveDownButton);
    
    buttonLayout->addStretch();
    pageLayout->addLayout(buttonLayout);
    
    // 右侧：已选学生列表
    QGroupBox *selectedGroup = new QGroupBox("已选学生（按优先级）", page);
    selectedGroup->setStyleSheet("QGroupBox { "
                                "border: 1px solid #ccc; "
                                "border-radius: 5px; "
                                "margin-top: 10px; "
                                "font-weight: bold; "
                                "} "
                                "QGroupBox::title { "
                                "subcontrol-origin: margin; "
                                "subcontrol-position: top center; "
                                "padding: 0 5px; "
                                "}");
    QVBoxLayout *selectedLayout = new QVBoxLayout(selectedGroup);
    
    widgets.selectedList = new QListWidget(selectedGroup);
    widgets.selectedList->setStyleSheet("QListWidget { "
                                       "border: 1px solid #ddd; "
                                       "border-radius: 3px; "
                                       "} "
                                       "QListWidget::item { "
                                       "padding: 5px; "
                                       "} "
                                       "QListWidget::item:selected { "
                                       "background-color: #4CAF50; "
                                       "color: white; "
                                       "}");
    connect(widgets.selectedList, &QListWidget::itemDoubleClicked, 
            this, &StudentInviteDialog::onSelectedListDoubleClicked);
    selectedLayout->addWidget(widgets.selectedList);
    
    pageLayout->addWidget(selectedGroup);
    
    mainPageLayout->addWidget(studentWidget);
    
    // 下半部分：扫荡任务设置
    QGroupBox *taskGroup = new QGroupBox("扫荡任务设置", page);
    taskGroup->setStyleSheet("QGroupBox { "
                            "border: 2px solid #2196F3; "
                            "border-radius: 5px; "
                            "margin-top: 10px; "
                            "font-weight: bold; "
                            "color: #2196F3; "
                            "} "
                            "QGroupBox::title { "
                            "subcontrol-origin: margin; "
                            "subcontrol-position: top center; "
                            "padding: 0 5px; "
                            "}");
    QHBoxLayout *taskLayout = new QHBoxLayout(taskGroup);
    taskLayout->setSpacing(15);
    
    // 任务索引
    QLabel *taskIndexLabel = new QLabel("任务索引:", taskGroup);
    taskIndexLabel->setStyleSheet("font-weight: normal; color: #333;");
    taskLayout->addWidget(taskIndexLabel);
    
    widgets.taskIndexSpinBox = new QSpinBox(taskGroup);
    widgets.taskIndexSpinBox->setRange(1, 25);
    widgets.taskIndexSpinBox->setValue(1);
    widgets.taskIndexSpinBox->setMinimumWidth(60);
    widgets.taskIndexSpinBox->setStyleSheet("QSpinBox { "
                                           "border: 2px solid #2196F3; "
                                           "border-radius: 3px; "
                                           "padding: 3px; "
                                           "background-color: white; "
                                           "}");
    taskLayout->addWidget(widgets.taskIndexSpinBox);
    
    taskLayout->addSpacing(20);
    
    // 子任务索引
    QLabel *subTaskIndexLabel = new QLabel("子任务索引:", taskGroup);
    subTaskIndexLabel->setStyleSheet("font-weight: normal; color: #333;");
    taskLayout->addWidget(subTaskIndexLabel);
    
    widgets.subTaskIndexSpinBox = new QSpinBox(taskGroup);
    widgets.subTaskIndexSpinBox->setRange(0, 5);
    widgets.subTaskIndexSpinBox->setValue(2);
    widgets.subTaskIndexSpinBox->setMinimumWidth(60);
    widgets.subTaskIndexSpinBox->setStyleSheet("QSpinBox { "
                                              "border: 2px solid #2196F3; "
                                              "border-radius: 3px; "
                                              "padding: 3px; "
                                              "background-color: white; "
                                              "}");
    taskLayout->addWidget(widgets.subTaskIndexSpinBox);
    
    taskLayout->addStretch();
    
    // 提示信息
    QLabel *hintLabel = new QLabel("(任务索引=任务序号, 子任务索引=关卡序号-1)", taskGroup);
    hintLabel->setStyleSheet("font-weight: normal; color: #888; font-size: 9pt;");
    taskLayout->addWidget(hintLabel);
    
    mainPageLayout->addWidget(taskGroup);
    
    // 保存控件引用
    tabWidgetsMap[windowTitle] = widgets;
    
    return page;
}

QListWidget* StudentInviteDialog::getCurrentAvailableList()
{
    int index = tabWidget->currentIndex();
    if (index >= 0 && index < windowTitles.size()) {
        return tabWidgetsMap[windowTitles[index]].availableList;
    }
    return nullptr;
}

QListWidget* StudentInviteDialog::getCurrentSelectedList()
{
    int index = tabWidget->currentIndex();
    if (index >= 0 && index < windowTitles.size()) {
        return tabWidgetsMap[windowTitles[index]].selectedList;
    }
    return nullptr;
}

void StudentInviteDialog::onAddButtonClicked()
{
    QListWidget *availableList = getCurrentAvailableList();
    QListWidget *selectedList = getCurrentSelectedList();
    
    if (!availableList || !selectedList) return;
    
    QListWidgetItem *item = availableList->currentItem();
    if (item) {
        QString studentName = item->text();
        // 检查是否已经添加
        bool exists = false;
        for (int i = 0; i < selectedList->count(); i++) {
            if (selectedList->item(i)->text() == studentName) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            selectedList->addItem(studentName);
        }
    }
}

void StudentInviteDialog::onRemoveButtonClicked()
{
    QListWidget *selectedList = getCurrentSelectedList();
    if (!selectedList) return;
    
    QListWidgetItem *item = selectedList->currentItem();
    if (item) {
        delete selectedList->takeItem(selectedList->row(item));
    }
}

void StudentInviteDialog::onMoveUpButtonClicked()
{
    QListWidget *selectedList = getCurrentSelectedList();
    if (!selectedList) return;
    
    int currentRow = selectedList->currentRow();
    if (currentRow > 0) {
        QListWidgetItem *item = selectedList->takeItem(currentRow);
        selectedList->insertItem(currentRow - 1, item);
        selectedList->setCurrentRow(currentRow - 1);
    }
}

void StudentInviteDialog::onMoveDownButtonClicked()
{
    QListWidget *selectedList = getCurrentSelectedList();
    if (!selectedList) return;
    
    int currentRow = selectedList->currentRow();
    if (currentRow >= 0 && currentRow < selectedList->count() - 1) {
        QListWidgetItem *item = selectedList->takeItem(currentRow);
        selectedList->insertItem(currentRow + 1, item);
        selectedList->setCurrentRow(currentRow + 1);
    }
}

void StudentInviteDialog::onSaveButtonClicked()
{
    this->accept();
}

void StudentInviteDialog::onAvailableListDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    onAddButtonClicked();
}

void StudentInviteDialog::onSelectedListDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    onRemoveButtonClicked();
}

QStringList StudentInviteDialog::getStudentList(const QString &windowTitle) const
{
    QStringList students;
    if (tabWidgetsMap.contains(windowTitle)) {
        QListWidget *selectedList = tabWidgetsMap[windowTitle].selectedList;
        for (int i = 0; i < selectedList->count(); i++) {
            students.append(selectedList->item(i)->text());
        }
    }
    return students;
}

void StudentInviteDialog::setStudentList(const QString &windowTitle, const QStringList &students)
{
    if (tabWidgetsMap.contains(windowTitle)) {
        QListWidget *selectedList = tabWidgetsMap[windowTitle].selectedList;
        selectedList->clear();
        for (const QString &student : students) {
            selectedList->addItem(student);
        }
    }
}

QHash<QString, QStringList> StudentInviteDialog::getAllStudentLists() const
{
    QHash<QString, QStringList> lists;
    for (const QString &title : windowTitles) {
        lists[title] = getStudentList(title);
    }
    return lists;
}

void StudentInviteDialog::setAllStudentLists(const QHash<QString, QStringList> &lists)
{
    for (auto it = lists.constBegin(); it != lists.constEnd(); ++it) {
        setStudentList(it.key(), it.value());
    }
}

QPair<int, int> StudentInviteDialog::getTaskParams(const QString &windowTitle) const
{
    if (tabWidgetsMap.contains(windowTitle)) {
        int taskIndex = tabWidgetsMap[windowTitle].taskIndexSpinBox->value();
        int subTaskIndex = tabWidgetsMap[windowTitle].subTaskIndexSpinBox->value();
        return qMakePair(taskIndex, subTaskIndex);
    }
    return qMakePair(1, 2);  // 默认值
}

void StudentInviteDialog::setTaskParams(const QString &windowTitle, int taskIndex, int subTaskIndex)
{
    if (tabWidgetsMap.contains(windowTitle)) {
        tabWidgetsMap[windowTitle].taskIndexSpinBox->setValue(taskIndex);
        tabWidgetsMap[windowTitle].subTaskIndexSpinBox->setValue(subTaskIndex);
    }
}

QHash<QString, QPair<int, int>> StudentInviteDialog::getAllTaskParams() const
{
    QHash<QString, QPair<int, int>> params;
    for (const QString &title : windowTitles) {
        params[title] = getTaskParams(title);
    }
    return params;
}

void StudentInviteDialog::setAllTaskParams(const QHash<QString, QPair<int, int>> &params)
{
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        setTaskParams(it.key(), it.value().first, it.value().second);
    }
}

