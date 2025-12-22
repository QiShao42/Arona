#ifndef STUDENTINVITEDIALOG_H
#define STUDENTINVITEDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QStringList>
#include <QHash>
#include <QPair>

class StudentInviteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StudentInviteDialog(QWidget *parent = nullptr);
    ~StudentInviteDialog();
    
    // 获取指定窗口的邀请学生列表
    QStringList getStudentList(const QString &windowTitle) const;
    
    // 设置指定窗口的邀请学生列表
    void setStudentList(const QString &windowTitle, const QStringList &students);
    
    // 获取所有窗口的学生列表
    QHash<QString, QStringList> getAllStudentLists() const;
    
    // 设置所有窗口的学生列表
    void setAllStudentLists(const QHash<QString, QStringList> &lists);
    
    // 获取指定窗口的扫荡任务参数 (taskIndex, subTaskIndex)
    QPair<int, int> getTaskParams(const QString &windowTitle) const;
    
    // 设置指定窗口的扫荡任务参数
    void setTaskParams(const QString &windowTitle, int taskIndex, int subTaskIndex);
    
    // 获取所有窗口的扫荡任务参数
    QHash<QString, QPair<int, int>> getAllTaskParams() const;
    
    // 设置所有窗口的扫荡任务参数
    void setAllTaskParams(const QHash<QString, QPair<int, int>> &params);
    
    // 获取指定窗口的强制邀请设置
    bool isForceInviteEnabled(const QString &windowTitle) const;
    
    // 设置指定窗口的强制邀请状态
    void setForceInviteEnabled(const QString &windowTitle, bool enabled);
    
    // 获取所有窗口的强制邀请设置
    QHash<QString, bool> getAllForceInviteSettings() const;
    
    // 设置所有窗口的强制邀请状态
    void setAllForceInviteSettings(const QHash<QString, bool> &settings);
    
    // 获取指定窗口的扫荡任务启用状态
    bool isSweepTaskEnabled(const QString &windowTitle) const;
    
    // 设置指定窗口的扫荡任务启用状态
    void setSweepTaskEnabled(const QString &windowTitle, bool enabled);
    
    // 获取所有窗口的扫荡任务启用设置
    QHash<QString, bool> getAllSweepTaskSettings() const;
    
    // 设置所有窗口的扫荡任务启用状态
    void setAllSweepTaskSettings(const QHash<QString, bool> &settings);

private slots:
    void onAddButtonClicked();
    void onRemoveButtonClicked();
    void onMoveUpButtonClicked();
    void onMoveDownButtonClicked();
    void onSaveButtonClicked();
    void onAvailableListDoubleClicked(QListWidgetItem *item);
    void onSelectedListDoubleClicked(QListWidgetItem *item);

private:
    void setupUi();
    void loadAvailableStudents();
    QListWidget* getCurrentAvailableList();
    QListWidget* getCurrentSelectedList();
    
    // UI控件
    QTabWidget *tabWidget;
    
    // 每个标签页的控件
    struct TabWidgets {
        QListWidget *availableList;  // 可选学生列表
        QListWidget *selectedList;   // 已选学生列表
        QPushButton *addButton;
        QPushButton *removeButton;
        QPushButton *moveUpButton;
        QPushButton *moveDownButton;
        // 扫荡任务参数
        QSpinBox *taskIndexSpinBox;     // 任务索引
        QSpinBox *subTaskIndexSpinBox;  // 子任务索引
        QCheckBox *sweepTaskCheckBox;   // 扫荡任务启用
        QCheckBox *forceInviteCheckBox; // 强制邀请启用
    };
    
    QHash<QString, TabWidgets> tabWidgetsMap;
    
    QPushButton *saveButton;
    QPushButton *cancelButton;
    
    // 窗口标题列表
    QStringList windowTitles;
    
    // 可用学生名称列表
    QStringList availableStudents;
    
    // 创建单个标签页
    QWidget* createTabPage(const QString &windowTitle);
};

#endif // STUDENTINVITEDIALOG_H

