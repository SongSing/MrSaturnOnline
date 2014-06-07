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
        JoinCommand = 0x01,
        UnjoinCommand = 0x02,
        UserListCommand = 0x03,
        ChannelListCommand = 0x04,
        JoinChannelCommand = 0x05,
        UnjoinChannelCommand = 0x06,
        UserJoinedChannelCommand = 0x07,
        UserLeftChannelCommand = 0x08
    };

    enum Length // number of bits
    {
        MessageLength = 4,
        NameLength = 2,
        NameListLength = 8,
        IdLength = 2,
        IdListLength = 8,
        ColourLength = 2,
        ColourListLength = 8,
        UserListLength = 8,
        ChannelIdLength = 2,
        ChannelIdListLength = 8,
        ChannelNameLength = 2,
        ChannelNameListLength = 8,
    };
};

#endif // ENUMS_H
