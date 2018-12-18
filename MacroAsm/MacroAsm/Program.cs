using System;
using System.Collections.Generic;

namespace MacroAsm
{
    class Program
    {
        static void Main(string[] args)
        {
            Macros macros = new Macros("macro", new List<string> {"a", "b"}, new List<string> {"@a + @b"});
            var res = macros.Run(new List<string> {"rv", "rb"});
            foreach (var el in res)
            {
                Console.WriteLine(el);
            }
        }
    }
}