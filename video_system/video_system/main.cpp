#include "frmmain.h"
#include "frmlogin.h"

#include "quihelper.h"
#include "dbquery.h"
#include "appinit.h"
#include <QDebug>


int main(int argc, char *argv[])
{
    // 初始化静态库中的 Qt 资源
    // core_qui 库中的资源文件需要显式初始化
    Q_INIT_RESOURCE(qss);
    Q_INIT_RESOURCE(qm);
    Q_INIT_RESOURCE(font);
    Q_INIT_RESOURCE(control);
    Q_INIT_RESOURCE(onvif);
    

    //下面这行表示不打印Qt内部类的警告提示信息
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    //QLoggingCategory::setFilterRules("*.critical=false\n*.warning=false");
#endif
    QUIHelper::initMain(false);
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/main.ico"));

    //强制指定启动窗体方便测试
    AppConfig::IndexStart = 0;
    AppInit::Instance()->start();

    QWidget *w;
    if (AppConfig::AutoLogin) {
        w = new frmMain;
    } else {
        w = new frmLogin;
    }
    


    w->show();
    w->activateWindow();
    return a.exec();
}
