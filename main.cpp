#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
std::regex Indent(R"(^(\s*)(.*)$)");
std::regex Heading(R"(^\s*(#{1,6})\s*(.*)$)");
std::regex Break(R"(^(.*)(\s\s|\\)$)");
std::regex Bold_Italic(R"(([^\\]|^)([*_]{3})((?:(?!\2).)+)\2)");
std::regex Bold(R"(([^\\]|^)([*_]{2})((?:(?!\2).)+)\2)");
std::regex Italic(R"(([^\\]|^)([*_])((?:(?!\2).)+)\2)");
std::regex Partition(R"(^\s*(?:[-_*]\s*){3,}$)");
std::regex Strikethrough(R"(~~((?!~~).+?)~~)");
std::regex InlineCode(R"((^|[^\\`])(`+)([^`](?:(?!\2).)+?[^\\`])\2(?!`))");
std::regex CodeBlock(R"(^ {0,3}(`{3,})[^`]*$)");
std::regex CodeBlockClose(R"(^ {0,3}(`{3,})\s*$)");
std::regex IndentCodeBlock(R"(^ {4}(.*)$)");
std::regex Image(R"(!\[(.*?)\]\(\s*(\S*?)(?:\s+\"(.*?)\")?\s*\))");
std::regex Anchor(R"(\[(.*?)\]\(\s*(\S*?)(?:\s+\"(.*?)\")?\s*\))");
std::regex URL(R"(<(https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+)>)");
std::regex rawURL(R"((?:^|\s)(https?://[\w!?/+\-_~;.,*&@#$%()'[\]]+)(?:$|\s))");
std::regex Mail(R"(<([\w\d.+-]+@([a-zA-Z\d][a-zA-Z\d-]*[a-zA-Z\d]*\.)+[a-zA-Z]{2,})>)");
std::regex rawMail(R"((?:^|\s)([\w\d.+-]+@([a-zA-Z\d][a-zA-Z\d-]*[a-zA-Z\d]*\.)+[a-zA-Z]{2,})(?:$|\s))");
std::regex UnorderedList(R"(^(\s{0,3}[-+*]\s)(.*)$)");
std::regex OrderedList(R"(^(\s{0,3}\d+\.\s)(.*)$)");
std::regex Blockquotes(R"(^(\s{0,3}>\s)(.*)$)");
std::regex Checkbox(R"(^\[[ xX]\])");
std::regex Table(R"(^(?:\s*\|\s*.*?)+\|\s*$)");
std::regex TableMeta(R"(^(?:\s*\|\s*:?-+:?)+\|\s*$)");
std::regex Escape(R"(\\([\!\"\#\$\%\&\'\(\)\-\^\\\@\[\;\:\]\,\.\/\\\=\~\|\`\{\+\*\}\<\>\?\_]))");

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
  oFile << "<!DOCTYPE HTML><head><meta charset=\"utf-8\"/><link rel=\"stylesheet\" href=\"markdown-style.css\"><title>"
        << outFileName << "</title></head><body>";
  bool br_flag = false;
  bool p_flag = false;
  bool p_close_flag = false;
  bool code_flag = false;
  bool code_close_flag = false;
  bool list_blqu_flag = false;
  int indent = 0;
  int code_cnt = 0;
  int blqu_cnt = 0;
  char list_type = ' ';
  std::vector<int> list_cnt = {0};
  std::vector<bool> is_ordered = {};
  std::string line_md;
  std::string line_html;
  std::string line_html_start;
  std::string line_html_content;
  std::string line_html_end;
  std::smatch m;
  while (std::getline(iFile, line_md))
  {
    std::regex_match(line_md, m, Indent);
    indent = m[1].str().length() - list_cnt.back();
    if (indent < 0)
    {
      if (p_flag)
        oFile << "</p>";
      p_flag = false;
      if (code_cnt == 0)
      {
        if (code_flag)
          oFile << "</pre>";
      }
      else
      {
        if (code_flag)
          oFile << "</code>";
        oFile << "</pre>";
      }
      code_flag = false;
      code_cnt = 0;
      while (indent < 0)
      {
        if (is_ordered.back())
          oFile << "</ol>";
        else
          oFile << "</ul>";
        list_cnt.pop_back();
        is_ordered.pop_back();
        indent = m[1].str().length() - list_cnt.back();
      }
    }
    line_md = std::string(indent, ' ') + m[2].str();
    line_html = "";
    if (code_cnt == 0)
    {
      // Outside the fence code
      p_close_flag = p_flag;
      p_flag = false;
      code_close_flag = code_flag;
      if (line_md.empty())
      {
      }
      else if (std::regex_match(line_md, m, IndentCodeBlock))
      {
        if (!code_flag)
          line_html += "<pre>";
        line_html += (m[1].str() + "\n");
        code_flag = true;
        code_close_flag = false;
      }
      else if (std::regex_match(line_md, m, CodeBlock))
      {
        line_html += "<pre>";
        code_cnt = m[1].str().length();
        if (m[2].str().empty())
        {
          line_html += "<code>";
          code_flag = true;
        }
      }
      else if (std::regex_match(line_md, m, Partition))
      {
        line_html += "<hr>";
      }
      else
      {
        line_html_start = "";
        line_html_end = "";
        list_blqu_flag = true;
        // while (list_blqu_flag)
        // {
        //   list_blqu_flag = false;
        //   // list
        //   while (std::regex_match(line_md, m, UnorderedList))
        //   {
        //     list_blqu_flag = true;
        //   }
        //   while (std::regex_match(line_md, m, OrderedList))
        //   {
        //     list_blqu_flag = true;
        //   }
        //   // blqu
        //   while (std::regex_match(line_md, m, Blockquotes))
        //   {
        //     list_blqu_flag = true;
        //   }
        // }
        if (std::regex_match(line_md, m, Heading))
        {
          line_html_start = (line_html_start + "<h" + std::to_string(m[1].str().length()) + ">");
          line_html_content = m[2].str();
          line_html_end = ("</h" + std::to_string(m[1].str().length()) + ">" + line_html_end);
        }
        else
        {
          line_html_content = line_md;
          p_flag = true;
          if (p_close_flag)
          {
            p_close_flag = false;
            if (!br_flag)
              line_html_start += " ";
          }
          else
            line_html_start += "<p>";
          if (std::regex_match(line_md, m, Break))
          {
            line_html_content = std::regex_replace(line_html_content, Break, "$1");
            line_html_end = ("<br>" + line_html_end);
            br_flag = true;
          }
          else
            br_flag = false;
        }
        line_html_content = std::regex_replace(line_html_content, Bold_Italic, "$1<strong><em>$3</em></strong>");
        line_html_content = std::regex_replace(line_html_content, Bold, "$1<strong>$3</strong>");
        line_html_content = std::regex_replace(line_html_content, Italic, "$1<em>$3</em>");
        line_html_content = std::regex_replace(line_html_content, Strikethrough, "<s>$1</s>");
        line_html_content = std::regex_replace(line_html_content, InlineCode, "$1<code>$3</code>");
        line_html_content = std::regex_replace(line_html_content, Image, "<img alt=\"$1\" src=\"$2\" title=\"$3\">");
        line_html_content = std::regex_replace(line_html_content, Anchor, "<a href=\"$2\" title=\"$3\">$1</a>");
        line_html_content = std::regex_replace(line_html_content, URL, "<a href=\"$1\">$1</a>");
        line_html_content = std::regex_replace(line_html_content, rawURL, "<a href=\"$1\">$1</a>");
        line_html_content = std::regex_replace(line_html_content, Mail, "<a href=\"mailto:$1\">$1</a>");
        line_html_content = std::regex_replace(line_html_content, rawMail, "<a href=\"mailto:$1\">$1</a>");
        line_html = line_html_start + line_html_content + line_html_end;
      }
      if (code_close_flag)
      {
        line_html = ("</pre>" + line_html);
        if (code_cnt == 0)
          code_flag = false;
      }
      if (p_close_flag)
        line_html = ("</p>" + line_html);
      line_html = std::regex_replace(line_html, Escape, "$1");
    }
    else
    {
      // Inside the fence code
      std::regex_match(line_md, m, CodeBlockClose);
      if (m[1].str().length() == code_cnt)
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
