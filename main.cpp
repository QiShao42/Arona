#include "arona.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序图标（任务栏和窗口图标）
    a.setWindowIcon(QIcon(":/images/icon/app_icon.png"));
    
    arona w;
    w.show();
    return a.exec();
}
