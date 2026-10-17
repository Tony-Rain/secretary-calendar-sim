#include <QApplication>
#include "ParamsDialog.h"
#include "MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    ParamsDialog dlg;
    MainWindow* w = nullptr;

    QObject::connect(&dlg, &QDialog::accepted, [&]() {
        w = new MainWindow(dlg.getParams());
        w->show();
    });

    dlg.open();
    return app.exec();
}
