#include "timerdialog.h"
#include <QTime>

TimerDialog::TimerDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

TimerDialog::~TimerDialog()
{
}

void TimerDialog::setupUi()
{
    this->setWindowTitle("定时执行设置");
    this->setFixedSize(300, 450);
    this->setModal(true);
    
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 启用定时复选框
    enableTimerCheckBox = new QCheckBox("启用定时执行", this);
    enableTimerCheckBox->setStyleSheet("QCheckBox { "
                                      "font-weight: bold; "
                                      "font-size: 11pt; "
                                      "color: #FF9800; "
                                      "}");
    mainLayout->addWidget(enableTimerCheckBox);
    
    // 时间列表分组框
    timeListGroupBox = new QGroupBox("定时时间（最多8个）", this);
    timeListGroupBox->setStyleSheet("QGroupBox { "
                                   "border: 2px solid #FF9800; "
                                   "border-radius: 5px; "
                                   "margin-top: 10px; "
                                   "padding-top: 10px; "
                                   "font-weight: bold; "
                                   "color: #FF9800; "
                                   "} "
                                   "QGroupBox::title { "
                                   "subcontrol-origin: margin; "
                                   "subcontrol-position: top center; "
                                   "padding: 0 5px; "
                                   "}");
    
    QVBoxLayout *timeListLayout = new QVBoxLayout(timeListGroupBox);
    timeListLayout->setSpacing(5);
    timeListLayout->setContentsMargins(10, 15, 10, 10);
    
    // 创建8个时间选择器
    for (int i = 0; i < 8; i++) {
        QHBoxLayout *timeRowLayout = new QHBoxLayout();
        timeRowLayout->setSpacing(8);
        
        timeCheckBoxes[i] = new QCheckBox(timeListGroupBox);
        timeCheckBoxes[i]->setStyleSheet("QCheckBox { font-weight: normal; }");
        timeRowLayout->addWidget(timeCheckBoxes[i]);
        
        QLabel *numberLabel = new QLabel(QString("%1:").arg(i + 1), timeListGroupBox);
        numberLabel->setStyleSheet("font-weight: normal; color: #666;");
        numberLabel->setFixedWidth(20);
        timeRowLayout->addWidget(numberLabel);
        
        timeEdits[i] = new QTimeEdit(timeListGroupBox);
        timeEdits[i]->setDisplayFormat("HH:mm");
        timeEdits[i]->setTime(QTime(0, 0));
        timeEdits[i]->setStyleSheet("QTimeEdit { "
                                   "border: 2px solid #FF9800; "
                                   "border-radius: 3px; "
                                   "padding: 3px; "
                                   "background-color: white; "
                                   "font-weight: normal; "
                                   "}");
        timeRowLayout->addWidget(timeEdits[i]);
        
        timeListLayout->addLayout(timeRowLayout);
    }
    
    mainLayout->addWidget(timeListGroupBox);
    
    // 清空按钮
    clearAllButton = new QPushButton("清空所有", this);
    clearAllButton->setMinimumHeight(30);
    clearAllButton->setStyleSheet("QPushButton { "
                                 "background-color: rgba(255, 152, 0, 150); "
                                 "color: white; "
                                 "border: 2px solid rgba(255, 152, 0, 255); "
                                 "border-radius: 5px; "
                                 "font-weight: bold; "
                                 "padding: 5px; "
                                 "} "
                                 "QPushButton:hover { "
                                 "background-color: rgba(255, 152, 0, 200); "
                                 "} "
                                 "QPushButton:pressed { "
                                 "background-color: rgba(245, 124, 0, 255); "
                                 "}");
    connect(clearAllButton, &QPushButton::clicked, this, &TimerDialog::onClearAllButtonClicked);
    mainLayout->addWidget(clearAllButton);
    
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
                             "padding: 5px; "
                             "} "
                             "QPushButton:hover { "
                             "background-color: #4CAF50; "
                             "} "
                             "QPushButton:pressed { "
                             "background-color: #388E3C; "
                             "}");
    connect(saveButton, &QPushButton::clicked, this, &TimerDialog::onSaveButtonClicked);
    buttonLayout->addWidget(saveButton);
    
    cancelButton = new QPushButton("取消", this);
    cancelButton->setMinimumHeight(35);
    cancelButton->setStyleSheet("QPushButton { "
                               "background-color: rgba(158, 158, 158, 200); "
                               "color: white; "
                               "border: 2px solid rgba(158, 158, 158, 255); "
                               "border-radius: 5px; "
                               "font-weight: bold; "
                               "padding: 5px; "
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

void TimerDialog::onClearAllButtonClicked()
{
    // 清空所有定时设置
    for (int i = 0; i < 8; i++) {
        timeCheckBoxes[i]->setChecked(false);
        timeEdits[i]->setTime(QTime(0, 0));
    }
}

void TimerDialog::onSaveButtonClicked()
{
    // 保存并关闭
    this->accept();
}

bool TimerDialog::isTimerEnabled() const
{
    return enableTimerCheckBox->isChecked();
}

QVector<QTime> TimerDialog::getScheduledTimes() const
{
    QVector<QTime> times;
    
    for (int i = 0; i < 8; i++) {
        if (timeCheckBoxes[i]->isChecked()) {
            times.append(timeEdits[i]->time());
        }
    }
    
    return times;
}

void TimerDialog::setTimerEnabled(bool enabled)
{
    enableTimerCheckBox->setChecked(enabled);
}

void TimerDialog::setScheduledTimes(const QVector<QTime> &times)
{
    // 先清空所有设置
    for (int i = 0; i < 8; i++) {
        timeCheckBoxes[i]->setChecked(false);
        timeEdits[i]->setTime(QTime(0, 0));
    }
    
    // 设置新的时间
    int index = 0;
    for (const QTime &time : times) {
        if (index >= 8) break;
        
        timeCheckBoxes[index]->setChecked(true);
        timeEdits[index]->setTime(time);
        index++;
    }
}

