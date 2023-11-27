#include "QtTimeAxis.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtTimeAxis w;
	//w.resize(1920, w.height());
	w.resize(w.width(), 96);
    w.show();
    return a.exec();
}
