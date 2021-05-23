#include "filterdialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>

FilterDialog::FilterDialog()
{
    auto box = new QVBoxLayout;

    auto patternEdit = new QLineEdit;
    box->addWidget(patternEdit);

    auto caseSensitiveCheckBox = new QCheckBox("大小写敏感");
    box->addWidget(caseSensitiveCheckBox);

    auto useRegexCheckBox = new QCheckBox("这是正则表达式");
    box->addWidget(useRegexCheckBox);

    auto buttons = new QHBoxLayout;
    auto cancelBtn = new QPushButton("取消");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(cancelBtn);

    auto okBtn = new QPushButton("确定");
    connect(patternEdit, &QLineEdit::textChanged, [okBtn](const QString& text){
        okBtn->setEnabled(!text.isEmpty());
    });
    okBtn->setDefault(true);
    connect(okBtn, &QPushButton::clicked, [this, caseSensitiveCheckBox, useRegexCheckBox, patternEdit]{
        mArg.caseSensitive = caseSensitiveCheckBox->isChecked();
        mArg.regex = useRegexCheckBox->isChecked();
        mArg.pattern = patternEdit->text();
        accept();
    });
    buttons->addWidget(okBtn);

    box->addLayout(buttons);
    setLayout(box);
}
