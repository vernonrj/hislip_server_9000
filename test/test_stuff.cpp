#include <gtest/gtest.h>

#include "message.h"
#include "buttstream.h"
#include "scpi_parser.h"

TEST(MessageTest, Default) {
    hs::Message m;
    ASSERT_EQ(m, (hs::Message {
        .message_type = hs::MessageType::Initialize,
        .control_code = 0x0,
        .message_param = 0x0,
        .data = ""
    }));
}

TEST(MessageTest, ParseInitialize) {
    const char init[] = "HS" "\x00" "\x00" "\x06\x09" "RS" "\x00\x00\x00\x00\x00\x00\x00\x00" "";
    ASSERT_EQ(hs::Message::from_bytes(std::string_view(std::begin(init), std::end(init))),
              (hs::Message {
                  .message_type = hs::MessageType::Initialize,
                  .control_code = 0x0,
                  .message_param = 0x06095253,
                  .data = ""
              }));
}

TEST(MessageTest, ToInitialize) {
    const char init[] = "HS" "\x00" "\x00" "\x06\x09" "RS" "\x00\x00\x00\x00\x00\x00\x00\x00";
    ASSERT_EQ(hs::Message::from_bytes(std::string_view(std::begin(init), std::end(init))).to_bytes(), std::string(std::begin(init), std::end(init)-1));
}

TEST(MessageTest, ParseInitializeResponse) {
    const char init[] = "HS" "\x01" "\x01" "\x02\x00" "\x04\x20" "\x00\x00\x00\x00\x00\x00\x00\x00" "";
    ASSERT_EQ(hs::Message::from_bytes(std::string_view(std::begin(init), std::end(init))),
              (hs::Message {
                  .message_type = hs::MessageType::InitializeResponse,
                  .control_code = 0x1,
                  .message_param = 0x02000420,
                  .data = ""
              }));
}

TEST(MessageTest, ToInitializeResponse) {
    const char init[] = "HS" "\x01" "\x01" "\x02\x00" "\x04\x20" "\x00\x00\x00\x00\x00\x00\x00\x00";
    ASSERT_EQ(hs::Message::from_bytes(std::string_view(std::begin(init), std::end(init))).to_bytes(), std::string(std::begin(init), std::end(init)-1));
}


TEST(MessageTest, ParseFatalError) {
    const char init[] = "HS" "\x02" "\x03" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x25" "oops looks like something went wrong!";
    ASSERT_EQ(hs::Message::from_bytes(std::string_view(std::begin(init), std::end(init))),
              (hs::Message {
                  .message_type = hs::MessageType::FatalError,
                  .control_code = static_cast<uint8_t>(hs::FatalErrorCode::InvalidInitSeq),
                  .message_param = 0x0,
                  .data = "oops looks like something went wrong!"
              }));
}

TEST(MessageTest, ToFatalError) {
    const char init[] = "HS" "\x02" "\x03" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x25" "oops looks like something went wrong!";
    ASSERT_EQ(hs::Message::from_bytes(std::string_view(std::begin(init), std::end(init))).to_bytes(), std::string(std::begin(init), std::end(init)-1));
}

TEST(MessageTest, ParseError) {
    const char init[] = "HS" "\x03" "\x02" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x25" "oops looks like something went wrong!";
    ASSERT_EQ(hs::Message::from_bytes(std::string_view(std::begin(init), std::end(init))),
              (hs::Message {
                  .message_type = hs::MessageType::Error,
                  .control_code = static_cast<uint8_t>(hs::ErrorCode::UnrecognizedControlCode),
                  .message_param = 0x0,
                  .data = "oops looks like something went wrong!"
              }));
}

TEST(MessageTest, ToError) {
    const char init[] = "HS" "\x03" "\x02" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x25" "oops looks like something went wrong!";
    ASSERT_EQ(hs::Message::from_bytes(std::string_view(std::begin(init), std::end(init))).to_bytes(), std::string(std::begin(init), std::end(init)-1));
}

TEST(Buttreader, ToU8) {
    std::string s = "\x05\x10\x04";
    ButtstreamReader b{std::string_view(s)};
    uint8_t value{};
    b >> value;
    ASSERT_EQ(value, 0x05);
    b >> value;
    ASSERT_EQ(value, 0x10);
    b >> value;
    ASSERT_EQ(value, 0x04);
    try {
        b >> value;
        FAIL();
    } catch (...) {
        /* good */
    }
}

TEST(ButtReader, ToU16) {
    std::string s = "\x05\x10\x04\x15\x23\x55\x04";
    ButtstreamReader b{std::string_view(s)};
    uint16_t value{};
    b >> value;
    ASSERT_EQ(value, 0x0510);
    b >> value;
    ASSERT_EQ(value, 0x0415);
    b >> value;
    ASSERT_EQ(value, 0x2355);
    try {
        b >> value;
        FAIL();
    } catch (...) {
        /* good */
    }
}

TEST(ButtReader, ToU32) {
    std::string s = "\x05\x10\x04\x15" "\x23\x55\x04\xf3" "\x42\x69\x05\x88" "\x36\x99\x99";
    ButtstreamReader b{std::string_view(s)};
    uint32_t value{};
    b >> value;
    ASSERT_EQ(value, 0x05100415);
    b >> value;
    ASSERT_EQ(value, 0x235504f3);
    b >> value;
    ASSERT_EQ(value, 0x42690588);
    try {
        b >> value;
        FAIL();
    } catch (...) {
        /* good */
    }
}

TEST(ButtReader, ToU64) {
    std::string s = "\x05\x10\x04\x15\x23\x55\x04\xf3" "\x42\x69\x05\x88\x36\x99\x99\x85" "\x50\x43\x01\x44\x60\xcc\xef";
    ButtstreamReader b{std::string_view(s)};
    uint64_t value{};
    b >> value;
    ASSERT_EQ(value, 0x05100415235504f3ULL);
    b >> value;
    ASSERT_EQ(value, 0x4269058836999985ULL);
    try {
        b >> value;
        FAIL();
    } catch (...) {
        /* good */
    }
}

TEST(ButtReader, Rest) {
    std::string s = "\x05\x10\x04\x15\x23\x55\x04\x56\x27";
    ButtstreamReader b{std::string_view(s)};
    uint16_t value{};
    b >> value;
    ASSERT_EQ(value, 0x0510);
    b >> value;
    ASSERT_EQ(value, 0x0415);
    b >> value;
    ASSERT_EQ(value, 0x2355);
    ASSERT_EQ(std::move(b).rest_of_bytes(), "\x04\x56\x27");
    try {
        b >> value;
        FAIL() << "got unexpected bytes: " << value;
    } catch (...) {
        /* good */
    }
}

TEST(ScpiParser, command) {
    ASSERT_EQ(*scpi::parse("MMEM:CAT"), (scpi::Command {.header = "MMEM:CAT"}));
    ASSERT_EQ(*scpi::parse("MMEM:CAT:LOL:GOT? 5"), (scpi::Command {.header = "MMEM:CAT:LOL:GOT?", .args = {"5"}}));
}

TEST(ScpiParser, StarCommand) {
    ASSERT_EQ(*scpi::parse("*OPC"), (scpi::Command {.header = "*OPC"}));
    ASSERT_EQ(*scpi::parse("*OPC?"), (scpi::Command {.header = "*OPC?"}));
    ASSERT_EQ(*scpi::parse("*TST? 5"), (scpi::Command {.header = "*TST?", .args = {"5"}}));
}

TEST(ScpiParser, NormHeader) {
    ASSERT_EQ((scpi::Command {.header = "TEST:THIS"}).norm_header(), ":TEST:THIS");
    ASSERT_EQ((scpi::Command {.header = "TEST:THIS?"}).norm_header(), ":TEST:THIS?");
    ASSERT_EQ((scpi::Command {.header = "TESTing:THISthing"}).norm_header(), ":TEST:THIS");
    ASSERT_EQ((scpi::Command {.header = "TESTing:THISthing?"}).norm_header(), ":TEST:THIS?");
    ASSERT_EQ((scpi::Command {.header = "mmemory:catalog"}).norm_header(), ":MMEM:CAT");
    ASSERT_EQ((scpi::Command {.header = "mmemory:catalog?"}).norm_header(), ":MMEM:CAT?");
}