namespace MacroAsm
{
    /// <summary>
    /// Является оператором ассемблера
    /// </summary>
    public struct Operator
    {
        public int Number { get; set; }
        public string Label { get; set; }
        public string Code { get; set; }
        public string[] Arguments { get; set; }
        public string Comment { get; set; }
        public string Source { get; set; }
    }
}