
// 消息体
struct Message{
    // 消息类型:0->服务器消息,1->用户消息
    int message_flag;
    // 消息内容
    char content[256];
};