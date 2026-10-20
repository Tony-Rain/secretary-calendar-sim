#include <QApplication>
#include "ParamsDialog.h"
#include "MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    ParamsDialog dlg;
    if (dlg.exec() != QDialog::Accepted) return 0;
    MainWindow w(dlg.params());
    w.show();
    return app.exec();
}
