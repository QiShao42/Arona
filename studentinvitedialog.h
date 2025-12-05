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

