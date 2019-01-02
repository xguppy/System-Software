using System;
using System.Collections.Generic;
using IronPython.Hosting;
using Microsoft.Scripting.Hosting;

namespace MacroAsm
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length != 2) throw new ArgumentException("Неверное количество аргументов ком. строки");
            var macroAssembly = new MacroAssembly(args[0]);
            try
            {
                macroAssembly.Work(args[1]);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                throw;
            }
        }
    }
}