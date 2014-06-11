#ifndef ENUMS_H
#define ENUMS_H

class Enums
{
public:
    Enums();

    enum Command // actual base-16 int
    {
        PrintTextCommand = 0x00,
        MessageCommand = 0x01,
        JoinCommand = 0x02,
        UnjoinCommand = 0x03,
        UserListCommand = 0x04,
        ChannelListCommand = 0x05,
        JoinChannelCommand = 0x06,
        LeaveChannelCommand = 0x07,
        UserJoinedChannelCommand = 0x08,
        UserLeftChannelCommand = 0x09,
        CreateChannelCommand = 0x0A,
        RemoveChannelCommand = 0x0B,
        SetChatImageCommand = 0x0C,
        ImageCommand = 0x0D,
        InfoCommand = 0x0E
    };

    enum Length // number of bits
    {
        MessageLength = 4,
        NameLength = 2,
        NameListLength = 8,
        IdLength = 2,
        IdListLength = 8,
        ColorLength = 2,
        ColorListLength = 8,
        UserListLength = 8,
        ChannelIdLength = 2,
        ChannelIdListLength = 8,
        ChannelNameLength = 2,
        ChannelNameListLength = 8,
        ChatImageLength = 16,
        ImageLength = 16,
        SpriteLength = 2,
        SpriteListLength = 8
    };
};

#endif // ENUMS_H
