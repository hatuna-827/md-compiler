#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
std::regex Heading(R"(^\s*(#{1,6})\s*(.*))");
std::regex Break(R"(^(.*)(\s\s|\\)$)");
std::regex Bold_Italic(R"(([^\\])([*_]{3})((?:(?!\2).)+)\2)");
std::regex Bold(R"(([^\\])([*_]{2})((?:(?!\2).)+)\2)");
std::regex Italic(R"(([^\\])([*_])((?:(?!\2).)+)\2)");
std::regex Partition(R"(^\s*(?:[-_*]\s*){3,}$)");
std::regex Strikethrough(R"(~~(.+?)~~)");
std::regex InlineCode(R"((^|[^\\])(`{1,2})((?:(?!\2).)+?[^\\])\2)");
std::regex CodeBlock(R"(^(`{3,})\s*(\S*)$)");
std::regex Image(R"(!\[(.*?)\]\(\s*(\S*?)(?:\s+\"(.*?)\")?\s*\))");
std::regex Anchor(R"(\[(.*?)\]\(\s*(\S*?)(?:\s+\"(.*?)\")?\s*\))");
std::regex URL(R"(https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+)");
std::regex UnorderedList(R"(^[-+*]\s+(.*))");
std::regex OrderedList(R"(^\d+\.\s+(.*))");
std::regex Blockquotes(R"()");
std::regex Checkbox(R"()");
std::regex Table(R"()");

int main(int argc, char *argv[])
{
  std::string inFileName, outFileName;
  if (argc == 1)
  {
    std::cout << "Error: Not enough arguments" << std::endl;
    return 0;
  }
  else
  {
    inFileName = argv[1];
    outFileName = 2 < argc ? argv[2] : "index.html";
  }
  std::ofstream oFile(outFileName, std::ios::trunc);
  std::ifstream iFile(inFileName);
  oFile << "<!DOCTYPE HTML><head><meta charset=\"utf-8\"/><link rel=\"stylesheet\" href=\"markdown-style.css\"><title> "
        << outFileName << "</title></head><body>";
  bool emp_flag = true;
  bool br_flag = false;
  bool code_flag = false;
  int code_cnt = 0;
  int blqu_cnt = 0;
  int table_cnt = 0;
  std::string line_md;
  std::string line_html;
  std::smatch m;
  while (std::getline(iFile, line_md))
  {
    line_html = "";
    if (code_cnt == 0)
    {
      if (line_md.empty())
      {
        if (emp_flag)
        {
          line_html = "";
        }
        else
        {
          line_html = "<br>";
        }
      }
      else
      {
        line_html = line_md;
        if (std::regex_match(line_md, m, Heading))
        {
          line_html = "<h" + std::to_string(m[1].str().length()) + '>' +
                      m[2].str() +
                      "</h" + std::to_string(m[1].str().length()) + '>';
        }
        else if (std::regex_match(line_md, m, CodeBlock))
        {
          line_html = "<pre>";
          code_cnt = m[1].str().length();
          if (m[2].str().empty())
          {
            line_html += "<code>";
            code_flag = true;
          }
        }
        else if (std::regex_match(line_md, m, Partition))
        {
          line_html = std::regex_replace(line_html, Partition, "<hr>");
        }
        else
        {
          if (!br_flag)
            line_html = "<p>";
          if (std::regex_match(line_md, m, Break))
          {
            line_html = m[1].str() + "<br>";
            br_flag = true;
          }
          else
          {
            line_html += line_md + "</p>";
            br_flag = false;
          }
          line_html = std::regex_replace(line_html, Bold_Italic, "$1<strong><em>$3</em></strong>");
          line_html = std::regex_replace(line_html, Bold, "$1<strong>$3</strong>");
          line_html = std::regex_replace(line_html, Italic, "$1<em>$3</em>");
          line_html = std::regex_replace(line_html, Strikethrough, "<s>$1</s>");
          line_html = std::regex_replace(line_html, InlineCode, "$1<code>$3</code>");
          line_html = std::regex_replace(line_html, Image, "<img alt=\"$1\" src=\"$2\" title=\"$3\">");
          line_html = std::regex_replace(line_html, Anchor, "<a href=\"$2\" title=\"$3\">$1</a>");
        }
      }
    }
    else
    {
      // In codeblock
      if (line_md == std::string(code_cnt, '`'))
      {
        if (code_flag)
          line_html += "</code>";
        line_html += "</pre>";
        code_cnt = 0;
        code_flag = false;
      }
      else
      {
        line_html = line_md + "\n";
      }
    }
    oFile << line_html;
  }
  oFile << "</body></HTML>";
  oFile.close();
  iFile.close();
  return 0;
}
