#include <gtest/gtest.h>
#define UNIT_TESTS 1
#include "../device.cpp"


static constexpr double EPS = 1e-2; // = POSSIBLE_ERROR

// --- 1. Stream: имя формируется "s<номер>"
TEST(StreamUnit, AutoNameFromIndex) {
    Stream s1(1);
    EXPECT_EQ(s1.getName(), std::string("s1"));
}

// --- 2. Stream: set/get массового расхода
TEST(StreamUnit, MassFlowSetGet) {
    Stream s2(2);
    s2.setMassFlow(12.5);
    EXPECT_NEAR(s2.getMassFlow(), 12.5, EPS);
}

// --- 3. Mixer: один выход = сумма входов
TEST(MixerIntegration, SingleOutputEqualsSumOfInputs) {
    streamcounter = 0;

    Mixer d1(2);
    auto s1 = std::make_shared<Stream>(++streamcounter);
    auto s2 = std::make_shared<Stream>(++streamcounter);
    auto out = std::make_shared<Stream>(++streamcounter);

    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(out);

    d1.updateOutputs();

    EXPECT_NEAR(out->getMassFlow(), 15.0, EPS);
}

// --- 4. Mixer: лишний выход -> исключение std::string("Too much outputs")
TEST(MixerGuards, TooManyOutputsThrowsStdString) {
    streamcounter = 0;

    Mixer d1(2);
    auto s1 = std::make_shared<Stream>(++streamcounter);
    auto s2 = std::make_shared<Stream>(++streamcounter);
    auto out1 = std::make_shared<Stream>(++streamcounter);
    auto out2 = std::make_shared<Stream>(++streamcounter);

    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(out1);

    try {
        d1.addOutput(out2);
        FAIL() << "Expected std::string exception";
    } catch (const std::string& ex) {
        EXPECT_EQ(ex, "Too much outputs");
    } catch (...) {
        FAIL() << "Expected std::string, got something else";
    }
}

// --- 5. Mixer: лишний вход -> исключение std::string("Too much inputs")
TEST(MixerGuards, TooManyInputsThrowsStdString) {
    streamcounter = 0;

    Mixer d1(2);
    auto s1 = std::make_shared<Stream>(++streamcounter);
    auto s2 = std::make_shared<Stream>(++streamcounter);
    auto s3 = std::make_shared<Stream>(++streamcounter);
    auto out = std::make_shared<Stream>(++streamcounter);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(out);

    try {
        d1.addInput(s3);
        FAIL() << "Expected std::string exception";
    } catch (const std::string& ex) {
        EXPECT_EQ(ex, "Too much inputs");
    } catch (...) {
        FAIL() << "Expected std::string, got something else";
    }
}

// --- 6. Reactor: лишний выход при одинарном реакторе -> исключение const char*
TEST(ReactorGuards, TooManyOutputsThrowsCharPtr) {
    streamcounter = 0;

    Reactor rx(false); // один выход
    auto in   = std::make_shared<Stream>(++streamcounter);
    auto out1 = std::make_shared<Stream>(++streamcounter);
    auto out2 = std::make_shared<Stream>(++streamcounter);

    in->setMassFlow(10.0);

    rx.addInput(in);
    rx.addOutput(out1);

    try {
        rx.addOutput(out2);
        FAIL() << "Expected const char* exception";
    } catch (const char* ex) {
        EXPECT_STREQ(ex, "OUTPUT STREAM LIMIT!");
    } catch (...) {
        FAIL() << "Expected const char*";
    }
}

// --- 7. Reactor: закон сохранения и равномерное деление при двух выходах
TEST(ReactorIntegration, MassConservationAndEvenSplit) {
    streamcounter = 0;

    Reactor rx(true); // два выхода
    auto in   = std::make_shared<Stream>(++streamcounter);
    auto o1   = std::make_shared<Stream>(++streamcounter);
    auto o2   = std::make_shared<Stream>(++streamcounter);

    in->setMassFlow(10.0);

    rx.addInput(in);
    rx.addOutput(o1);
    rx.addOutput(o2);

    rx.updateOutputs();

    EXPECT_NEAR(o1->getMassFlow() + o2->getMassFlow(), in->getMassFlow(), EPS);
    EXPECT_NEAR(o1->getMassFlow(), 5.0, EPS);
    EXPECT_NEAR(o2->getMassFlow(), 5.0, EPS);
}
