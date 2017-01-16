#ifndef LOGIN_FORM_H
#define LOGIN_FORM_H

#include <QWidget>

namespace Ui {
class LoginForm;
}

class LoginForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = 0);
    ~LoginForm();
    void hide();
private:
    Ui::LoginForm *ui;

signals:
    void gettedLoginFromForm(QString);
public slots:
    void getLoginFromForm();
};

#endif // LOGIN_FORM_H
