#pragma once
#include <QSyntaxHighlighter>

class GLSLSyntaxHighlighter : public QSyntaxHighlighter {
public:
  GLSLSyntaxHighlighter(QTextDocument *doc);
  ~GLSLSyntaxHighlighter() {}

  void highlightBlock(const QString &text);

private:
  struct Rule {
    QRegExp pattern;
    const QTextCharFormat *format;
    inline Rule() : format(0) {}
    inline Rule(const QRegExp &_pattern, const QTextCharFormat &_format)
      : pattern(_pattern), format(&_format) {}
  };
  QVector<Rule> rules_;
  void addPatternsToRules(const char **patterns, const QTextCharFormat &format);

  enum BlockType {
    BlockPlain,
    BlockMultilineComment
  };
  QRegExp multicomStart_, multicomEnd_;
  QTextCharFormat commentFormat_;
  QTextCharFormat keywordsFormat_;
  QTextCharFormat typesFormat_;
  QTextCharFormat functionsFormat_;
  QTextCharFormat macroFormat_;
  QTextCharFormat builtinFormat_;

  static const char *c_keywords_[];
  static const char *c_types_[];
  static const char *c_functions_[];
}; // class GLSLSyntaxHighlighter
