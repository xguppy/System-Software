using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using Microsoft.Scripting.Hosting;

namespace MacroAsm
{
    /// <summary>
    /// Макроассемблер
    /// </summary>
    public class MacroAsm
    {
        private ScriptEngine _engine;
        private ScriptScope _nameTable;
        private List<Operator> _operators;
        
        /// <summary>
        /// Конструктор макроассемблера
        /// </summary>
        /// <param name="inputFileName">Входной файл</param>
        public MacroAsm(string inputFileName)
        {
            if (!File.Exists(inputFileName)) throw new IOException();
            
            using (var fileStream = new StreamReader(inputFileName))
            {
                _operators.Add(AssemblyOperator(fileStream.ReadLine()));
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
            States curState = States.Start;
            Operator assemblyOperator = new Operator();
            StringBuilder curStrParse = new StringBuilder();
            TypeChar chType;
            assemblyOperator.Work = true;
            while(i < sz)
            {
                chType = ToTypeChar(parseString[i]);
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
                            assemblyOperator.Work = false;
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
                            assemblyOperator.Work = assemblyOperator.Label.Length != 0;
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
                    default:
                        assemblyOperator.Comment = parseString.Substring(i);
                        break;
                }
                ++i;
            }
        
            if (curState == States.CodeOperations)  assemblyOperator.Code = curStrParse.ToString();
            else if (curState == States.Arg) assemblyOperator.Arguments = curStrParse.ToString().Split(',');
        
            if (assemblyOperator.Arguments.Length != 0)    //Если есть аргументы
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
    }
}