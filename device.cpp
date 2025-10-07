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
 * @details
 *  - Имя задаётся явным вызовом setName() либо автоматически формируется в конструкторе как "s<номер>".
 *  - Массовый расход задаётся методом setMassFlow() и считывается через getMassFlow().
 *  - Метод print() выводит краткую информацию о потоке в стандартный вывод.
 *
 * @par Пример
 * @code
 * Stream s(1);            // имя -> "s1"
 * s.setMassFlow(10.0);    // устанавливаем расход
 * s.print();              // Вывод: Stream s1 flow = 10
 * @endcode
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
     * * @post getName() == s.
     */
    void setName(string s){name=s;}

    /**
     * @brief Возвращает имя потока.
     * @return Текущее имя.
     * @note Метод возвращает копию строки.
     */
    string getName(){return name;}

    /**
     * @brief Устанавливает массовый расход потока.
     * @param m Значение массового расхода.
     * * @post getMassFlow() == m.
     */
    void setMassFlow(double m){mass_flow=m;}

    /**
     * @brief Возвращает массовый расход потока.
     * @return Текущее значение массового расхода.
     * @note Метод помечен как @c const.
     */
    double getMassFlow() const {return mass_flow;}

    /**
     * @brief Печатает краткую информацию о потоке в стандартный вывод.
     * @details Формат строки: @c "Stream <имя> flow = <значение>\\n".
     *          Используется @c cout и @c endl.
     */
    void print() { cout << "Stream " << getName() << " flow = " << getMassFlow() << endl; }
};


/**
 * @class Device
 * @brief Абстрактное устройство с набором входных и выходных потоков.
 * @details Хранит коллекции входов/выходов и ограничивает их количество.
 *          Конкретные устройства должны реализовать логика перерасчёта выходных потоков на основе входных
 *          в методе updateOutputs().
 * @note Поля @c inputAmount и @c outputAmount должны быть инициализированы
 *       (обычно в конструкторе наследника) до вызовов addInput()/addOutput().
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
     * @throw const char* Бросает строку "INPUT STREAM LIMIT!", если достигнут лимит входов.
     */
    void addInput(shared_ptr<Stream> s){
      if(inputs.size() < inputAmount) inputs.push_back(s);
      else throw"INPUT STREAM LIMIT!";
    }

    /**
     * @brief Добавляет выходной поток.
     * @param s Указатель на поток, который устройство будет наполнять как выход.
     * @throw const char* Бросает строку "OUTPUT STREAM LIMIT!", если достигнут лимит выходов.
     */
    void addOutput(shared_ptr<Stream> s){
      if(outputs.size() < outputAmount) outputs.push_back(s);
      else throw "OUTPUT STREAM LIMIT!";
    }

    /**
     * @brief Возвращает список входных потоков.
     * @return Копия вектора @c vector<shared_ptr<Stream>> с текущими входами.
     * @note Возврат по значению: вызывающий получает копию контейнера.
     */
    vector<shared_ptr<Stream>> getInputs() const { return inputs; }
    
    /**
     * @brief Возвращает список выходных потоков.
     * @return Копия вектора @c vector<shared_ptr<Stream>> с текущими выходами.
     * @note Возврат по значению: вызывающий получает копию контейнера.
     */
    vector<shared_ptr<Stream>> getOutputs() const { return outputs; }

    /**
     * @brief Пересчитывает выходные потоки на основе входных.
     * @details Виртуальная функция: конкретное устройство определяет,
     *          как преобразуются входы в выходы.
     * @remark Делает класс абстрактным; экземпляры @c Device напрямую не создаются.
     */
    virtual void updateOutputs() = 0;
};


class Mixer: public Device
{
    private:
      int _inputs_count = 0;
    public:
      Mixer(int inputs_count): Device() {
        _inputs_count = inputs_count;
      }
      void addInput(shared_ptr<Stream> s) {
        if (inputs.size() == _inputs_count) {
          throw "Too much inputs"s;
        }
        inputs.push_back(s);
      }
      void addOutput(shared_ptr<Stream> s) {
        if (outputs.size() == MIXER_OUTPUTS) {
          throw "Too much outputs"s;
        }
        outputs.push_back(s);
      }
      void updateOutputs() override {
        double sum_mass_flow = 0;
        for (const auto& input_stream : inputs) {
          sum_mass_flow += input_stream -> getMassFlow();
        }

        if (outputs.empty()) {
          throw "Should set outputs before update"s;
        }

        double output_mass = sum_mass_flow / outputs.size(); // divide 0

        for (auto& output_stream : outputs) {
          output_stream -> setMassFlow(output_mass);
        }
      }
};


class Reactor : public Device{
public:
    Reactor(bool isDoubleReactor) {
        inputAmount = 1;
        if (isDoubleReactor) outputAmount = 2;
        else inputAmount = 1;
    }
    
    void updateOutputs() override{
        double inputMass = inputs.at(0) -> getMassFlow();
            for(int i = 0; i < outputAmount; i++){
            double outputLocal = inputMass * (1/outputAmount);
            outputs.at(i) -> setMassFlow(outputLocal);
        }
    }
};


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

    if (abs(s3->getMassFlow()) - 15 < POSSIBLE_ERROR) {
      cout << "Test 1 passed"s << endl;
    } else {
      cout << "Test 1 failed"s << endl;
    }
}

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


void testTooManyOutputStreams(){
    streamcounter=0;
    
    Reactor dl = new Reactor(false);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);
    dl.addInput(s1);
    dl.addOutput(s2);
    try{
        dl.addOutput(s3);
    } catch(const string ex){
         if (ex == "OUTPUT STREAM LIMIT!")
            cout << "Test 1 passed" << endl;

        return;
    }
    
     cout << "Test 1 failed" << endl;
}

void testTooManyInputStreams(){
    streamcounter=0;
    
    Reactor dl = new Reactor(false);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);
    dl.addInput(s1);
    try{
        dl.addInput(s3);
    } catch(const string ex){
         if (ex == "INPUT STREAM LIMIT!")
            cout << "Test 2 passed" << endl;

        return;
    }
    
     cout << "Test 2 failed"s << endl;
}

void testInputEqualOutput(){
        streamcounter=0;
    
    Reactor dl = new Reactor(true);
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    s1->setMassFlow(10.0);
    s2->setMassFlow(5.0);
    dl.addInput(s1);
    dl.addOutput(s2);
    dl.addOutput(s3);
    
    dl.updateOutputs();
    
    if(dl.outputs.at(0).getMassFlow + dl.outputs.at(1).getMassFlow == dl.inputs.at(0).getMassFlow)
        cout << "Test 3 passed" << endl;
    else
        cout << "Test 3 failed" << endl;
}

void tests(){
    testInputEqualOutput();
    testTooManyOutputStreams();
    testTooManyInputStreams();
    
    shouldSetOutputsCorrectlyWithOneOutput();
    shouldCorrectOutputs();
    shouldCorrectInputs();
}

/**
 * @brief The entry point of the program.
 * @return 0 on successful execution.
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
