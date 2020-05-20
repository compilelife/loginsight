#include "shortcuthelpdlg.h"
#include "ui_shortcuthelpdlg.h"

ShortcutHelpDlg::ShortcutHelpDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutHelpDlg)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Tool);
    setWindowTitle("快捷键");
}

ShortcutHelpDlg::~ShortcutHelpDlg()
{
    delete ui;
}
