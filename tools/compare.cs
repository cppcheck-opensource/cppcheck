using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Text.RegularExpressions;

namespace difftool
{
    class Program
    {
        static private string FirstLine(string s)
        {
            int i = s.IndexOf('\n');
            return i > 0 ? s.Substring(0, i) : s;
        }

        static private List<string> GetDiff(string[] s1, string[] s2)
        {
            List<string> ret = new List<string>();

            int i2 = 0;
            while (i2 < s2.Length && (s2[i2] == "" || !s2[i2].StartsWith("mozilla/")))
                i2++;
            for (int i1 = 0; i1 < s1.Length; ++i1)
            {
                if (s1[i1] == "" || !s1[i1].StartsWith("mozilla/"))
                    continue;

                if (i2 >= s2.Length)
                {
                    ret.Add(s1[i1]);
                    continue;
                }
                if (FirstLine(s1[i1]) == FirstLine(s2[i2]))
                {
                    ++i2;
                    continue;
                }
                for (int i3 = i2 + 1; i3 < i2 + 300 && i3 < s2.Length; ++i3)
                {
                    if (FirstLine(s1[i1]) == FirstLine(s2[i3]))
                    {
                        i2 = i3;
                        break;
                    }
                }
                if (FirstLine(s1[i1]) == FirstLine(s2[i2]))
                {
                    ++i2;
                    continue;
                }
                ret.Add(s1[i1]);
            }
            return ret;
        }

        static private string TextToHtml(string s)
        {
            return s.Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;");
        }

        static Boolean IsNameChar(char c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
        }

        // color theme:

        // http://www.eclipsecolorthemes.org/?view=theme&id=47
        /*
        private const string backgroundColor = "1E1E1E";
        private const string foregroundFont = "<font color=\"#E2E2E2\">";
        private const string keywordFont = "<font color=\"#8DCBE2\">";
        private const string numberFont = "<font color=\"#EAB882\">";
        private const string stringFont = "<font color=\"#CC9393\">";
        private const string singleLineCommentFont = "<font color=\"#7F9F7F\">";
        private const string multiLineCommentFont = "<font color=\"#73879B\">";
        private const string lineNumberFont = "<font color=\"#C0C0C0\">";
        private const string graphBackgroundColor = "#303030";
        private const string graphBorderColor = "#8DCBE2";
        private const string graphTextColor = "#8DCBE2";
        */
        private const string backgroundColor = "white";
        private const string foregroundFont = "<font color=\"black\">";
        private const string symbolFont = "<font style=\"background-color:#c0c0c0\">";
        private const string keywordFont = "<font color=\"blue\">";
        private const string numberFont = "<font color=\"green\">";
        private const string stringFont = "<font color=\"#800000\">";
        private const string singleLineCommentFont = "<font color=\"gray\">";
        private const string multiLineCommentFont = "<font color=\"gray\">";
        private const string lineNumberFont = "<font color=\"black\">";
        private const string graphBackgroundColor = "#e0e0e0";
        private const string graphBorderColor = "black";
        private const string graphTextColor = "black";

        static private void writeSourceFile(string path, string w, string filename)
        {
            string errorLoc;
            string errorMessage;
            if (w[0] == '[')
            {
                errorLoc = w.Substring(1, w.IndexOf(']') - 1);
                int i1 = w.IndexOf(')') + 1;
                errorMessage = w.Substring(i1);
            }
            else
            {
                errorLoc = w.Substring(0, w.IndexOf(':', w.IndexOf(':') + 1));
                int i1 = w.IndexOf(": ", w.IndexOf(": ") + 1) + 1;
                int i2 = w.LastIndexOf('[') - 1;
                errorMessage = w.Substring(i1, i2 - i1).Trim();
            }
            string f = errorLoc.Substring(0, errorLoc.LastIndexOf(':'));
            int errorLine = Int32.Parse(errorLoc.Substring(1 + errorLoc.LastIndexOf(':')));
            string symbolName = "";
            if (errorMessage.IndexOf(": ") > 5)
            {
                string end = errorMessage.Substring(errorMessage.LastIndexOf(": "));
                // get symbol name
                Regex regex = new Regex(": '?[a-zA-Z_][a-zA-Z0-9_]*'?\\.?");
                if (regex.IsMatch(end))
                    symbolName = end.Trim(':', ' ', '\'', '.');
            }
            string[] lines = System.IO.File.ReadAllLines(path + f);
            string[] keywords = { "void", "bool", "char", "short", "int", "long", "double", "float",
                                  "class", "struct", "union", "enum", "namespace", "friend", "using",
                                  "for", "if", "while", "switch", "case", "default",
                                  "return", "goto", "break", "continue", "throw",
                                  "unsigned", "signed",
                                  "private", "protected", "public",
                                  "static", "const", "template", "typename", "auto", "virtual",
                                  "true", "false", "this" };
            System.IO.StreamWriter file = new System.IO.StreamWriter(path + filename);
            file.WriteLine("<html>");
            file.WriteLine("<head>");
            file.WriteLine("<title>" + TextToHtml(w) + "</title>");
            file.WriteLine("<style>");
            file.WriteLine("  table {");
            file.WriteLine("    border: 0px;");
            file.WriteLine("    border-spacing: 0;");
            file.WriteLine("  }");
            file.WriteLine("  td {");
            file.WriteLine("    border: 1px solid red;");
            file.WriteLine("    padding: 5px;");
            file.WriteLine("    background-color: #ffe0e0;");
            file.WriteLine("}");
            file.WriteLine("</style>");
            file.WriteLine("</head>");
            file.WriteLine("<body bgcolor=\"" + backgroundColor + "\"><pre>" + foregroundFont + TextToHtml(w) + "\n");
            int linenr = 0;
            bool multiline = false;
            foreach (string line in lines)
            {
                ++linenr;
                file.Write(lineNumberFont + (linenr.ToString() + "        ").Substring(0, 8) + "</font>");

                for (int i = 0; i < line.Length;)
                {
                    if (line[i] == ' ')
                    {
                        file.Write(line[i]);
                        i++;
                    }
                    else if (multiline)
                    {
                        string currentToken = "";
                        while (i < line.Length && multiline)
                        {
                            currentToken += line[i];
                            i++;
                            if (currentToken.EndsWith("*/"))
                                multiline = false;
                        }
                        file.Write(multiLineCommentFont + TextToHtml(currentToken) + "</font>");
                    }
                    else if (IsNameChar(line[i]))
                    {
                        string currentToken = "";
                        while (i < line.Length && IsNameChar(line[i]))
                        {
                            currentToken += line[i];
                            i++;
                        }
                        if (currentToken == symbolName)
                            file.Write(symbolFont + currentToken + "</font>");
                        else if (keywords.Contains(currentToken))
                            file.Write(keywordFont + currentToken + "</font>");
                        else if (currentToken[0] >= '0' && currentToken[0] <= '9')
                            file.Write(numberFont + currentToken + "</font>");
                        else
                            file.Write(currentToken);
                    }
                    else if (line[i] == '\"' || line[i] == '\'')
                    {
                        string currentToken = "";
                        while (i < line.Length)
                        {
                            currentToken += line[i];
                            i++;
                            if (currentToken.Length > 1 && currentToken[0] == line[i - 1])
                                break;
                        }
                        file.Write(stringFont + TextToHtml(currentToken) + "</font>");
                    }
                    else if (line[i] == '/' && i + 1 < line.Length && line[i + 1] == '/')
                    {
                        file.Write(singleLineCommentFont + TextToHtml(line.Substring(i)) + "</font>");
                        i = line.Length;
                    }
                    else if (line[i] == '/' && i + 1 < line.Length && line[i + 1] == '*')
                    {
                        multiline = true;
                        string currentToken = "/*";
                        i += 2;
                        while (i < line.Length && multiline)
                        {
                            currentToken += line[i];
                            i++;
                            if (currentToken.Length > 3 && currentToken.EndsWith("*/"))
                                multiline = false;
                        }
                        file.Write(multiLineCommentFont + TextToHtml(currentToken) + "</font>");
                    }
                    else
                    {
                        file.Write(TextToHtml(line.Substring(i, 1)));
                        i++;
                    }
                }
                file.Write('\n');

                if (errorLine == linenr)
                    file.WriteLine("<table width=\"100%\"><tr><td><b>" + TextToHtml(errorMessage) + "</td></tr></table>");
            }
            file.Write("</font></pre></body></html>");
            file.Close();
        }

        static private void combineWarnings(string[] lines)
        {
            for (int i = 0; i < lines.Length; ++i)
            {
                if (lines[i].IndexOf(": note:") > 0 || !lines[i].StartsWith("mozilla/"))
                    lines[i] = "";
                continue;

                int i2 = i + 1;
                while (i2 < lines.Length && lines[i2].IndexOf(": note:") > 0)
                {
                    lines[i] = lines[i] + '\n' + lines[i2];
                    lines[i2] = "";
                    i2++;
                }
            }
        }

        static void Main(string[] args)
        {
            string path;
            if (Directory.Exists("/home/danielm/shared/testing"))
                path = "/home/danielm/shared/testing/";
            else if (Directory.Exists("/home/danielm/testing"))
                path = "/home/danielm/testing/";
            else if (Directory.Exists(@"c:\Users\danielm.evidente\testing"))
                path = @"c:\Users\danielm.evidente\testing\";
            else
            {
                Console.WriteLine("Directory testing not found");
                return;
            }

            string[] old = System.IO.File.ReadAllLines(path + "1.82.txt");
            string[] head = System.IO.File.ReadAllLines(path + "head.txt");

            combineWarnings(old);
            combineWarnings(head);

            Array.Sort(old, StringComparer.InvariantCulture);
            Array.Sort(head, StringComparer.InvariantCulture);

            List<string> removed = GetDiff(old, head);
            List<string> added = GetDiff(head, old);

            int fileIndex = 1;

            using (System.IO.StreamWriter file = new System.IO.StreamWriter(path + "html/main.html"))
            {
                file.WriteLine("<html>");
                file.WriteLine("<head>");
                file.WriteLine("<title>Comparison 1.82 / Head</title>");
                file.WriteLine("</head>");
                file.WriteLine("<body bgcolor=\"" + backgroundColor + "\">");
                file.WriteLine(foregroundFont);

                file.WriteLine("<h1>Comparison 1.82 / Head</h1>");
                file.WriteLine("<p>Package: {0}</p>", @"ftp://ftp.se.debian.org/debian/pool/main/f/firefox/firefox_59.0~b4.orig.tar.bz2");
                file.WriteLine("<p>+ Added warnings {0}</p>", added.Count);
                file.WriteLine("<p>- Removed warnings {0}</p>", removed.Count);

                file.WriteLine("<svg width=\"{0}\" height=\"{1}\">", 60 + Math.Max(added.Count, removed.Count) + 10, 40);
                file.WriteLine("  <rect x=\"0\" y=\"0\" width=\"100%\" height=\"100%\" stroke=\"" + graphBorderColor + "\" fill=\"" + graphBackgroundColor + "\"/>");
                file.WriteLine("  <text x=\"4\" y=\"15\" font-family=\"Verdana\" font-size=\"10\" fill=\"" + graphTextColor + "\">Added</text>");
                file.WriteLine("  <text x=\"4\" y=\"30\" font-family=\"Verdana\" font-size=\"10\" fill=\"" + graphTextColor + "\">Removed</text>");
                file.WriteLine("  <rect x=\"60\" y=\"8\" width=\"{0}\" height=\"8\" fill=\"#40ff40\" />", added.Count);
                file.WriteLine("  <rect x=\"60\" y=\"23\" width=\"{0}\" height=\"8\" fill=\"#ff4040\" />", removed.Count);
                file.WriteLine("</svg>");

                string[] title = { "Added warnings", "Removed warnings" };
                List<string>[] warnings = { added, removed };
                for (int i = 0; i < 2; i++)
                {
                    file.WriteLine("<h2>" + title[i] + "</h2>");
                    file.WriteLine("<pre>");
                    foreach (string w in warnings[i])
                    {
                        string filename = "code" + fileIndex.ToString() + ".html";
                        fileIndex++;
                        writeSourceFile(path, w, "html/" + filename);
                        file.WriteLine("<a href=\"{0}\">{1}</a>", filename, TextToHtml(w));
                    }
                    file.WriteLine("</pre>");
                }

                file.WriteLine("</body>");
                file.WriteLine("</html>");
            }
        }
    }
}
