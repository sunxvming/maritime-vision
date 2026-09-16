#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "head.h"

class AppConfig
{
public:
    static QString ConfigFile;      //配置文件文件路径及名称

    //基本配置参数1
    static int IndexStart;          //启动窗体索引 0-主程序 1-演示示例
    static int WorkMode;            //软件工作模式 0-视频监控 1-机器人监控 2-无人机监控
    static int NavStyle;            //导航按钮样式 0-左侧+左侧 1-左侧+上侧 2-上侧+左侧 3-上侧+上侧
    static QString StyleName;       //样式名称
    static QString LogoImage;       //软件图标
    static QString Copyright;       //版权所有
    static QString TitleCn;         //软件中文名称
    static QString TitleEn;         //软件英文名称


    static QString WebServerUrl;         //推理服务器地址
    static QString AIServerUrl;         //推理服务器地址

    //基本配置参数2
    static int RecordsPerpage;      //表格分页时每页数据
    static int PageButtonCount;     //分页页码按钮个数
    static int MsgListCount;        //图文警情最大行数
    static int MsgTableCount;       //表格消息最大行数
    static int TimeHideCursor;      //程序多久未操作自动隐藏光标
    static int TimeAutoFull;        //程序多久未操作自动全屏界面
    static int AlarmSoundCount;     //报警声音次数 0-不启用
    static int WindowOpacity;       //停靠窗体透明度
    static int TipInterval;         //弹框显示的时间 单位秒 0表示一直显示除非手动关闭 10000-表示禁用

    //基本配置参数3
    static bool AutoRun;            //自动登录
    static bool AutoLogin;          //自动登录
    static bool AutoPwd;            //记住密码
    static bool SaveLog;            //日志输出
    static bool SaveRunTime;        //运行时间记录
    static bool RightInfo;          //右上角提示信息显示
    static bool SaveUrl;            //保存播放记录

    //基本配置参数4
    static bool FormMax;            //主界面最大化
    static bool FormFull;           //主界面全屏
    static QRect FormGeometry;      //最后窗体位置区域
    static QRect PollGeometry;      //轮询提示面板区域
    static QString SelectDirName;   //最后选择的目录
    static QString ExpandItem;      //展开的节点集合

    //基本配置参数5
    static QString LastLoginer;     //最后登录用户
    static QString LastFormMain;    //最后的主界面
    static QString LastFormVideo;   //最后的视频回放界面
    static QString LastFormMap;     //最后的电子地图界面
    static QString LastFormData;    //最后的日志查询界面
    static QString LastFormConfig;  //最后的系统设置界面

    //可视化配置参数
    static bool FullScreen;         //全屏模式
    static bool VisibleIpcSearch;   //摄像机搜索界面显示
    static bool VisiblePollPlus;    //轮询点批量界面显示

    //视频配置参数1
    static int ScaleMode;           //图片缩放模式
    static int VideoMode;           //视频显示模式
    static int BorderWidth;         //视频通道边框
    static int BannerPosition;      //悬浮工具位置
    static int SoundValue;          //最后声音大小
    static bool SoundMuted;         //最后静音状态
    static bool SharedData;         //共享解码线程

    //视频配置参数2
    static bool PlayAudio;          //解码播放声音
    static QString Hardware;        //硬件加速名称
    static QString Transport;       //网络传输协议
    static int DecodeType;          //解码速度策略
    static int Caching;             //数据缓存大小
    static int ReadTimeout;         //采集超时时间
    static int ConnectTimeout;      //连接超时时间

    //视频配置参数3
    static int VideoCount;          //视频通道数
    static QString VideoType;       //视频几画面
    static bool VideoMax;           //视频最大化
    static bool VideoDrag;          //通道交换
    static bool SaveVideo;          //保存视频
    static bool VideoSmart;         //人工智能
    static bool OnvifEvent;         //事件订阅
    static bool OnvifNtp;           //自动校时

    //视频配置参数4
    static bool CheckOffline;       //检测离线
    static bool TreeBig;            //设备列表放大
    static bool TreeRtsp;           //显示码流节点
    static int RtspType;            //默认码流类型
    static int AlarmSaveTime;       //报警视频保存时间(单位秒)
    static int OpenIpcPolicy;       //双击打开设备对应策略 0-最后空白通道 1-最后按下通道
    static int OpenNvrPolicy;       //双击打开分组对应策略 0-最后空白通道 1-从头清空通道
    static int OpenMaxCount;        //双击打开分组最大数量 0-最大通道数量 1-当前通道数量 2-自动调整数量
    static int TableDataPolicy;     //表格单元数据编辑策略 0-单选按下编辑 1-多选双击编辑

    //启用功能配置参数1
    static bool EnableChannel1;     //启用1画面切换
    static bool EnableChannel4;     //启用4画面切换
    static bool EnableChannel6;     //启用6画面切换
    static bool EnableChannel8;     //启用8画面切换
    static bool EnableChannel9;     //启用9画面切换
    static bool EnableChannel13;    //启用13画面切换
    static bool EnableChannel16;    //启用16画面切换
    static bool EnableChannel25;    //启用25画面切换
    static bool EnableChannel36;    //启用36画面切换
    static bool EnableChannel64;    //启用64画面切换


    //视频轮询配置参数
    static bool AutoPoll;           //启动后自动识别最后轮询状态
    static bool Polling;            //是否处于轮询状态
    static int PollInterval;        //轮询间隔(单位秒)
    static int PollType;            //轮询画面
    static int PollRtsp;            //码流类型 0-主码流 1-子码流
    static QString PollGroupNames;  //轮询分组名称集合
    static QString PollGroupLast;   //最后选中轮询分组

    //批量生成配置参数
    static int PlusType;            //格式类型
    static int PlusNumber;          //添加数量
    static bool PlusNvr;            //通道递增(nvr才有)
    static QString PlusAddr;        //起始地址
    static QString PlusMain;        //主码流格式
    static QString PlusSub;         //子码流格式

    //设备回放配置参数
    static int NvrCopyrightName;    //厂家索引
    static int NvrVideoType;        //类型索引
    static QString NvrUserName;     //用户名称
    static QString NvrUserPwd;      //用户密码
    static QString NvrVideoIP;      //设备地址

    //设备搜索配置参数
    static QString SearchUserName;  //用户名称
    static QString SearchUserPwd;   //用户密码
    static QString SearchLocalIP;   //指定网卡
    static QString SearchDeviceIP;  //指定设备
    static QString SearchFilter;    //条件过滤
    static QString SearchFilters;   //条件集合
    static int SearchInterval;      //搜索间隔(单位毫秒)
    static int SearchTimeout;       //超时时间(单位毫秒)
    static bool SearchClear;        //自动清空(每次都重新搜索)

    //本地数据库配置参数
    static bool LocalAutoInfo;      //自动填入端口和用户信息
    static int LocalConnMode;       //本地数据库连接方式
    static QString LocalDbType;     //本地数据库主机类型
    static QString LocalDbName;     //本地数据库数据库名
    static QString LocalHostName;   //本地数据库主机地址
    static int LocalHostPort;       //本地数据库通信端口
    static QString LocalUserName;   //本地数据库用户名称
    static QString LocalUserPwd;    //本地数据库用户密码

    //网络地图配置参数
    static QString MapVersionKey;   //地图秘钥
    static QString MapCenter;       //地图中心点坐标
    static int MapZoom;             //地图缩放级别

    //路径规划配置参数
    static QString RouteStartAddr;  //路径规划起点坐标
    static QString RouteEndAddr;    //路径规划终点坐标
    static int RouteType;           //路径规划策略方式
    static int RoutePointCount;     //路径规划目标点数
    static QString RouteDeviceName; //路径规划设备名称
    static int RouteMoveInterval;   //路径规划移动间隔
    static int RouteMoveMode;       //路径规划轨迹模式

    //文件上传配置参数
    static QString UploadHost;      //文件服务器地址
    static int UploadPort;          //文件服务器端口

    //各种颜色配置参数
    static QString ColorTextNormal; //常规文字颜色
    static QString ColorTextWarn;   //警告文字颜色
    static QString ColorTextAlarm;  //报警文字颜色
    static QString ColorIconAlarm;  //报警图标颜色
    static QString ColorMsgWait;    //待处理颜色
    static QString ColorMsgSolved;  //已处理颜色


    //网络通信配置参数
    static QString TcpServerHost;   //Tcp通信地址
    static int TcpServerPort;       //Tcp通信端口
    static int TcpListenPort;       //Tcp监听端口
    static int UdpListenPort;       //Udp监听端口


    //演示示例配置参数
    static int IndexDemo;           //演示示例索引
    static QString VideoImageUrl;   //视频图片目录
    static QString VideoWidgetUrl;  //视频控件地址
    static QString VideoSaveUrl;    //视频保存地址
    static QString VideoPlusUrl1;   //视频叠加地址1
    static QString VideoPlusUrl2;   //视频叠加地址2
    static QString VideoPlayUrl1;   //同步播放地址1
    static QString VideoPlayUrl2;   //同步播放地址2
    static QString VideoPlayUrl3;   //同步播放地址3
    static QString VideoPlayUrl4;   //同步播放地址4
    static QString VideoAudioUrl;   //音频频谱地址
    static QString VideoDragUrl;    //拖曳播放地址

    //读写软件配置文件
    static void readConfig();       //读取配置文件,在main函数最开始加载程序载入
    static void checkConfig();      //检验配置参数,比如限定最小值最大值重新校正配置参数等
    static void writeConfig();      //写入配置文件,在更改配置文件程序关闭时调用

    //读写布局配置文件
    static QByteArray readLayout(const QString &file);
    static void writeLayout(const QString &file, const QByteArray &data);
};

#endif // APPCONFIG_H
