#include <cstring>
// 消息体
struct Message{
    // 消息类型:0->系统消息,1->用户消息
    int message_flag;
    // 消息内容
    char content[256];
    // 发送者若为系统消息,则为空
    char source[64];
    // 接收者,若为系统消息,则为空,若为群聊消息,则为all
    char dest[64];
};

// 定义文件信息的结构体。
struct st_fileinfo
{
    char filename[256]; // 文件名。
    int filesize;       // 文件大小。
};

extern const std::string quit = "quit";
extern const std::string all = "all";