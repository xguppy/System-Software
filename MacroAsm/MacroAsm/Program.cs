using System;

namespace MacroAsm
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length != 2)
            {
                Console.WriteLine($"Неверное количество аргументов ком. строки вы ввели {args.Length} , а требуется 2");
                return;
            }
            try
            {
                var macroAssembly = new MacroAssembly(args[0]);
                macroAssembly.Work(args[1]);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
        }
    }
}