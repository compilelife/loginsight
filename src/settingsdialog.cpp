#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QPushButton>
#include <QMessageBox>
#include <QFontDialog>
#include "util.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    setWindowTitle("设置");

    QSettings config;
    mEditorFont.setFamily(config.value("editorFont", "monospace").toString());
    mEditorFont.setPointSize(config.value("editorFontSize", 12).toInt());
    updateFontLabel();

    ui->caseSensitiveCheckBox->setChecked(config.value("caseSensitive", true).toBool());
    ui->checkUpdateCheckBox->setChecked(config.value("checkUpdate", true).toBool());
    ui->wrapCheckBox->setChecked(config.value("wrap", false).toBool());

    connect(ui->cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->okBtn, &QPushButton::clicked, [this]{
       QSettings saveConfig;
       saveConfig.setValue("editorFont", mEditorFont.family());
       saveConfig.setValue("editorFontSize", mEditorFont.pointSize());
       saveConfig.setValue("caseSensitive", ui->caseSensitiveCheckBox->isChecked());
       saveConfig.setValue("checkUpdate", ui->checkUpdateCheckBox->isChecked());
       saveConfig.setValue("wrap", ui->wrapCheckBox->isChecked());
       saveConfig.sync();

       QMessageBox hint;
       hint.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
       hint.button(QMessageBox::Ok)->setText("立即重启");
       hint.button(QMessageBox::Cancel)->setText("稍后重启");
       hint.setText("设置保存成功，重启后生效！");
       hint.setWindowTitle("保存成功");
       switch(hint.exec()) {
       case QMessageBox::Ok:
           relaunchApp();
           return;
       default:
           this->accept();
            break;
       }
    });

    connect(ui->chooseFontBtn, &QPushButton::clicked, [this]{
        bool ok = false;
        auto font = QFontDialog::getFont(&ok, mEditorFont, this, "选择日志区的字体");
        if (ok) {
            mEditorFont = font;
            updateFontLabel();
        }
    });
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::updateFontLabel()
{
    ui->fontLabel->setText(QString("%1 %2").arg(mEditorFont.family()).arg(mEditorFont.pointSize()));
}
