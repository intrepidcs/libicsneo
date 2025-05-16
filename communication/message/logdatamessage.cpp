#include "icsneo/communication/message/logdatamessage.h"
#include <iostream>

using namespace icsneo;

std::shared_ptr<LogDataMessage> LogDataMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
    if(bytestream.size() % 2 != 0)
        return nullptr;
    const auto* begin = (char16_t*)bytestream.data();
    const auto* end = begin + (bytestream.size() / sizeof(char16_t));
    return std::make_shared<LogDataMessage>(std::wstring(begin,end));
}