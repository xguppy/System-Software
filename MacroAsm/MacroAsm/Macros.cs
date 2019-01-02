using System;
using System.Collections.Generic;

namespace MacroAsm
{
    /// <summary>
    /// Макрос
    /// </summary>
    public class Macros
    {
        private const char ParamChar = '@';
        private string[] _parames;
        private List<string> _data;
        /// <summary>
        /// Конструктор макроса
        /// </summary>
        /// <param name="name">Имя макроса</param>
        /// <param name="parames">Параметры</param>
        /// <param name="data">Тело макроса</param>
        public Macros(string[] parames, List<string> data)
        {
            _parames = parames;
            _data = data;
        }
        /// <summary>
        /// Метод исполняет макрос
        /// </summary>
        /// <param name="args">Список реальных аргументов</param>
        /// <returns>Строки ассемблера</returns>
        /// <exception cref="ArgumentException">Если не то количество</exception>
        public List<string> Run(string[] args)
        {
            if (args.Length != _parames.Length) throw new ArgumentException();
            
            var res = new List<string>();
            foreach (var element in _data)
            {
                string tmp = element;
                for (int i = 0; i < _parames.Length; i++)
                {
                    tmp = tmp.Replace(ParamChar + _parames[i], args[i]);
                }
                res.Add(tmp);
            }
            
            return res;
            
        }
    }
}