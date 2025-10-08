#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

using namespace std;

int streamcounter; ///< Global variable to keep track of stream creation.

const int MIXER_OUTPUTS = 1;
const float POSSIBLE_ERROR = 0.01;


/**
 * @class Stream
 * @brief Представляет материальный поток с именем и массовым расходом.
 */
class Stream
{
private:
    double mass_flow; ///< Массовый расход потока.
    string name;      ///< Имя потока. 

public:
    /**
     * @brief Конструктор, создающий поток с именем вида "s<номер>".
     * @param s Порядковый номер потока; используется для формирования имени.
     */
    Stream(int s){setName("s"+std::to_string(s));}

    /**
     * @brief Устанавливает имя потока.
     * @param s Новое имя потока.
     */
    void setName(string s){name=s;}

    /**
     * @brief Возвращает имя потока.
     * @return Текущее имя.
     */
    string getName(){return name;}

    /**
     * @brief Устанавливает массовый расход потока.
     * @param m Значение массового расхода.
     */
    void setMassFlow(double m){mass_flow=m;}

    /**
     * @brief Возвращает массовый расход потока.
     * @return Текущее значение массового расхода.
     */
    double getMassFlow() const {return mass_flow;}

    /**
     * @brief Печатает краткую информацию о потоке в стандартный вывод.
     */
    void print() { cout << "Stream " << getName() << " flow = " << getMassFlow() << endl; }
};


/**
 * @class Device
 * @brief Абстрактное устройство с набором входных и выходных потоков.
 */
class Device
{
protected:
    vector<shared_ptr<Stream>> inputs;  ///< Входные потоки, подключённые к устройству.
    vector<shared_ptr<Stream>> outputs; ///< Выходные потоки, формируемые устройством.
    int inputAmount; ///< Максимально допустимое количество входных потоков.
    int outputAmount; ///< Максимально допустимое количество выходных потоков.

public:
    /**
     * @brief Добавляет входной поток.
     * @param s Указатель на поток, который нужно подключить ко входу.
     */
    void addInput(shared_ptr<Stream> s){
      if(inputs.size() < inputAmount) inputs.push_back(s);
      else throw"INPUT STREAM LIMIT!";
    }

    /**
     * @brief Добавляет выходной поток.
     * @param s Указатель на поток, который устройство будет наполнять как выход.
     */
    void addOutput(shared_ptr<Stream> s){
      if(outputs.size() < outputAmount) outputs.push_back(s);
      else throw "OUTPUT STREAM LIMIT!";
    }

    /**
     * @brief Возвращает список входных потоков.
     * @return Копия вектора @c vector<shared_ptr<Stream>> с текущими входами.
     */
    vector<shared_ptr<Stream>> getInputs() const { return inputs; }
    
    /**
     * @brief Возвращает список выходных потоков.
     * @return Копия вектора @c vector<shared_ptr<Stream>> с текущими выходами.
     */
    vector<shared_ptr<Stream>> getOutputs() const { return outputs; }

    /**
     * @brief Пересчитывает выходные потоки на основе входных.
     */
    virtual void updateOutputs() = 0;
};


/**
 * @class Mixer
 * @brief Устройство-миксер: суммирует расходы входных потоков и распределяет по выходам.
 */
class Mixer: public Device
{
private:
    int _inputs_count = 0; ///< Разрешённое количество входных потоков, задаётся в конструкторе.

public:
    /**
     * @brief Создаёт миксер с заданным количеством входов.
     * @param inputs_count Максимально допустимое число входных потоков.
     */
    Mixer(int inputs_count): Device() {
        _inputs_count = inputs_count;
    }

    /**
     * @brief Добавляет входной поток.
     * @param s Умный указатель на поток @ref Stream для подключения ко входу.
     */
    void addInput(shared_ptr<Stream> s) {
        if (inputs.size() == _inputs_count) {
            throw "Too much inputs"s;
        }
        inputs.push_back(s);
    }

    /**
     * @brief Добавляет выходной поток.
     * @param s Умный указатель на поток @ref Stream, который будет заполнен на выходе.
     */
    void addOutput(shared_ptr<Stream> s) {
        if (outputs.size() == MIXER_OUTPUTS) {
            throw "Too much outputs"s;
        }
        outputs.push_back(s);
    }

    
    /**
     * @brief Пересчитывает выходные потоки на основе входных.
     */
    void updateOutputs() override {
        double sum_mass_flow = 0;
        for (const auto& input_stream : inputs) {
            sum_mass_flow += input_stream->getMassFlow();
        }

        if (outputs.empty()) {
            throw "Should set outputs before update"s;
        }

        double output_mass = sum_mass_flow / outputs.size();

        for (auto& output_stream : outputs) {
            output_stream->setMassFlow(output_mass);
        }
    }
};


/**
 * @class Reactor
 * @brief Реактор с одним входом и 1–2 выходами, равномерно распределяющий расход.
 */
class Reactor : public Device{
public:
    /**
     * @brief Конструктор: настраивает число выходов (1 или 2) при единственном входе.
     * @param isDoubleReactor Если @c true — два выхода, иначе один.
     */
    Reactor(bool isDoubleReactor) {
        inputAmount = 1;
        if (isDoubleReactor) outputAmount = 2;
        else outputAmount = 1;
    }
    
    /**
     * @brief Перерасчёт выходных потоков из входного.
     */
    void updateOutputs() override{
        double inputMass = inputs.at(0) -> getMassFlow();
            for(int i = 0; i < outputAmount; i++){
            double outputLocal = inputMass * (1.0/outputAmount);
            outputs.at(i) -> setMassFlow(outputLocal);
        }
    }
};

#ifndef UNIT_TESTS
/**
 * @test
 * @brief Проверяет, что Mixer с одним выходом устанавливает суммарный расход входов на выход.
 */
void shouldSetOutputsCorrectlyWithOneOutput() {
    streamcounter=0;
    Mixer d1 = Mixer(2);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(s3);

    d1.updateOutputs();

    if (abs(s3->getMassFlow() - 15) < POSSIBLE_ERROR) {
      cout << "Mixer Test 1 passed"s << endl;
    } else {
      cout << "Mixer Test 1 failed"s << endl;
    }
}

/**
 * @test
 * @brief Проверяет, что Mixer не допускает больше выходов, чем разрешено.
 */
void shouldCorrectOutputs() {
    streamcounter=0;
    Mixer d1 = Mixer(2);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    shared_ptr<Stream> s4(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(s3);

    try {
      d1.addOutput(s4);
    } catch (const string ex) {
      if (ex == "Too much outputs"s) {
        cout << "Test 2 passed"s << endl;

        return;
      }
    }

    cout << "Test 2 failed"s << endl;
}

/**
 * @test
 * @brief Проверяет, что Mixer запрещает добавлять больше входов, чем разрешено.
 */
void shouldCorrectInputs() {
    streamcounter=0;
    Mixer d1 = Mixer(2);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    shared_ptr<Stream> s4(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    d1.addInput(s1);
    d1.addInput(s2);
    d1.addOutput(s3);

    try {
      d1.addInput(s4);
    } catch (const string ex) {
      if (ex == "Too much inputs"s) {
        cout << "Test 3 passed"s << endl;

        return;
      }
    }

    cout << "Test 3 failed"s << endl;
}


/**
 * @test
 * @brief Проверяет, что Reactor не допускает добавление второго выходного потока при 1 разрешённом выходе.
 */
void testTooManyOutputStreams(){
    streamcounter=0;
    
    Reactor dl(false);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);
    dl.addInput(s1);
    dl.addOutput(s2);
    try{
        dl.addOutput(s3);
    } catch (const char* ex) {
         if (string(ex)  == "OUTPUT STREAM LIMIT!")
            cout << "Reactor Test 1 passed" << endl;

        return;
    }
    
     cout << "Reactor Test 1 failed" << endl;
}


/**
 * @test
 * @brief Проверяет, что Reactor запрещает добавление второго входного потока при лимите в один вход.
 */
void testTooManyInputStreams(){
    streamcounter = 0;
    
    Reactor dl(false);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    s1->setMassFlow(10.0);

    dl.addInput(s1);
    
    try {
        dl.addInput(s2); 
    } catch (const char* ex) { 
        if (string(ex) == "INPUT STREAM LIMIT!") {
            cout << "Reactor Test 2 passed" << endl;
            return;
        }
    }
    
    cout << "Reactor Test 2 failed" << endl;
}


/**
 * @test
 * @brief Проверяет закон сохранения потока для Reactor с двумя выходами.
 */
void testInputEqualOutput(){
        streamcounter=0;
    
    Reactor dl(true);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);
    dl.addInput(s1);
    dl.addOutput(s2);
    dl.addOutput(s3);
    
    dl.updateOutputs();
    
    const auto outs = dl.getOutputs();
    const auto ins  = dl.getInputs();

    const double sum_out = outs.at(0)->getMassFlow() + outs.at(1)->getMassFlow();
    const double in      = ins.at(0)->getMassFlow();

    if (std::abs(sum_out - in) < POSSIBLE_ERROR)
        std::cout << "Reactor Test 3 passed\n";
    else
        std::cout << "Reactor Test 3 failed\n";
}

/**
 * @test
 * @brief Запускает набор тестов для Mixer и Reactor.
 */
void tests(){
    testInputEqualOutput();
    testTooManyOutputStreams();
    testTooManyInputStreams();
    
    shouldSetOutputsCorrectlyWithOneOutput();
    shouldCorrectOutputs();
    shouldCorrectInputs();
}

/**
 * @brief Точка входа в программу.
 * @return 0 при успешном завершении.
 */
int main()
{
    streamcounter = 0;

    // Create streams
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));

    // Set mass flows
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);

    // Create a device (e.g., Mixer) and add input/output streams
    // Mixer d1;
    // d1.addInput(s1);
    // d1.addInput(s2);
    // d1.addOutput(s3);

    // Update the outputs of the device
    // d1.updateOutputs();

    // Print stream information
//    s1->print();
//    s2->print();
//    s3->print();
    tests();

    return 0;
}
#endif // UNIT_TESTS

