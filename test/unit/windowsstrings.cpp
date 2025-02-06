#include <gtest/gtest.h>

#include <icsneo/platform/windows/strings.h>

#include <string>

TEST(WindowsUtilTest, testConvertWideString) {
#ifdef WIN32
    std::wstring wideString(L"Hello World!");
    auto normalString = icsneo::convertWideString(wideString);

    ASSERT_STREQ(normalString.c_str(), "Hello World!");
#endif
}

TEST(WindowsUtilTest, testConvertStringToWide) {
#ifdef WIN32
    std::string normalString("Hello World!");
    auto wideString = icsneo::convertStringToWide(normalString);

    ASSERT_STREQ(wideString.c_str(), L"Hello World!");
#endif
}