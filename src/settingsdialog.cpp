#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QPushButton>
#include <QMessageBox>
#include <QFontDialog>
#include "util.h"

void regedit_new(QString appPath,QString className,QString ext,QString extDes)
{
    QString baseUrl("HKEY_CURRENT_USER\\Software\\Classes");    // 要添加的顶层目录
    QSettings settingClasses(baseUrl, QSettings::NativeFormat);
    settingClasses.setValue("/" + className + "/Shell/Open/Command/.", "\"" + appPath + "\" \"%1\"");
    settingClasses.setValue("/" + className + "/.", extDes);
    settingClasses.setValue("/" + className + "/DefaultIcon/.", appPath + ",0");
    // 关联ext 和 类别
    settingClasses.setValue("/" + ext + "/OpenWithProgIds/" + className, "");
    // 立即保存该修改
    settingClasses.sync();
}

void addFileAssociation(QStringList exts)
{
    QString app_path = qApp->applicationFilePath();
    /* 一定要将此路径转换为"\\"形式，因为qt默认识别的是"/"，windows并不识别 */
    app_path = app_path.split("/").join("\\");

    QString className("loginsight file");                           // 自定义的类别
    QString extDes("loginisight file");

    for (auto&& ext : exts)
        regedit_new(app_path,className,ext,extDes);
}

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
    ui->dynamicLimitSpinBox->setValue(config.value("defaultDynamicLimit").toInt());
    ui->closeTabPromptCheckBox->setChecked(config.value("closeTabPrompt").toBool());

    connect(ui->cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->okBtn, &QPushButton::clicked, [this]{
       QSettings saveConfig;
       saveConfig.setValue("editorFont", mEditorFont.family());
       saveConfig.setValue("editorFontSize", mEditorFont.pointSize());
       saveConfig.setValue("caseSensitive", ui->caseSensitiveCheckBox->isChecked());
       saveConfig.setValue("checkUpdate", ui->checkUpdateCheckBox->isChecked());
       saveConfig.setValue("wrap", ui->wrapCheckBox->isChecked());
       saveConfig.setValue("defaultDynamicLimit", ui->dynamicLimitSpinBox->value());
       saveConfig.setValue("closeTabPrompt", ui->closeTabPromptCheckBox->isChecked());
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

#if defined (Q_OS_WIN)
    //windows特有的，在注册表添加文件关联
    connect(ui->addFileAssociationBtn, &QPushButton::clicked, []{
        addFileAssociation({".log", ".txt", ".liprj"});
        QMessageBox::information(nullptr, "设置", "已经在注册表写入文件关联信息");
    });
#else
    ui->addFileAssociationBtn->setVisible(false);
#endif
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::updateFontLabel()
{
    ui->fontLabel->setText(QString("%1 %2").arg(mEditorFont.family()).arg(mEditorFont.pointSize()));
}
