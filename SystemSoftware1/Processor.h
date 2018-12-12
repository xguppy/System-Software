#ifndef SYSTEMSOFTWARE1_PROCESSOR_H
#define SYSTEMSOFTWARE1_PROCESSOR_H


#include <cstdint>
#include <string>
#include <stack>
#include "Commands.h"


class Processor final {
private:
    //Количество ячеек памяти
    static const uint16_t N = 65535u;
    //Регистры
    Registers regs_{};
    // Комманды
    std::vector<Command*> commands_;
    //Перезагрузка ПК
    void Reset() noexcept;

    //Инициализация
    void InitCmd() noexcept;

    //Конструктор по умолчанию будет в private
    Processor() = default;
public:
    //Удаляем конструктор копирования
    Processor(const Processor &root) = delete;

    //Память открыта
    Memory memory_;

    //Удаляем копирование присваиванием
    Processor &operator=(const Processor &) = delete;

    //Деструктор
    virtual ~Processor() noexcept;

    //Запустить VM
    void Run() noexcept;

    //Задать IP
    void SetIp(address ip) noexcept;

    //Трассировка
    void Trace() const noexcept;

    //Получить данные из памяти
    static command GetCommand(const Memory &mem, address i) noexcept;

    //Создание единственного объекта класса с флагом трассировки
    static Processor &Instance(uint8_t debuging) noexcept;
};


#endif //SYSTEMSOFTWARE1_PROCESSOR_H
