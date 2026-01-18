#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
std::regex Indent(R"(^(\s*)(.*)$)");
std::regex Heading(R"(^\s*(#{1,6})\s*(.*)$)");
std::regex HeadingLine1(R"(^\s*=+\s*$)");
std::regex HeadingLine2(R"(^\s*-+\s*$)");
std::regex Break(R"(^(.*)(\s\s|\\)$)");
std::regex Bold_Italic(R"(([^\\]|^)([*_])\2\2((?:(?!\2\2\2).)+)\2\2\2)");
std::regex Bold(R"(([^\\]|^)([*_])\2((?:(?!\2\2).)+)\2\2)");
std::regex Italic(R"(([^\\]|^)([*_])((?:(?!\2).)+)\2)");
std::regex Partition(R"(^\s*([-_*])(?:\s*\1){2,}\s*$)");
std::regex Strikethrough(R"((^|[^\\])~~((?!~~).+?[^\\])~~)");
std::regex InlineCode(R"((^|[^\\`])(`+)([^`](?:(?!\2).)+?[^\\`])\2(?!`))");
std::regex CodeBlock(R"(^ {0,3}(`{3,})[^`]*$)");
std::regex CodeBlockClose(R"(^ {0,3}(`{3,})\s*$)");
std::regex IndentCodeBlock(R"(^(?: {4}|\t)(.*)$)");
std::regex Image(R"(!\[(.*?)\]\(\s*(\S*?)(?:\s+\"(.*?)\")?\s*\))");
std::regex Anchor(R"(\[(.*?)\]\(\s*(\S*?)(?:\s+\"(.*?)\")?\s*\))");
std::regex Reference(R"(^\s*\[(.*)\]:\s*(\S*)\s+\"(.*)\"\s*$)");
std::regex URL(R"(&lt;(https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+)&gt;)");
std::regex rawURL(R"((?:^|\s)(https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+)(?:$|\s))");
std::regex Mail(R"(&lt;([\w\d.+-]+@([a-zA-Z\d][a-zA-Z\d-]*[a-zA-Z\d]*\.)+[a-zA-Z]{2,})&gt;)");
std::regex rawMail(R"((?:^|\s)([\w\d.+-]+@([a-zA-Z\d][a-zA-Z\d-]*[a-zA-Z\d]*\.)+[a-zA-Z]{2,})(?:$|\s))");
std::regex UnorderedList(R"(^(\s{0,3}[-+*]\s)(.*)$)");
std::regex OrderedList(R"(^(\s{0,3}\d+\.\s)(.*)$)");
std::regex Blockquotes(R"(^(\s{0,3}>\s)(.*)$)");
std::regex Checkbox(R"(^\[[ xX]\])");
std::regex Table(R"(^(?:\s*\|\s*.*?)+\|\s*$)");
std::regex TableMeta(R"(^(?:\s*\|\s*:?-+:?)+\|\s*$)");
std::regex HtmlTagOpen(R"(\\?<)");
std::regex HtmlTagClose(R"(\\?>)");
std::regex Escape(R"(\\([\!\"\#\$\%\&\'\(\)\-\^\\\@\[\;\:\]\,\.\/\\\=\~\|\`\{\+\*\}\<\>\?\_]))");

class process
{
public:
  std::string md;
  std::string html = "";
  std::string header = "";
  std::string footer = "";
  process(std::string init_md) { md = init_md; }
  void add_tag(std::string new_header, std::string new_footer)
  {
    header = header + new_header;
    footer = new_footer + footer;
  }
  void add_header(std::string new_header)
  {
    header = new_header + header;
  }
  void add_footer(std::string new_footer)
  {
    footer = footer + new_footer;
  }
};

int main(int argc, char *argv[])
{
  std::string inFileName, outFileName, cssFileName;
  if (argc == 1)
  {
    std::cout << "Error: Not enough arguments" << std::endl;
    return 0;
  }
  else
  {
    inFileName = argv[1];
    outFileName = 2 < argc ? argv[2] : "index.html";
    cssFileName = 3 < argc ? argv[3] : "";
  }
  std::ofstream oFile(outFileName, std::ios::trunc);
  std::ifstream iFile(inFileName);
  if (!iFile)
  {
    std::cerr << "Error: The file could not be opened" << std::endl;
    return 1;
  }
  oFile << "<!DOCTYPE html><html><head><meta charset=\"utf-8\"/>";
  if (!cssFileName.empty())
    oFile << "<link rel=\"stylesheet\" href = \"" << cssFileName << "\">";
  oFile << "<title>" << outFileName << "</title></head><body>";
  std::vector<process> p;
  std::string buf;
  while (std::getline(iFile, buf))
  {
    p.emplace_back(buf);
  }
  bool br_flag = false;
  bool p_flag = false;
  bool p_close_flag = false;
  bool code_flag = false;
  bool code_close_flag = false;
  bool list_blqu_flag = false;
  int indent = 0;
  int code_cnt = 0;
  int blqu_cnt = 0;
  std::vector<int> list_cnt = {0};
  std::vector<char> indent_type = {' '};
  std::smatch m;
  for (int i = 0; i < p.size(); ++i)
  {
    // Outside the fence code
    std::regex_match(p[i].md, m, Indent);
    indent = m[1].str().length() - list_cnt.back();
    if (indent < 0)
    {
      if (p_flag)
        p[i - 1].add_footer("</p>");
      p_flag = false;
      if (code_cnt == 0)
      {
        if (code_flag)
          p[i - 1].add_footer("</pre>");
      }
      else
      {
        if (code_flag)
          p[i - 1].add_footer("</code>");
        p[i - 1].add_footer("</pre>");
      }
      code_flag = false;
      code_cnt = 0;
      while (indent < 0)
      {
        if (indent_type.back() == '1')
          p[i - 1].add_footer("</ol>");
        else
          p[i - 1].add_footer("</ul>");
        list_cnt.pop_back();
        indent_type.pop_back();
        indent = m[1].str().length() - list_cnt.back();
      }
    }
    p[i].md = std::string(indent, ' ') + m[2].str();
    p[i].md = std::regex_replace(p[i].md, HtmlTagOpen, "&lt;");
    p[i].md = std::regex_replace(p[i].md, HtmlTagClose, "&gt;");
    if (code_cnt == 0)
    {
      p_close_flag = p_flag;
      p_flag = false;
      code_close_flag = code_flag;
      if (p[i].md.empty())
      {
      }
      else if (std::regex_match(p[i].md, m, IndentCodeBlock))
      {
        if (!code_flag)
          p[i].add_tag("<pre>", "");
        p[i].html = m[1].str() + "\n";
        code_flag = true;
        code_close_flag = false;
      }
      else if (std::regex_match(p[i].md, m, CodeBlock))
      {
        p[i].add_tag("<pre>", "");
        code_cnt = m[1].str().length();
        if (m[2].str().empty())
        {
          p[i].add_tag("<code>", "");
          code_flag = true;
        }
      }
      // else if (std::regex_match(p[i].md, m, HeadingLine1))
      // {
      //   p[i - 1].add_tag("<h1>", "</h1>");
      // }
      // else if (std::regex_match(p[i].md, m, HeadingLine2))
      // {
      //   p[i - 1].add_tag("<h2>", "</h2>");
      // }
      else if (std::regex_match(p[i].md, m, Partition))
      {
        p[i].html = "<hr>";
      }
      else
      {
        if (std::regex_match(p[i].md, m, Heading))
        {
          p[i].add_tag("<h" + std::to_string(m[1].str().length()) + ">", "</h" + std::to_string(m[1].str().length()) + ">");
          p[i].html = m[2].str();
        }
        else
        {
          p[i].html = p[i].md;
          p_flag = true;
          if (p_close_flag)
          {
            p_close_flag = false;
            if (!br_flag)
              p[i].add_tag(" ", "");
          }
          else
            p[i].add_tag("<p>", "");
          if (std::regex_match(p[i].md, m, Break))
          {
            p[i].html = std::regex_replace(p[i].html, Break, "$1");
            p[i].add_tag("", "<br>");
            br_flag = true;
          }
          else
            br_flag = false;
        }
        p[i].html = std::regex_replace(p[i].html, Bold_Italic, "$1<strong><em>$3</em></strong>");
        p[i].html = std::regex_replace(p[i].html, Bold, "$1<strong>$3</strong>");
        p[i].html = std::regex_replace(p[i].html, Italic, "$1<em>$3</em>");
        p[i].html = std::regex_replace(p[i].html, Strikethrough, "$1<s>$2</s>");
        p[i].html = std::regex_replace(p[i].html, InlineCode, "$1<code>$3</code>");
        p[i].html = std::regex_replace(p[i].html, Image, "<img alt=\"$1\" src=\"$2\" title=\"$3\">");
        p[i].html = std::regex_replace(p[i].html, Anchor, "<a href=\"$2\" title=\"$3\">$1</a>");
        p[i].html = std::regex_replace(p[i].html, URL, "<a href=\"$1\">$1</a>");
        p[i].html = std::regex_replace(p[i].html, rawURL, "<a href=\"$1\">$1</a>");
        p[i].html = std::regex_replace(p[i].html, Mail, "<a href=\"mailto:$1\">$1</a>");
        p[i].html = std::regex_replace(p[i].html, rawMail, "<a href=\"mailto:$1\">$1</a>");
        p[i].html = std::regex_replace(p[i].html, Escape, "$1");
      }
      if (code_close_flag)
      {
        p[i - 1].add_footer("</pre>");
        if (code_cnt == 0)
          code_flag = false;
      }
      if (p_close_flag)
        p[i - 1].add_footer("</p>");
    }
    else
    {
      // Inside the fence code
      std::regex_match(p[i].md, m, CodeBlockClose);
      if (m[1].str().length() == code_cnt)
      {
        p[i].add_tag("", "</pre>");
        if (code_flag)
          p[i].add_tag("", "</code>");
        code_cnt = 0;
        code_flag = false;
      }
      else
      {
        p[i].html = p[i].md + "\n";
      }
    }
  }
  for (int i = 0; i < p.size(); ++i)
  {
    oFile << p[i].header << p[i].html << p[i].footer;
  }
  oFile << "</body></html>";
  oFile.close();
  iFile.close();
  return 0;
}
