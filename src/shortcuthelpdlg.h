#ifndef SHORTCUTHELPDLG_H
#define SHORTCUTHELPDLG_H

#include <QDialog>

namespace Ui {
class ShortcutHelpDlg;
}

class ShortcutHelpDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutHelpDlg(QWidget *parent = nullptr);
    ~ShortcutHelpDlg();

private:
    Ui::ShortcutHelpDlg *ui;
};

#endif // SHORTCUTHELPDLG_H
