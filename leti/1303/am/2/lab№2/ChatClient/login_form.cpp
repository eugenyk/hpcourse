#include "login_form.h"
#include "ui_login_form.h"

LoginForm::LoginForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);
    //connect(ui->sendLoginButton,SIGNAL(pressed()),this,SIGNAL(pressedSendLoginButton()));
    connect(ui->sendLoginButton,SIGNAL(pressed()),this,SLOT(getLoginFromForm()));
}

void LoginForm::getLoginFromForm(){
   QString login=  ui->loginLineEdit->text();
   emit gettedLoginFromForm(login);

}

LoginForm::~LoginForm()
{
    delete ui;
}
