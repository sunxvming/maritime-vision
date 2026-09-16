#include "commonstyle.h"
#include "quihelper.h"

void CommonStyle::addQtControlStyle(QStringList &list)
{
    //默认Qt对话框样式
    list << QString("QMessageBox{background:%1;}").arg("#FFFFFF");
    list << QString("QMessageBox>QLabel{color:%1;}").arg("#000000");
    list << QString("QDialogButtonBox>QPushButton{min-width:%1px;}").arg(50);

    //悬浮窗体背景色
    list << QString("QDockWidget{background:%1;}").arg(QUIConfig::BorderColor);
    //分隔条背景颜色
    list << QString("QSplitter{qproperty-handleWidth:1px;}QSplitter::handle{background:%1;}").arg(QUIConfig::BorderColor);
    //停靠窗体标题栏样式
    list << QString("CustomTitleBar{background:%1;border-bottom:1px solid %2;}").arg(QUIConfig::NormalColorStart).arg(QUIConfig::BorderColor);
    //停靠窗体分隔条样式,他娘的根本不是QSplitter,做梦也想不到这样设置,搞了很久
    list << QString("QMainWindow::separator{width:%1px;height:%1px;margin:%2px;padding:%2px;background:%3;}").arg(2).arg(0).arg(QUIConfig::BorderColor);

    //选项卡居中
    list << QString("QTabBar,QTabWidget::tab-bar{alignment:center;}");
    //选项卡 不显示切换按钮 自动拉伸-居然自带了卧槽之前还自己来计算
    list << QString("QTabBar{qproperty-usesScrollButtons:false;qproperty-documentMode:true;qproperty-expanding:true;}");

    //提示信息背景透明度
    list << QString("QToolTip{opacity:230;}");
    //树状控件
    list << QString("QTreeView{padding:5px 0px 5px 5px;}");
    //日历控件
    list << QString("QCalendarWidget{qproperty-gridVisible:true;}");

    //表格奇数偶数行不同颜色
    list << QString("QTableView,QTreeView{qproperty-alternatingRowColors:false;}");
    //单元格光标在最前面
    list << QString("QGroupBox>QLineEdit,QFrame>QLineEdit{qproperty-cursorPosition:0;}");
}

void CommonStyle::addCustomControlStyle(QStringList &list, int borderWidth)
{
    //硬盘使用空间控件
    list << QString("DeviceSizeTable{qproperty-bgColor:%1;}").arg(QUIConfig::PanelColor);
    list << QString("DeviceSizeTable{qproperty-chunkColor1:%1;}").arg(QUIConfig::NormalColorStart);
    list << QString("DeviceSizeTable{qproperty-chunkColor2:%1;}").arg(QUIConfig::HighColor);
    list << QString("DeviceSizeTable{qproperty-textColor1:%1;}").arg(QUIConfig::TextColor);
    list << QString("DeviceSizeTable{qproperty-textColor2:%1;}").arg(QUIConfig::PanelColor);

    //视频播放控件
    list << QString("AbstractVideoWidget{qproperty-borderWidth:%1;}").arg(borderWidth);
    list << QString("AbstractVideoWidget{qproperty-bgTextSize:%1;}").arg(QUIConfig::FontSize + 10);
    list << QString("AbstractVideoWidget{qproperty-borderColor:%1;}").arg(QUIConfig::BorderColor);
    list << QString("AbstractVideoWidget{qproperty-focusColor:%1;}").arg(QUIConfig::HighColor);
    list << QString("AbstractVideoWidget{qproperty-textColor:%1;}").arg(QUIConfig::TextColor);
    list << QString("AbstractVideoWidget{qproperty-borderColor:%1;}").arg(QUIConfig::BorderColor);

    //背景颜色可以搞个透明度更美观
    list << QString("AbstractVideoWidget{qproperty-bannerBgAlpha:%1;}").arg(200);
    list << QString("AbstractVideoWidget{qproperty-bannerBgColor:%1;}").arg(QUIConfig::BorderColor);
    list << QString("AbstractVideoWidget{qproperty-bannerTextColor:%1;}").arg(QUIConfig::TextColor);
    list << QString("AbstractVideoWidget{qproperty-bannerPressColor:%1;}").arg(QUIConfig::HighColor);

    //视频回放控件
    list << QString("VideoPlayback{qproperty-bgColor:%1;qproperty-videoBgColor:%1;}").arg(QUIConfig::PanelColor);
    list << QString("VideoPlayback{qproperty-textColor:%1;qproperty-videoTextColor:%1;}").arg(QUIConfig::TextColor);
    list << QString("VideoPlayback{qproperty-videoChColor:%1;}").arg(QUIConfig::NormalColorStart);
    list << QString("VideoPlayback{qproperty-videoDataColor:%1;}").arg(QUIConfig::HighColor);

    //自定义面板标题控件
    list << QString("NavTitle{qproperty-bgColor:%1;}").arg(QUIConfig::NormalColorStart);
    list << QString("NavTitle{qproperty-textColor:%1;}").arg(QUIConfig::TextColor);
    list << QString("NavTitle{qproperty-borderColor:%3;}").arg(QUIConfig::BorderColor);
    list << QString("NavTitle{qproperty-iconNormalColor:%1;}").arg(QUIConfig::TextColor);
    list << QString("NavTitle{qproperty-iconHoverColor:%1;}").arg(QUIConfig::HighColor);
    list << QString("NavTitle{qproperty-iconPressColor:%1;}").arg(QUIConfig::BorderColor);
}

void CommonStyle::addNavPageStyle(QStringList &list, int pageButtonCount)
{
    //分页导航
    list << QString("NavPage{qproperty-pageButtonCount:%1;qproperty-showLabInfo:%2;}")
         .arg(pageButtonCount).arg("true");
    list << QString("NavPage{qproperty-fontSize:%1;qproperty-borderWidth:%2;qproperty-borderRadius:%3;qproperty-borderColor:%4;}")
         .arg(QUIConfig::FontSize + 5).arg(0).arg(5).arg(QUIConfig::BorderColor);
    list << QString("NavPage{qproperty-normalBgColor:%1;qproperty-normalTextColor:%2;}")
         .arg(QUIConfig::NormalColorEnd).arg(QUIConfig::TextColor);
    list << QString("NavPage{qproperty-hoverBgColor:%1;qproperty-hoverTextColor:%2;}")
         .arg(QUIConfig::DarkColorStart).arg(QUIConfig::TextColor);
    list << QString("NavPage{qproperty-pressedBgColor:%1;qproperty-pressedTextColor:%2;}")
         .arg(QUIConfig::DarkColorEnd).arg(QUIConfig::TextColor);
    list << QString("NavPage{qproperty-checkedBgColor:%1;qproperty-checkedTextColor:%2;}")
         .arg(QUIConfig::DarkColorEnd).arg(QUIConfig::TextColor);
}

void CommonStyle::addNavBtnStyle(QStringList &list, int topBtnRadius, int leftBtnRadius)
{
    //顶部导航按钮,可以自行修改圆角角度,采用弱属性机制[flag=\"btnNavTop\"],也可采用对象名#widgetBtn
    list << QString("QWidget[flag=\"btnNavTop\"]>QAbstractButton{font-size:%1px;border-radius:%2px;}")
         .arg(QUIConfig::FontSize + 3).arg(topBtnRadius);
    list << QString("QWidget[flag=\"btnNavTop\"]>QAbstractButton{background:%1;border:2px solid %2;}")
         .arg("transparent").arg("transparent");
    //悬停和选中可以分开不同颜色
    list << QString("QWidget[flag=\"btnNavTop\"]>QAbstractButton:hover{background:%1;border:2px solid %2;}")
         .arg(QUIConfig::DarkColorEnd).arg(QUIConfig::BorderColor);
    list << QString("QWidget[flag=\"btnNavTop\"]>QAbstractButton:checked{background:%1;border:2px solid %2;}")
         .arg(QUIConfig::DarkColorEnd).arg(QUIConfig::BorderColor);

    //左侧导航按钮,可以自行修改圆角角度,采用弱属性机制[flag=\"btnNavLeft\"],也可采用对象名#widgetLeft
    list << QString("QWidget[flag=\"btnNavLeft\"]>QAbstractButton{font-size:%1px;border-radius:%2px;}")
         .arg(QUIConfig::FontSize + 3).arg(leftBtnRadius);
    list << QString("QWidget[flag=\"btnNavLeft\"]>QAbstractButton{background:%1;border:2px solid %2;}")
         .arg("transparent").arg("transparent");
    //悬停和选中可以分开不同颜色
    list << QString("QWidget[flag=\"btnNavLeft\"]>QAbstractButton:hover{background:%1;border:2px solid %2;}")
         .arg(QUIConfig::DarkColorEnd).arg(QUIConfig::BorderColor);
    list << QString("QWidget[flag=\"btnNavLeft\"]>QAbstractButton:checked{background:%1;border:2px solid %2;}")
         .arg(QUIConfig::DarkColorEnd).arg(QUIConfig::BorderColor);
}

void CommonStyle::addSwitchButtonStyle(QStringList &list, const QString &styleName, int btnWidth, int btnHeight)
{
    //尺寸大小
    list << QString("SwitchButton{min-width:%1px;max-width:%1px;min-height:%2px;max-height:%2px;}")
         .arg(btnWidth).arg(btnHeight);

    //加深的样式颜色需要反着来效果更明显
    bool dark = QUIStyle::isDark1(styleName);
    if (dark) {
        list << QString("SwitchButton{qproperty-bgColorOn:%1;qproperty-bgColorOff:%2;}")
             .arg(QUIConfig::DarkColorEnd).arg(QUIConfig::NormalColorEnd);
    } else {
        list << QString("SwitchButton{qproperty-bgColorOn:%1;qproperty-bgColorOff:%2;}")
             .arg(QUIConfig::NormalColorEnd).arg(QUIConfig::DarkColorEnd);
    }

    //禁用状态设置颜色透明度区分
    QColor color(QUIConfig::TextColor);
    color.setAlpha(100);
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    QString textColor = color.name(QColor::HexArgb);
#else
    QString textColor = QUIConfig::PanelColor;
#endif
    list << QString("SwitchButton{qproperty-textColorOn:%1;qproperty-textColorOff:%2;}")
         .arg(QUIConfig::TextColor).arg(textColor);
    list << QString("SwitchButton{qproperty-sliderColorOn:%1;qproperty-sliderColorOff:%2;}")
         .arg(QUIConfig::PanelColor).arg(QUIConfig::PanelColor);
}

void CommonStyle::addDarkStyle(QStringList &list, const QString &styleName)
{
    //加深的样式需要设置不一样的颜色
    bool dark = QUIStyle::isDark2(styleName);
    if (dark) {
        list << QString();
        list << QString("XSlider{qproperty-normalColor:%1;qproperty-grooveColor:%2;qproperty-borderColor:%2;}")
             .arg(QUIConfig::BorderColor).arg(QUIConfig::DarkColorEnd);
        list << QString("XSlider{qproperty-handleColor:%1;qproperty-textColor:%2;}")
             .arg(QUIConfig::TextColor).arg(QUIConfig::BorderColor);
        list << QString("CustomPlot{qproperty-bgColor:%1;qproperty-textColor:%2;qproperty-gridColor:%2;}")
             .arg(QUIConfig::BorderColor).arg(QUIConfig::TextColor);
    } else {
        list << QString("XSlider{qproperty-normalColor:%1;qproperty-grooveColor:%2;qproperty-borderColor:%2;}")
             .arg(QUIConfig::NormalColorStart).arg(QUIConfig::BorderColor);
        list << QString("XSlider{qproperty-handleColor:%1;qproperty-textColor:%2;}")
             .arg(QUIConfig::PanelColor).arg(QUIConfig::TextColor);
        list << QString("CustomPlot{qproperty-bgColor:%1;qproperty-textColor:%2;qproperty-gridColor:%2;}")
             .arg(QUIConfig::NormalColorStart).arg(QUIConfig::TextColor);
    }

    //重新设置单元格等item选中和悬停颜色 默认悬停-DarkColorEnd 选中-NormalColorEnd
    //也可以设置两种颜色为一种颜色
    if (dark) {
        QUIConfig::HoverBgColor = QUIConfig::NormalColorEnd;
        QUIConfig::SelectBgColor = QUIConfig::DarkColorEnd;
    }

    //哪个最后设置则 选中+悬停 状态就用哪个颜色
    QUIConfig::HoverCoverSelected = false;
    if (QUIConfig::HoverCoverSelected) {
        list << QString("QTableView::item:selected,QListView::item:selected,QTreeView::item:selected{background:%1;}").arg(QUIConfig::SelectBgColor);
        list << QString("QTableView::item:hover,QListView::item:hover,QTreeView::item:hover{background:%1;}").arg(QUIConfig::HoverBgColor);
    } else {
        list << QString("QTableView::item:hover,QListView::item:hover,QTreeView::item:hover{background:%1;}").arg(QUIConfig::HoverBgColor);
        list << QString("QTableView::item:selected,QListView::item:selected,QTreeView::item:selected{background:%1;}").arg(QUIConfig::SelectBgColor);
    }

    //设备面板
    list << QString("PanelItem{qproperty-titleColor:%1;}").arg(QUIConfig::PanelColor);
    list << QString("PanelItem{qproperty-borderColor:%1;}").arg(QUIConfig::HighColor);
    list << QString("PanelItem{qproperty-titleDisableColor:%1;}").arg(QUIConfig::DarkColorEnd);
    list << QString("PanelItem{qproperty-borderDisableColor:%1;}").arg(QUIConfig::BorderColor);

    //进度条
#ifdef __arm__
    int sliderHeight = 16;
#else
    int sliderHeight = 12;
#endif
    list << QString("XSlider{qproperty-sliderHeight:%1;qproperty-showText:%2;}").arg(sliderHeight).arg(false);
    list << QString("#sliderPtzStep,#sliderFps{qproperty-sliderHeight:%1;qproperty-showText:%2;}").arg(15).arg(true);
}

void CommonStyle::addGaugeCloudStyle(QStringList &list, const QString &styleName)
{
    //根据不同的样式设置不同的风格
    if (styleName.contains("blackvideo")) {
        list << QString("GaugeCloud{qproperty-cloudStyle:CloudStyle_Black;}");
    } else if (styleName.contains("blackblue") || styleName.contains("darkblue")) {
        list << QString("GaugeCloud{qproperty-cloudStyle:CloudStyle_Blue;}");
    } else if (styleName.contains("purple")) {
        list << QString("GaugeCloud{qproperty-cloudStyle:CloudStyle_Purple;}");
    } else if (styleName.contains("silvery") || styleName.contains("gray") || styleName.contains("lightgray") || styleName.contains("flatwhite")) {
        list << QString("GaugeCloud{qproperty-cloudStyle:CloudStyle_White;}");
    } else {
        //设置为自定义风格
        list << QString("GaugeCloud{qproperty-cloudStyle:CloudStyle_Custom;}");
        //根据样式的不同颜色设置云台的颜色
        list << QString("GaugeCloud{qproperty-baseColor:%1;qproperty-bgColor:%2;qproperty-arcColor:%3;}")
             .arg(QUIConfig::PanelColor).arg(QUIConfig::BorderColor).arg(QUIConfig::NormalColorStart);
        list << QString("GaugeCloud{qproperty-borderColor:%1;qproperty-textColor:%2;}")
             .arg(QUIConfig::HighColor).arg(QUIConfig::TextColor);
    }

    //云台控件鼠标进入+按下颜色设置
    list << QString("GaugeCloud{qproperty-enterColor:%1;qproperty-pressColor:%2;}").arg("#47CAF6").arg(QUIConfig::HighColor);
}

void CommonStyle::addFormStyle(QStringList &list, const QString &styleName, int btnMinWidth)
{
    //中英文标题字体加粗
    list << QString("#labTitleCn{font-size:%1px;}").arg(QUIConfig::FontSize + 13);
    list << QString("#labTitleEn{font-size:%1px;}").arg(QUIConfig::FontSize + 2);

    //停靠窗体自定义标题栏字体放大
    list << QString("#dockTitle{font:%1px;min-height:%2px;}").arg(QUIConfig::FontSize + 3).arg(20);
    list << QString("QLabel[flag=\"title\"]{border:none;padding:5px;}");

    //添加删除保存清空等一排按钮最小宽度
    list << QString("QWidget[flag=\"navbtn\"] QPushButton{min-width:%1px;}").arg(btnMinWidth);

    //登录登出窗体软件标题样式
    QString textColor = "#FFFFFF";
    if (styleName.contains("darkblue") || styleName.contains("blackblue") || styleName.contains("purple")) {
        textColor = QUIConfig::TextColor;
    }
    list << QString("#frmLogin>#labName,#frmLogout>#labName{font:22px;color:%1;}").arg(textColor);

    //委托中的按钮样式
    list << QString("#DbDelegate_pushButton{border-radius:0px;margin:1px;padding:1px;}");

    //主背景
    list << QString("QWidget#widgetMain{background:%1;}").arg(QUIConfig::BorderColor);
    //右上角菜单
    list << QString("QWidget#widgetMenu>QPushButton{border-radius:0px;padding:0px;margin:1px 1px 2px 1px;}");
}
