#include "GLSLSyntaxHighlighter.h"

const char *GLSLSyntaxHighlighter::c_keywords_[] = {
  "attribute", "const", "uniform", "varying", "centroid", "break", "continue",
  "do", "for", "while", "if", "else", "in", "inout", "out", "true", "false",
  "invariant", "discard", "return", "struct",
  "lowp", "mediump", "highp", "precision", 0
};

const char *GLSLSyntaxHighlighter::c_types_[] = {
  "void", "float", "int", "bool", "mat2", "mat3", "mat4",
  "mat2x2", "mat2x3", "mat2x4", "mat3x2", "mat3x3", "mat3x4",
  "mat4x2", "mat4x3", "mat4x4", "vec2", "vec3", "vec4",
  "ivec2", "ivec3", "ivec4", "bvec2", "bvec3", "bvec4",
  "sampler1D", "sampler2D", "sampler3D", "samplerCube",
  "sampler1DShadow", "sampler2DShadow", 0
};

const char *GLSLSyntaxHighlighter::c_functions_[] = {
  "radians", "degrees", "sin", "cos", "tan", "asin", "acos", "atan", "pow",
  "exp", "log", "exp2", "log2", "sqrt", "inversesqrt", "abs", "sign", "floor",
  "ceil", "fract", "mod", "min", "max", "clamp", "mix", "step", "smoothstep",
  "length", "distance", "dot", "cross", "normalize", "ftransform",
  "faceforward", "reflect", "refract", "matrixCompMult", "outerProduct",
  "transpose", "lessThan", "lessThanEqual", "greaterThan", "greaterThanEqual",
  "equal", "notEqual", "any", "all", "not",
  "texture1D", "texture1DProj", "texture1DLod", "Texture1DProjLod",
  "texture2D", "texture2DProj", "texture2DLod", "Texture2DProjLod",
  "texture3D", "texture3DProj", "texture3DLod", "Texture3DProjLod",
  "textureCube", "textureCubeLod",
  "shadow1D", "shadow1DProj", "shadow1DLod", "Texture1DProjLod",
  "dFdx", "dFdy", "fwidth",
  0
};

GLSLSyntaxHighlighter::GLSLSyntaxHighlighter(QTextDocument *doc)
  : QSyntaxHighlighter(doc),
  multicomStart_("/\\*"), multicomEnd_("\\*/") {
  commentFormat_.setForeground(Qt::darkGreen);
  commentFormat_.setFontItalic(true);
  keywordsFormat_.setForeground(Qt::darkRed);
  typesFormat_.setForeground(Qt::darkBlue);
  //typesFormat_.setFontWeight(QFont::Bold);
  functionsFormat_.setForeground(Qt::darkGreen);
  macroFormat_.setForeground(Qt::darkRed);
  //macroFormat_.setFontWeight(QFont::Bold);
  //builtinFormat_.setFontWeight(QFont::Bold);
  builtinFormat_.setForeground(Qt::red);

  addPatternsToRules(c_keywords_, keywordsFormat_);
  addPatternsToRules(c_types_, typesFormat_);
  addPatternsToRules(c_functions_, functionsFormat_);

  rules_.append(Rule(QRegExp("\\bgl_.*\\b"), builtinFormat_));
  rules_.append(Rule(QRegExp("#[^\n]*"), macroFormat_));
  rules_.append(Rule(QRegExp("//[^\n]*"), commentFormat_));
}

void GLSLSyntaxHighlighter::addPatternsToRules(const char **patterns,
                                               const QTextCharFormat &format) {
  for(; *patterns != 0; ++patterns)
    rules_.append(Rule(QRegExp(QString("\\b%1\\b").arg(*patterns)), format));
}

void GLSLSyntaxHighlighter::highlightBlock(const QString &text) {
  foreach(const Rule &rule, rules_) {
    int index = rule.pattern.indexIn(text);
    while (index >= 0) {
      int length = rule.pattern.matchedLength();
      setFormat(index, length, *rule.format);
      index = rule.pattern.indexIn(text, index + length);
    }
  }

  setCurrentBlockState(BlockPlain);
  int start = (previousBlockState() != BlockMultilineComment) 
    ? text.indexOf(multicomStart_) : 0;
  while (start >= 0) {
    int end = text.indexOf(multicomEnd_, start);
    int length;
    if (end == -1) {
      setCurrentBlockState(BlockMultilineComment);
      length = text.length() - start;
    } else {
      length = end - start + multicomEnd_.matchedLength();
    }
    setFormat(start, length, commentFormat_);
    start = text.indexOf(multicomStart_, start + length);
  }
}
