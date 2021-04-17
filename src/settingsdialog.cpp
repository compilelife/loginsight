#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QPushButton>
#include "toast.h"
#include <QFontDialog>

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

    ui->gotoEOFCheckBox->setChecked(config.value("gotoEOF", true).toBool());
    ui->caseSensitiveCheckBox->setChecked(config.value("caseSensitive", true).toBool());
    ui->checkUpdateCheckBox->setChecked(config.value("checkUpdate", true).toBool());
    ui->wrapCheckBox->setChecked(config.value("wrap", false).toBool());

    connect(ui->cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->okBtn, &QPushButton::clicked, [this]{
       QSettings saveConfig;
       saveConfig.setValue("editorFont", mEditorFont.family());
       saveConfig.setValue("editorFontSize", mEditorFont.pointSize());
       saveConfig.setValue("gotoEOF", ui->gotoEOFCheckBox->isChecked());
       saveConfig.setValue("caseSensitive", ui->caseSensitiveCheckBox->isChecked());
       saveConfig.setValue("checkUpdate", ui->checkUpdateCheckBox->isChecked());
       saveConfig.setValue("wrap", ui->wrapCheckBox->isChecked());
       saveConfig.sync();
       Toast::instance().show(Toast::INFO, "设置保存成功，重启后生效！");
       this->accept();
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
