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

// --- 8. Mixer: updateOutputs без выходов -> std::string exception
TEST(MixerGuards, UpdateWithoutOutputsThrowsString) {
    streamcounter = 0;
    Mixer d1(1);
    auto in = std::make_shared<Stream>(++streamcounter);
    in->setMassFlow(3.0);
    d1.addInput(in);
    EXPECT_THROW(d1.updateOutputs(), std::string);
}

// --- 9. Mixer: два выхода -> равномерное распределение
TEST(MixerIntegration, TwoOutputsEvenSplit) {
    streamcounter = 0;
    Mixer d1(2);
    auto s1 = std::make_shared<Stream>(++streamcounter);
    auto s2 = std::make_shared<Stream>(++streamcounter);
    auto o1 = std::make_shared<Stream>(++streamcounter);
    auto o2 = std::make_shared<Stream>(++streamcounter);

    s1->setMassFlow(6.0);
    s2->setMassFlow(2.0);

    d1.addInput(s1); d1.addInput(s2);
    d1.addOutput(o1); d1.addOutput(o2);

    d1.updateOutputs();

    EXPECT_NEAR(o1->getMassFlow(), 4.0, EPS);
    EXPECT_NEAR(o2->getMassFlow(), 4.0, EPS);
}

// --- 10. Mixer: нет входов, но есть выходы -> на выходе 0
TEST(MixerIntegration, NoInputsGivesZeroOutputs) {
    streamcounter = 0;
    Mixer d1(2);
    auto o = std::make_shared<Stream>(++streamcounter);
    d1.addOutput(o);
    d1.updateOutputs();
    EXPECT_NEAR(o->getMassFlow(), 0.0, EPS);
}

// --- 11. Reactor: лишний вход (ограничение = 1) -> const char* exception
TEST(ReactorGuards, TooManyInputsThrowsCharPtr) {
    streamcounter = 0;
    Reactor rx(false); // 1 вход
    auto in1 = std::make_shared<Stream>(++streamcounter);
    auto in2 = std::make_shared<Stream>(++streamcounter);
    rx.addInput(in1);
    EXPECT_THROW(rx.addInput(in2), const char*);
}

// --- 12. Reactor: один выход = весь вход
TEST(ReactorIntegration, SingleOutputEqualsInput) {
    streamcounter = 0;
    Reactor rx(false);
    auto in  = std::make_shared<Stream>(++streamcounter);
    auto out = std::make_shared<Stream>(++streamcounter);
    in->setMassFlow(7.0);
    rx.addInput(in);
    rx.addOutput(out);
    rx.updateOutputs();
    EXPECT_NEAR(out->getMassFlow(), in->getMassFlow(), EPS);
}

// --- 13. Reactor: вход 0 -> оба выхода 0
TEST(ReactorIntegration, ZeroInputGivesZeroOutputs) {
    streamcounter = 0;
    Reactor rx(true);
    auto in  = std::make_shared<Stream>(++streamcounter);
    auto o1  = std::make_shared<Stream>(++streamcounter);
    auto o2  = std::make_shared<Stream>(++streamcounter);
    in->setMassFlow(0.0);
    rx.addInput(in);
    rx.addOutput(o1);
    rx.addOutput(o2);
    rx.updateOutputs();
    EXPECT_NEAR(o1->getMassFlow(), 0.0, EPS);
    EXPECT_NEAR(o2->getMassFlow(), 0.0, EPS);
}

// --- 14. Stream::print печатает имя (проверяем вывод)
TEST(StreamUnit, PrintWritesName) {
    Stream s(1);
    s.setMassFlow(3.0);
    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    s.print();
    std::cout.rdbuf(old);
    EXPECT_NE(oss.str().find("Stream s1"), std::string::npos);
}

// --- 15. Device getters: возвращают то, что добавили (через Mixer)
TEST(DeviceAPI, GettersReturnInputsOutputs) {
    streamcounter = 0;
    Mixer d1(2);
    auto s1 = std::make_shared<Stream>(++streamcounter);
    auto s2 = std::make_shared<Stream>(++streamcounter);
    auto o  = std::make_shared<Stream>(++streamcounter);
    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(o);
    auto ins = d1.getInputs();
    auto outs = d1.getOutputs();
    ASSERT_EQ(ins.size(), 2u);
    ASSERT_EQ(outs.size(), 1u);
    EXPECT_EQ(ins[0]->getName(), "s1");
    EXPECT_EQ(outs[0]->getName(), "s3");
}