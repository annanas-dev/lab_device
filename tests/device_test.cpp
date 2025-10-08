#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>

#define UNIT_TESTS 1
#include "../device.cpp"

static constexpr double EPS = 1e-2;

// ---------- Stream ----------
TEST(StreamUnit, AutoNameFromIndex) {
    Stream s1(1);
    EXPECT_EQ(s1.getName(), "s1");
}

TEST(StreamUnit, RenameAndGetMassFlow) {
    Stream s(5);
    s.setName("feed");
    s.setMassFlow(12.5);
    EXPECT_EQ(s.getName(), "feed");
    EXPECT_NEAR(s.getMassFlow(), 12.5, EPS);
}

TEST(StreamUnit, PrintWritesNameAndFlow) {
    Stream s(1);
    s.setMassFlow(3.0);
    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    s.print();
    std::cout.rdbuf(old);
    const auto out = oss.str();
    EXPECT_NE(out.find("Stream s1"), std::string::npos);
    EXPECT_NE(out.find("3"), std::string::npos);
}

// ---------- Mixer ----------
TEST(MixerIntegration, SingleOutputEqualsSumOfInputs) {
    streamcounter = 0;
    Mixer mx(3);                                        // разрешим 3 входа
    auto s1 = std::make_shared<Stream>(++streamcounter);
    auto s2 = std::make_shared<Stream>(++streamcounter);
    auto s3 = std::make_shared<Stream>(++streamcounter);
    auto out = std::make_shared<Stream>(++streamcounter);

    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);
    s3->setMassFlow(2.5);

    mx.addInput(s1); mx.addInput(s2); mx.addInput(s3);  // успех-путь addInput
    mx.addOutput(out);                                  // единственный выход

    mx.updateOutputs();                                 // outputs.size()==1

    EXPECT_NEAR(out->getMassFlow(), 17.5, EPS);
}

TEST(MixerIntegration, NoInputsGivesZeroAtOutput) {
    streamcounter = 0;
    Mixer mx(2);
    auto out = std::make_shared<Stream>(++streamcounter);
    mx.addOutput(out);
    mx.updateOutputs();                                 // суммирование по пустому списку
    EXPECT_NEAR(out->getMassFlow(), 0.0, EPS);
}

TEST(MixerGuards, TooManyInputsThrowsStdString) {
    streamcounter = 0;
    Mixer mx(2);
    auto s1 = std::make_shared<Stream>(++streamcounter);
    auto s2 = std::make_shared<Stream>(++streamcounter);
    auto s3 = std::make_shared<Stream>(++streamcounter);
    auto o  = std::make_shared<Stream>(++streamcounter);
    mx.addInput(s1); mx.addInput(s2);
    mx.addOutput(o);
    EXPECT_THROW(mx.addInput(s3), std::string);         // "Too much inputs"
}

TEST(MixerGuards, TooManyOutputsThrowsStdString) {
    streamcounter = 0;
    Mixer mx(2);
    auto s1 = std::make_shared<Stream>(++streamcounter);
    auto s2 = std::make_shared<Stream>(++streamcounter);
    auto o1 = std::make_shared<Stream>(++streamcounter);
    auto o2 = std::make_shared<Stream>(++streamcounter);
    mx.addInput(s1); mx.addInput(s2);
    mx.addOutput(o1);
    EXPECT_THROW(mx.addOutput(o2), std::string);        // MIXER_OUTPUTS == 1
}

TEST(MixerGuards, UpdateWithoutOutputsThrowsStdString) {
    streamcounter = 0;
    Mixer mx(1);
    auto s = std::make_shared<Stream>(++streamcounter);
    s->setMassFlow(1.0);
    mx.addInput(s);
    EXPECT_THROW(mx.updateOutputs(), std::string);      // "Should set outputs before update"
}

// ---------- Reactor ----------
TEST(ReactorIntegration, TwoOutputsEvenSplitAndConservation) {
    streamcounter = 0;
    Reactor rx(true);                                   // два выхода
    auto in  = std::make_shared<Stream>(++streamcounter);
    auto o1  = std::make_shared<Stream>(++streamcounter);
    auto o2  = std::make_shared<Stream>(++streamcounter);

    in->setMassFlow(10.0);

    rx.addInput(in);
    rx.addOutput(o1);
    rx.addOutput(o2);

    rx.updateOutputs();

    EXPECT_NEAR(o1->getMassFlow(), 5.0, EPS);
    EXPECT_NEAR(o2->getMassFlow(), 5.0, EPS);
    EXPECT_NEAR(o1->getMassFlow() + o2->getMassFlow(), in->getMassFlow(), EPS);
}

TEST(ReactorIntegration, SingleOutputEqualsInput) {
    streamcounter = 0;
    Reactor rx(false);                                  // один выход
    auto in  = std::make_shared<Stream>(++streamcounter);
    auto o   = std::make_shared<Stream>(++streamcounter);
    in->setMassFlow(7.0);
    rx.addInput(in);
    rx.addOutput(o);
    rx.updateOutputs();
    EXPECT_NEAR(o->getMassFlow(), 7.0, EPS);
}

TEST(ReactorGuards, TooManyOutputsThrowsCharPtr) {
    streamcounter = 0;
    Reactor rx(false);                                  // разрешён 1 выход
    auto in  = std::make_shared<Stream>(++streamcounter);
    auto o1  = std::make_shared<Stream>(++streamcounter);
    auto o2  = std::make_shared<Stream>(++streamcounter);
    rx.addInput(in);
    rx.addOutput(o1);
    EXPECT_THROW(rx.addOutput(o2), const char*);        // "OUTPUT STREAM LIMIT!"
}

TEST(ReactorGuards, TooManyInputsThrowsCharPtr) {
    streamcounter = 0;
    Reactor rx(false);                                  // разрешён 1 вход
    auto in1 = std::make_shared<Stream>(++streamcounter);
    auto in2 = std::make_shared<Stream>(++streamcounter);
    rx.addInput(in1);
    EXPECT_THROW(rx.addInput(in2), const char*);        // "INPUT STREAM LIMIT!"
}

// Ветка ошибок внутри updateOutputs():
//  - без входа -> inputs.at(0) бросит std::out_of_range
//  - при двух выходах, но добавлен только один -> outputs.at(i) бросит std::out_of_range
TEST(ReactorGuards, UpdateWithoutInputThrowsOutOfRange) {
    Reactor rx(true);
    auto o1 = std::make_shared<Stream>(1);
    auto o2 = std::make_shared<Stream>(2);
    rx.addOutput(o1);
    rx.addOutput(o2);
    EXPECT_THROW(rx.updateOutputs(), std::out_of_range);
}

TEST(ReactorGuards, UpdateWithMissingSecondOutputThrowsOutOfRange) {
    Reactor rx(true);
    auto in  = std::make_shared<Stream>(1);
    auto o1  = std::make_shared<Stream>(2);
    in->setMassFlow(1.0);
    rx.addInput(in);
    rx.addOutput(o1);                // второй выход не добавляем
    EXPECT_THROW(rx.updateOutputs(), std::out_of_range);
}

// ---------- Device API via Mixer ----------
TEST(DeviceAPI, GettersReturnCopiesWithExpectedSize) {
    streamcounter = 0;
    Mixer mx(2);
    auto s1 = std::make_shared<Stream>(++streamcounter);
    auto s2 = std::make_shared<Stream>(++streamcounter);
    auto o  = std::make_shared<Stream>(++streamcounter);
    mx.addInput(s1); mx.addInput(s2); mx.addOutput(o);
    auto ins  = mx.getInputs();
    auto outs = mx.getOutputs();
    ASSERT_EQ(ins.size(), 2u);
    ASSERT_EQ(outs.size(), 1u);
    EXPECT_EQ(ins[0]->getName(), "s1");
    EXPECT_EQ(outs[0]->getName(), "s3");
}
