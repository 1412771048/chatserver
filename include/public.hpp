#ifndef PUBLIC_H
#define PUBLIC_H

/*
server和client的公共文件，定义消息类型
*/
enum EnMsgType
{
    LOGIN_MSG = 1, // 登录 {"msgid":1, "id":25,"password":"123456"}
    LOGIN_MSG_ACK, // 登录响应 {"errmsg":"登录失败：用户id不存在 or 密码错误 or 已在线","errno":1,"msgid":2}/ {"errno":0,"id":26,"msgid":2,"name":"li si"}
    LOGINOUT_MSG, // 注销
    REG_MSG, // 注册 {"msgid":4,"name":"li si","password":"123456"}
    REG_MSG_ACK, // 注册响应 {"errno":0,"id":25,"msgid":5} / {"errno":1,"msgid":5}
    ONE_CHAT_MSG, // 单聊 {"msgid":6,"id":25,"from":"zhang san","to":26,"msg":"hello lisi"}
    ADD_FRIEND_MSG, // 添加好友 {"msgid":7,"id":25,"friendid":26}
    CREATE_GROUP_MSG, // 创建群组
    ADD_GROUP_MSG, // 加入群组
    GROUP_CHAT_MSG, // 群聊天
};

#endif
