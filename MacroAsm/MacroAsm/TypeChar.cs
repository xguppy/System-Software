namespace MacroAsm
{
    public enum TypeChar: byte    //7 сигналов
    {
        Blank,                    // Пробелы
        Colon,                    // Двоеточия
        Comment,                  // Комментарии
        Digit,                    // Цифры
        Id,                       // Идентификаторы
        Underline,                // Подчеркивания
        Arguments                 // Аргументы
    }
}