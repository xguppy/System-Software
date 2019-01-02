using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using IronPython.Hosting;
using Microsoft.Scripting.Hosting;

namespace MacroAsm
{
    /// <summary>
    /// Макроассемблер
    /// </summary>
    public class MacroAssembly
    {
        //Макрофункция
        private delegate int MacroFunc(int number);
        //Движок интерпретатора Python 2
        private readonly ScriptEngine _engine = Python.CreateEngine();
        //Область видимости(Переменные, функции и проч.)
        private ScriptScope _nameTable;
        //Код на макроассемблере
        private readonly List<Operator> _operators = new List<Operator>();
        //Выходной код на ассемблере
        private List<string> _asmProgram = new List<string>();
        //Макрофункции
        private readonly Dictionary<string, MacroFunc> _funcs = new Dictionary<string, MacroFunc>();
        //Макросы
        private readonly Dictionary<string, Macros> _macros = new Dictionary<string, Macros>();
        //Специальный символ для foreach
        private const string forEachToken = "@val@";
        
        /// <summary>
        /// Конструктор макроассемблера
        /// </summary>
        /// <param name="inputFileName">Входной файл</param>
        public MacroAssembly(string inputFileName)
        {
            if (!File.Exists(inputFileName)) throw new IOException();
            //Опредение области видимости
            _nameTable = _engine.CreateScope();
            //Определение макрофункций
            _funcs[MacroToken.define.ToString()] = Def;
            _funcs[MacroToken.ifdef.ToString()] = IfDef;
            _funcs[MacroToken.macro.ToString()] = Macro;
            _funcs[MacroToken.@if.ToString()] = If;
            _funcs[MacroToken.@while.ToString()] = While;
            _funcs[MacroToken.ifndef.ToString()] = IfNDef;
            _funcs[MacroToken.repeat.ToString()] = Repeat;
            _funcs[MacroToken.include.ToString()] = Include;
            _funcs[MacroToken.@foreach.ToString()] = ForEach;
            _funcs[MacroToken.undef.ToString()] = UnDef;
            using (var fileStream = new StreamReader(inputFileName))
            {
                string line;
                int numberLine = 0;
                while ((line = fileStream.ReadLine()) != null)
                {
                    Operator currentOp = AssemblyOperator(line);
                    currentOp.Number = ++numberLine;
                    _operators.Add(currentOp);
                }
            }
        }
        /// <summary>
        /// Определяет тип символа
        /// </summary>
        /// <param name="ch">Входной символ</param>
        /// <returns>Тип указанного символа</returns>
        private static TypeChar ToTypeChar(char ch)
        {
            if(ch == ':') return TypeChar.Colon;
            if(ch == ';') return TypeChar.Comment;
            if(ch == '_') return TypeChar.Underline;
            if(Char.IsDigit(ch)) return TypeChar.Digit;
            if(Char.IsWhiteSpace(ch)) return TypeChar.Blank;
            if(Char.IsLetter(ch)) return TypeChar.Id;
            return TypeChar.Arguments;
        }
        /// <summary>
        /// Метод собирает оператор ассемблера
        /// </summary>
        /// <param name="parseString">Входная строка</param>
        /// <returns>Выходной оператор</returns>
        private static Operator AssemblyOperator(string parseString)
        {
            //Будем использовать перечисления и матрицу состояний
            States[,] statesMatrix =
            {
                //В начале у нас состояние Start
                {States.WaitingOperations, States.ErrSt, States.End, States.ErrSt, States.Label, States.Label, States.ErrSt},
                //Пробел->ждём,Двоеточие->ошибка,Комментарий->конец,Цифра->ошибка,Идентификатор->метка,Подчёркивание->метка,Любой->ошибка
                //Ждём операции
                {States.WaitingOperations, States.ErrSt, States.End, States.ErrSt, States.CodeOperations, States.CodeOperations, States.ErrSt},
                //Пробел->ждём,Двоеточие->ошибка,Комментарий->конец,Цифра->ошибка,Идентификатор->метка,Подчёркивание->метка,Любой->ошибка
                //Метка
                {States.NoColon, States.WaitingOperations, States.End, States.Label, States.Label, States.Label, States.ErrSt},
                //Пробел->Нет :,Двоеточие->Ждём операторов,Комментарий->конец,Цифра->метка,Идентификатор->метка,Подчёркивание->метка,Любой->ошибка
                //Нет ":"
                {States.ErrSt, States.ErrSt, States.ErrSt, States.ErrSt, States.ErrSt, States.ErrSt, States.ErrSt},
                //Из ошибочного состояния отсутсвия ':' попадём только в ошибку
                //Код операции
                {States.WaitingAruments, States.ErrSt, States.End, States.ErrSt, States.CodeOperations, States.ErrSt, States.ErrSt},
                //Пробел->ждём аргументы, Двоеточие->ошибка, Комментарий->конец, Цифра->ошибка, Идентификатор->код операции, Подчёркивание->ошибка , Любой->ошибка
                //Ожидание аргументов
                {States.WaitingAruments, States.ErrSt, States.End,  States.Arg, States.Arg, States.Arg, States.Arg},
                //Пробел->ждём, Двоеточие->ошибка, Комментарий->конец, Цифра->аргумент, Идентификатор->аргумент, Подчёркивание->аргумент , Любой->аргумент
                //Ввод аргументов
                {States.Arg, States.ErrSt, States.End,  States.Arg, States.Arg, States.Arg, States.Arg},
                //Пробел->ждём, Двоеточие->ошибка, Комментарий->конец, Цифра->аргумент, Идентификатор->аргумент, Подчёркивание->аргумент , Любой->аргумент
                //Конечное состояние
                {States.End, States.End, States.End, States.End, States.End, States.End, States.End},
                //Пробел->конец, Двоеточие->конец, Комментарий->конец, Цифра->конец, Идентификатор->конец, Подчёркивание->конец , Любой->конец
                //Состояние ошибки
                {States.End, States.End, States.End, States.End, States.End, States.End, States.End}
                //Конец(для наполнения, не обязательно)
            };

            int i = 0, sz = parseString.Length;
            var curState = States.Start;
            var assemblyOperator = new Operator();
            var curStrParse = new StringBuilder();
            //assemblyOperator.Work = true;
            assemblyOperator.Source = parseString;
            while(i < sz)
            {
                var chType = ToTypeChar(parseString[i]);
                States prev = curState;
                curState = statesMatrix[(int)curState, (int)chType];
                switch(prev)
                {
                    case States.Start:
                        if(curState == States.Label)
                        {
                            curStrParse.Append(parseString[i]);
                        }
                        else if(curState == States.End)
                        {
                            assemblyOperator.Comment = parseString;
                            //assemblyOperator.Work = false;
                        }
                        break;
                    case States.WaitingOperations:
                        if(curState == States.CodeOperations)
                        {
                            curStrParse.Append(parseString[i]);
                        }
                        else if(curState == States.End)
                        {
                            assemblyOperator.Comment = curStrParse.ToString();
                            //assemblyOperator.Work = assemblyOperator.Label.Length != 0;
                        }
                        break;
                    case States.Label:
                        if(curState == States.Label)
                        {
                            curStrParse.Append(parseString[i]);
                        }
                        else if(curState == States.WaitingOperations || curState == States.NoColon)
                        {
                            assemblyOperator.Label  = curStrParse.ToString();
                            curStrParse.Clear();
                        }
                        break;
                    case States.CodeOperations:
                        if(curState == States.CodeOperations)
                        {
                            curStrParse.Append(parseString[i]);
                        }
                        else if(curState == States.WaitingAruments)
                        {
                            assemblyOperator.Code = curStrParse.ToString();
                            curStrParse.Clear();
                        }
                        else if(curState == States.End)
                        {
                            assemblyOperator.Code = curStrParse.ToString();
                            assemblyOperator.Comment = parseString.Substring(i);
                            curStrParse.Clear();
                        }
                        else if(curState == States.Arg)
                        {
                            curStrParse.Append(parseString[i]);
                        }
                        break;
                    case States.WaitingAruments:
                        if(curState == States.End)
                        {
                            assemblyOperator.Comment = parseString.Substring(i);
                        }
                        else if(curState == States.Arg)
                        {
                            curStrParse.Clear();
                            curStrParse.Append(parseString[i]);
                        }
                        break;
                    case States.Arg:
                        if(curState == States.End)
                        {
                            assemblyOperator.Arguments = curStrParse.ToString().Split(',');
                            curStrParse.Clear();
                            assemblyOperator.Comment = parseString.Substring(i);
                        }
                        else if(curState == States.Arg)
                        {
                            curStrParse.Append(parseString[i]);
                        }
                        break;
                    case States.End:
                        return assemblyOperator;
                }
                ++i;
            }
        
            if (curState == States.CodeOperations)  assemblyOperator.Code = curStrParse.ToString();
            else if (curState == States.Arg) assemblyOperator.Arguments = curStrParse.ToString().Split(',');
        
            if (assemblyOperator.Arguments != null)    //Если есть аргументы
            {
                if (assemblyOperator.Code != "str")
                {
                    for (int j = 0; j < assemblyOperator.Arguments.Length; j++)
                    {
                        assemblyOperator.Arguments[j] = assemblyOperator.Arguments[j].Replace(" ", String.Empty);
                    }
                }
                else
                {
                    for (int j = 0; j < assemblyOperator.Arguments.Length; j++)
                    {
                        assemblyOperator.Arguments[j] = assemblyOperator.Arguments[j].Trim();
                    }
                }
            }
            return assemblyOperator;
        }
        /// <summary>
        /// Основной цикл работы макроассемблера
        /// </summary>
        /// <param name="outputFileName">Выходной файл макроассемблера</param>
        public void Work(string outputFileName)
        {
            ExecuteLines(0);
            using (var writeStream = new StreamWriter(outputFileName))
            {
                foreach (var line in _asmProgram)
                {
                    if (line.Trim().Length != 0)
                    {
                        writeStream.WriteLine(line);
                    }
                }
            }
        }
        
        private int ExecuteLines(int number)
        {
            int retPos = number;
            while (retPos < _operators.Count)
            {
                retPos = Exec(retPos);
            }
            return retPos;
        }

        private int Exec(int number)
        {
            int returnPos = number + 1;
            if (_operators[number].Code != null && _funcs.ContainsKey(_operators[number].Code))
            {
                returnPos = _funcs[_operators[number].Code].Invoke(number);
            }
            else if (_operators[number].Code != null && MacroToken.TryParse(_operators[number].Code, false, out MacroToken typeLex))
            {
                throw new Exception($"Нет объявленной лексемы для {typeLex}");
            }
            else
            {
                if (_operators[number].Code != null && _macros.ContainsKey(_operators[number].Code))
                {
                    var data = _macros[_operators[number].Code].Run(_operators[number].Arguments);
                    foreach (var elem in data)
                    {
                        _asmProgram.Add(elem);
                    }
                }
                else
                {
                    _asmProgram.Add(Replacement(_operators[number].Source));
                }
            }
            return returnPos;
        }
        /// <summary>
        /// Применяет макроподстановки
        /// </summary>
        /// <param name="str">Исходная строка</param>
        /// <returns>Строка с подставленными макропеременными</returns>
        private string Replacement(string str)
        {
            foreach (var name in _nameTable.GetItems())
            {
                if(name.Value != null)
                {
                    str = str.Replace($"##{name.Key}", $"|{name.Value}|");
                    str = str.Replace($"#{name.Key}", name.Value.ToString());
                }
            }
            return str;
        }

        ////Функции макроассемблера
        private int Def(int number)
        {
            if (_operators[number].Arguments.Length != 1) throw new ArgumentException($"Отсутсвуют аргументы для конструкции define на строке {_operators[number].Number}");
            _nameTable.SetVariable(_operators[number].Label, _engine.Execute(_operators[number].Arguments[0], _nameTable));
            return ++number;
        }
        
        //Удаление переменной
        private int UnDef(int number)
        {
            if (_operators[number].Arguments.Length != 1) throw new ArgumentException($"Отсутсвуют аргументы для конструкции undef  на строке {_operators[number].Number}");
            if (!_nameTable.RemoveVariable(_operators[number].Arguments[0]))
            {
                throw new ArgumentNullException();
            }
            return ++number;
        }

        private int IfDef(int number)
        {
            if(_operators[number].Arguments.Length > 1) throw new ArgumentException($"Отсутсвуют аргументы для конструкции ifdef  на строке {_operators[number].Number}");
            bool existElseToken = false;    //был ли найден токен "else"
            bool existEndToken = false;    //был ли найден токен "end"
            int lineOp = _operators[number].Number;
            if (_nameTable.ContainsVariable(_operators[number].Arguments[0]))
            {
                while (number < _operators.Count && !existEndToken)
                {
                    ++number;
                    
                    //Если встретился токен "else" то меняем логику
                    if (_operators[number].Code == MacroToken.@else.ToString())
                    {
                        existElseToken = true;
                    }
                    else if (_operators[number].Code == MacroToken.endif.ToString())
                    {
                        existEndToken = true;
                    }
                    
                    //Есди не встретился токен "else" то исполняем
                    if (existElseToken == false && existEndToken == false)
                    {
                        //Исполняем
                        Exec(number);
                    }
                }
            }
            else
            {
                while (number < _operators.Count && !existEndToken)
                {
                    ++number;
                    //Если встретился токен "else" то меняем логику
                    if (_operators[number].Code == MacroToken.@else.ToString())
                    {
                        existElseToken = true;
                        ++number;
                    }
                    else if (_operators[number].Code == MacroToken.endif.ToString())
                    {
                        existEndToken = true;
                    }
                    //Есди встретился токен "else" то исполняем
                    if (existElseToken && existEndToken == false)
                    {
                        //Исполняем
                        Exec(number);
                    }
                }
            }
            if (existEndToken != true)
            {
                throw new Exception($"Нет директивы endif для конструкции на строке {lineOp}");
            }
            return ++number;
        }

        private int IfNDef(int number)
        {
            if (_operators[number].Arguments.Length > 1) throw new ArgumentException($"Отсутсвуют аргументы для конструкции ifndef на строке {_operators[number].Number}");
            bool existElseToken = false; //был ли найден токен "else"
            bool existEndToken = false; //был ли найден токен "end"
            int lineOp = _operators[number].Number;
            if (_nameTable.ContainsVariable(_operators[number].Arguments[0]))
            {
                while (number < _operators.Count && !existEndToken)
                {
                    ++number;
                    //Если встретился токен "else" то меняем логику
                    if (_operators[number].Code == MacroToken.@else.ToString())
                    {
                        existElseToken = true;
                    }
                    else if (_operators[number].Code == MacroToken.endif.ToString())
                    {
                        existEndToken = true;
                    }
                    
                    //Есди встретился токен "else" то исполняем
                    if (existElseToken == false && existEndToken == false)
                    {
                        //Исполняем
                        Exec(number);
                    }
                }
            }
            else
            {
                while (number < _operators.Count && !existEndToken)
                {
                    ++number;
                    //Если встретился токен "else" то меняем логику
                    if (_operators[number].Code == MacroToken.@else.ToString())
                    {
                        existElseToken = true;
                        ++number;
                    }
                    else if (_operators[number].Code == MacroToken.endif.ToString())
                    {
                        existEndToken = true;
                    }
                    //Есди не встретился токен "else" то исполняем
                    if (existElseToken && existEndToken == false)
                    {
                        //Исполняем
                        Exec(number);
                    }

                    
                }
            }
            if (existEndToken != true)
            {
                throw new Exception($"Нет директивы endif для конструкции на строке {lineOp}");
            }

        return ++number;
        }

        private int Macro(int number)
        {
            string nameMacros = _operators[number].Label;
            var paramsMacro = _operators[number].Arguments;
            var dataMacros = new List<string>();
            bool existEndToken = false;
            int lineOp = _operators[number].Number;
            ++number;
            while (number < _operators.Count && !existEndToken)
            {
                if (_operators[number].Code == MacroToken.endmacro.ToString())
                {
                    existEndToken = true;
                }
                else
                {
                    dataMacros.Add(_operators[number].Source);
                }
                ++number;
            }
            if (existEndToken != true)
            {
                throw new Exception($"Нет директивы endmacro для конструкции на строке {lineOp}");
            }
            _macros[nameMacros] = new Macros(paramsMacro, dataMacros);
            return number;
        }

        private int If(int number)
        {
            if (_operators[number].Arguments.Length > 1) throw new ArgumentException($"Отсутсвуют аргументы для конструкции if на строке {_operators[number].Number}");
            bool existElseToken = false; //был ли найден токен "else"
            bool existEndToken = false; //был ли найден токен "end"
            int lineOp = _operators[number].Number;
            if (_engine.Execute<bool>(_operators[number].Arguments[0], _nameTable))
            {
                while (number < _operators.Count && !existEndToken)
                {
                    ++number;
                    //Если встретился токен "else" то меняем логику
                    if (_operators[number].Code == MacroToken.@else.ToString())
                    {
                        existElseToken = true;
                    }
                    else if (_operators[number].Code == MacroToken.endif.ToString())
                    {
                        existEndToken = true;
                    }
                    //Есди встретился токен "else" то исполняем
                    if (existElseToken == false && existEndToken == false)
                    {
                        //Исполняем
                        Exec(number);
                    }
                }
            }
            else
            {
                while (number < _operators.Count && !existEndToken)
                {
                    ++number;
                    //Если встретился токен "else" то меняем логику
                    if (_operators[number].Code == MacroToken.@else.ToString())
                    {
                        existElseToken = true;
                        ++number;
                    }
                    else if (_operators[number].Code == MacroToken.endif.ToString())
                    {
                        existEndToken = true;
                    }
                    //Есди встретился токен "else" то исполняем
                    if (existElseToken && existEndToken == false)
                    {
                        //Исполняем
                        Exec(number);
                    }
                }
            }

            if (existEndToken != true)
            {
                throw new Exception($"Нет директивы endif для конструкции на строке {lineOp}");
            }

            return ++number;
        }

        private int Repeat(int number)
        {
            if (_operators[number].Arguments.Length > 1) throw new ArgumentException($"Отсутсвуют аргументы для конструкции repeat на строке {_operators[number].Number}");
            var repeateCount =_engine.Execute<int>(_operators[number].Arguments[0], _nameTable);
            int startNumber = number;
            int endNumber = number;
            int lineOp = _operators[number].Number;
            while (repeateCount > 0)
            {
                bool existEndToken = false; //был ли найден токен "end"
                while (number < _operators.Count && !existEndToken)
                {
                    ++number;
                    if (_operators[number].Code == MacroToken.endrepeat.ToString())
                    {
                        existEndToken = true;
                        endNumber = number;
                    }

                    if (existEndToken == false)
                    {
                        Exec(number);    
                    }
                }
                --repeateCount;
                number = startNumber;
                if (existEndToken != true)
                {
                    throw new Exception($"Нет директивы endrepeate для конструкции на строке {lineOp}");
                }
            }
            return ++endNumber;
        }

        private int While(int number)
        {
            if (_operators[number].Arguments.Length > 1) throw new ArgumentException($"Отсутсвуют аргументы для конструкции while на строке {_operators[number].Number}");
            int startNumber = number;
            int endNumber = number;
            int lineOp = _operators[number].Number;
            while (_engine.Execute<bool>(_operators[number].Arguments[0], _nameTable))
            {
                bool existEndToken = false; //был ли найден токен "end"
                while (number < _operators.Count && !existEndToken)
                {
                    ++number;
                    if (_operators[number].Code == MacroToken.endwhile.ToString())
                    {
                        existEndToken = true;
                        endNumber = number;
                    }
                    if (existEndToken == false)
                    {
                        Exec(number);
                    }
                }
                number = startNumber;
                if (existEndToken != true)
                {
                    throw new Exception($"Нет директивы endwhile для конструкции на строке {lineOp}");
                }
            }
            return ++endNumber;
        }
        
        private int ForEach(int number)
        {
            if (_operators[number].Arguments.Length == 0) throw new ArgumentException($"Отсутсвуют аргументы для конструкции foreach на строке {_operators[number].Number}");
            var op = _operators[number];
            int startNumber = number;
            int endNumber = number;
            int lineOp = _operators[number].Number;
            foreach (var arg in op.Arguments)
            {
                bool existEndToken = false; //был ли найден токен "end"
                while (number < _operators.Count && !existEndToken)
                {
                    ++number;
                    
                    if (_operators[number].Code == MacroToken.endeach.ToString())
                    {
                        existEndToken = true;
                        endNumber = number;
                    }

                    if (existEndToken == false)
                    {
                        string strTmp = _operators[number].Source.Replace(forEachToken, arg);
                        _asmProgram.Add(strTmp);    
                        
                    }
                    
                }
                number = startNumber;
                if (existEndToken != true)
                {
                    throw new Exception($"Нет директивы endeach для конструкции на строке {lineOp}");
                }
            }
            return ++endNumber;
        }
        
        private int Include(int number)
        {
            if(_operators[number].Arguments.Length != 1) throw new ArgumentException($"Не указан файл для включения на строке {_operators[number].Number}");
            MacroAssembly macroAssembly = new MacroAssembly(_operators[number].Arguments[0]);
            _operators.InsertRange(++number, macroAssembly._operators);
            return number;
        }
    }
}