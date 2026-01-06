// 3.Jan.2026
using System;

namespace adaptiveRF
{
    internal class Program
    {
        static void Main(string[] args)
        {
            int batch = 4;
            string input_h = "";
            string input_p = "";
            try
            {
                if (args.Length == 0)
                {
                    Console.WriteLine("File and batch size missing!");
                    Console.WriteLine("Usage: adaptiveRF <filename> <batchsize>");
                    return;
                }
                else if (args.Length == 2)
                    batch = Convert.ToInt32(args[1]);
                input_h = args[0];
                input_p = args[0].Split('.')[0] + ".p";
            }
            catch (Exception ex1)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Error arguments: " + ex1.Message);
                Console.ForegroundColor = ConsoleColor.Gray;
                Environment.Exit(1);
            }
            Console.WriteLine("\nBatch size: " + batch);

            string[] lines = null;
            string[] plines = null;
            try
            {
                Console.Write("Loading " + input_h + " ...");
                lines = File.ReadAllLines(input_h);
                Console.Write("done\nLoading " + input_p + " ...");
                plines = File.ReadAllLines(input_p);
                Console.WriteLine("done");
            }
            catch (Exception ex2)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("Error reading files: " + ex2.Message);
                Console.ForegroundColor = ConsoleColor.Gray;
                Environment.Exit(2);
            }

            uint n_classes = 1;
            foreach (string l1 in lines)
            {
                if (l1.Contains("votes"))
                {
                    n_classes = Convert.ToUInt32(l1.Substring(l1.IndexOf('[') + 1, 1));
                    break;
                }
            }

            int n = 0, n_tree = 0;
            bool newBlock = true;
            List<List<List<string>>> p = new List<List<List<string>>>();
            List<List<string>> tree = null;
            List<int> leaves = null;
            string pl_temp = "";
            foreach (string pl in plines)
            {
                if (newBlock)
                {
                    pl_temp += pl;
                    if (pl_temp.Contains(']'))
                    {
                        leaves = ExtractIntegerValues(pl_temp);
                        n_tree++;
                        ToggleTextcolor(n_tree);
                        Console.WriteLine("Decision Tree: " + String.Format("{0,3}", n_tree) + ", Leaves: " + String.Join(", ", leaves.ToArray()));
                        tree = new List<List<string>>();
                        newBlock = false;
                    }
                }
                else
                {
                    List<string> row = ExtractStringValues(pl);
                    tree.Add(row);
                    n++;
                    if (n == leaves.Count)
                    {
                        p.Add(tree);
                        n = 0;
                        newBlock = true;
                        pl_temp = "";
                    }
                }
            }

            int b = -1, v = 0;
            int tree_idx = 0;
            int row_idx = 0;
            bool end = false;
            string data = "";
            string output = "adaptive_batchsize" + batch + "_" + input_h;
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.Write("Writing " + output + "...");
            using (StreamWriter sw = new StreamWriter(output))
            {
                foreach (string l2 in lines)
                {
                    if (l2.Contains("cstdarg"))
                    {
                        sw.Write("// ");
                    }
                    else if (l2.Contains("public"))
                    {
                        sw.WriteLine(l2);
                        sw.WriteLine("#define KLASSEN " + n_classes);
                        foreach (string s1 in snippet1)
                            sw.WriteLine(s1);
                        continue;
                    }
                    else if (l2.Contains("predict"))
                    {
                        sw.WriteLine(l2);
                        foreach (string s2 in snippet2)
                            sw.WriteLine(s2);
                        continue;
                    }
                    else if (l2.Contains("tree"))
                    {
                        b++;
                        if ((b > 0) && (b == batch))
                        {
                            foreach (string s3 in snippet3)
                                sw.WriteLine(s3);
                            b = 0;
                        }
                    }
                    else if ((!end) && l2.Contains("votes"))
                    {
                        if (v > 0)
                        {
                            for (int i = 0; i < n_classes; i++)
                            {
                                data += "p[" + i + "] += " + p[tree_idx][row_idx][i];
                                if (i < n_classes - 1)
                                    data += ", ";
                            }
                            data += ";";
                            sw.WriteLine(data);
                            data = "";
                            row_idx++;
                            if (row_idx == p[tree_idx].Count)
                            {
                                row_idx = 0;
                                tree_idx++;
                                if (tree_idx == p.Count)
                                    end = true;
                            }
                        }
                        else
                            v++;
                    }
                    sw.WriteLine(l2);
                }
            }
            Console.WriteLine("done");
        }
        static List<string> ExtractStringValues(string input)
        {
            string[] s = input.Split(']')[0].Split('[')[1].Split(' ', StringSplitOptions.RemoveEmptyEntries);
            if (s[0].Contains('.'))
            {
                for (int i = 0; i < s.Length; i++)
                {
                    if (s[i].Split('.')[1] == String.Empty)
                        s[i] += "0";
                }
            }
            return s.ToList();
        }

        static List<int> ExtractIntegerValues(string input)
        {
            string[] s = input.Split(']')[0].Split('[')[1].Split(' ', StringSplitOptions.RemoveEmptyEntries);
            List<int> ints = new List<int>();
            for (int i = 0; i < s.Length; i++)
                ints.Add(Convert.ToInt32(s[i]));
            return ints;
        }

        static void ToggleTextcolor(int n)
        {
            if (n % 2 == 0)
                Console.ForegroundColor = ConsoleColor.Cyan;
            else
                Console.ForegroundColor = ConsoleColor.Yellow;
        }

        static string[] snippet1 = new string[]
        {
            "float p[KLASSEN];",
            "int result = 0;",
            "int ScoreMargin()",
            "{",
            "\tfloat max_1st = 0.0;",
            "\tfloat max_2nd = 0.0;",
            "\tint idx_1st = 0;",
            "\tint idx_2nd = 0;",
            "\tfor (uint8_t i = 0; i < KLASSEN; i++)",
            "\t{",
            "\t\tif (p[i] > max_1st)",
            "\t\t\t{",
            "\t\t\tmax_2nd = max_1st;",
            "\t\t\tidx_2nd = idx_1st;",
            "\t\t\tmax_1st = p[i];",
            "\t\t\tidx_1st = i;",
            "\t\t}",
            "\t\telse if (p[i] > max_2nd)",
            "\t\t{",
            "\t\t\tmax_2nd = p[i];",
            "\t\t\tidx_2nd = i;",
            "\t\t}",
            "\t}",
            "\tif ((max_1st - max_2nd) > ALPHA)",
            "\t{",
            "\t\tresult = idx_1st;",
            "\t\treturn 1;",
            "\t}",
            "\telse",
            "\t\treturn 0;",
            "}",
        };

        static string[] snippet2 = new string[]
        {
            "result = 0;",
            "for (int i = 0; i < KLASSEN; i++)",
            "\tp[i] = 0.0;"
        };

        static string[] snippet3 = new string[]
        {
            "if (ScoreMargin() == 1)",
            "\treturn result;"
        };
    }
}