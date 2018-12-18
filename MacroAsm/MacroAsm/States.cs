namespace MacroAsm
{
    public enum States: byte  //9  состояний
    {
        Start,                // Стартовое состояние
        WaitingOperations,    // Ожидание кода операции
        Label,                // Ввод метки
        NoColon,              // Отсутствие символа :
        CodeOperations,       // Ввод кода операции
        WaitingAruments,      // Ожидание аргументов
        Arg,                  // Ввод аргумента
        End,                  // Конечное состояние
        ErrSt                 // Состояние ошибки
    }
}