#include "arona.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    arona w;
    w.show();
    return a.exec();
}
