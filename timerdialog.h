#ifndef TIMERDIALOG_H
#define TIMERDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QTimeEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QTime>
#include <QVector>

class TimerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimerDialog(QWidget *parent = nullptr);
    ~TimerDialog();
    
    // 获取定时设置
    bool isTimerEnabled() const;
    QVector<QTime> getScheduledTimes() const;
    
    // 设置定时配置
    void setTimerEnabled(bool enabled);
    void setScheduledTimes(const QVector<QTime> &times);

private slots:
    void onClearAllButtonClicked();
    void onSaveButtonClicked();

private:
    void setupUi();
    
    // UI控件
    QCheckBox *enableTimerCheckBox;
    QTimeEdit *timeEdits[8];
    QCheckBox *timeCheckBoxes[8];
    QPushButton *clearAllButton;
    QPushButton *saveButton;
    QPushButton *cancelButton;
    
    QVBoxLayout *mainLayout;
    QGroupBox *timeListGroupBox;
};

#endif // TIMERDIALOG_H

