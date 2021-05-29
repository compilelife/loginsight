#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <QWidget>

class MainWindow;
class QLabel;
class WelcomePage : public QWidget
{
    Q_OBJECT
public:
    explicit WelcomePage(MainWindow *parent = nullptr);

signals:

private:
    QWidget* buildRecentArea(const QString& name, const QString &key, const char *slot);
    QWidget* buildVersionArea();
    QWidget* buildActionsArea(const QString& name, const QVector<int>& actions);
    QLabel* createAreaLabel();
    void rebuildVersionArea();

protected:
    void showEvent(QShowEvent*) override;

private:
    MainWindow* mMainWnd;
    QString mTitleStyle;
    QWidget* mVersionWidget{nullptr};
};

#endif // WELCOMEPAGE_H
