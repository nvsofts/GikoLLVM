#ifndef __GIKO_PARSER_HPP
#define __GIKO_PARSER_HPP

#include <string>
#include <vector>

#include <boost/fusion/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "ast.hpp"

namespace giko
{

namespace parser
{

using namespace boost::spirit;
using namespace giko::ast;

template<typename Iterator, typename Skipper>
struct giko_grammar : qi::grammar<Iterator, ModuleAST *(), Skipper>
{
  qi::rule<Iterator, std::string(), Skipper> id;
  qi::rule<Iterator, std::vector<std::string>(), Skipper> vars;
  qi::rule<Iterator, FunctionAST *(), Skipper> func;
  qi::rule<Iterator, ModuleAST *(), Skipper> module;
  qi::rule<Iterator, BaseAST *(), Skipper> statement;
  qi::rule<Iterator, BuiltinAST *(), Skipper> builtin;
  qi::rule<Iterator, AssignAST *(), Skipper> assign;
  qi::rule<Iterator, IfStatementAST *(), Skipper> if_statement;
  qi::rule<Iterator, WhileStatementAST *(), Skipper> while_statement;
  qi::rule<Iterator, StatementsAST *(), Skipper> statements;
  qi::rule<Iterator, BaseAST *(), Skipper> l0, l1, l2, l3, l4, expr;

  giko_grammar() : giko_grammar::base_type(module)
  {
    using namespace boost::spirit::qi;
    using namespace boost::phoenix;
    using namespace boost;

    // 識別子
    id = lexeme[alpha[_val = _1] >> *(alnum[_val += _1])];

    // 変数宣言
    vars = "ﾍﾝｽｳ" >> id[push_back(_val, _1)] >> *(',' >> id[push_back(_val, _1)]);

    // 関数
    func = "ﾒｼﾞﾙｼ" >> id[_val = new_<FunctionAST>(_1)] >> *statements[push_back(phoenix::at_c<1>(*_val), _1)];

    // モジュール
    module = vars[_val = new_<ModuleAST>(), phoenix::at_c<0>(*_val) = _1] >> *func[push_back(phoenix::at_c<1>(*_val), _1)];

    // 文
    statement = builtin | assign | if_statement | while_statement;

    // 代入
    assign = id[_val = new_<AssignAST>(_1)] >> '=' >> expr[phoenix::at_c<1>(*_val) = _1];

    // 組み込み命令
    builtin = ("ﾎｻﾞｹ" >> id[_val = new_<BuiltinAST>("print"), push_back(phoenix::at_c<1>(*_val), new_<IdentifierAST>(_1))])
              | ("ｲﾚﾃﾐﾛ" >> id[_val = new_<BuiltinAST>("scan"), push_back(phoenix::at_c<1>(*_val), new_<IdentifierAST>(_1))])
              | ("ｼﾈ" >> eps[_val = new_<BuiltinAST>("exit")])
              | ("ﾗﾝｽｳ" >> id[_val = new_<BuiltinAST>("rand"), push_back(phoenix::at_c<1>(*_val), new_<IdentifierAST>(_1))])
              | ("ｲｯﾃｺｲ" >> id[_val = new_<BuiltinAST>("call"), push_back(phoenix::at_c<1>(*_val), new_<IdentifierAST>(_1))])
              | ("ｶｴﾚ" >> eps[_val = new_<BuiltinAST>("return")])
              | ("ﾇｹﾀﾞｾ" >> eps[_val = new_<BuiltinAST>("break")])
              | ("ﾂﾂﾞｹﾛ" >> eps[_val = new_<BuiltinAST>("continue")]);

    // if文
    if_statement = "ﾓｼﾓﾀﾞﾖ" >> expr[_val = new_<IfStatementAST>(), phoenix::at_c<0>(*_val) = _1]
                            >> "ﾀﾞｯﾀﾗ" >> statements[phoenix::at_c<1>(*_val) = _1]
                            >> -("ｼﾞｬﾅｲﾅﾗ" >> statements[phoenix::at_c<2>(*_val) = _1]);

    // while文
    while_statement = "ﾙｰﾌﾟ" >> expr[_val = new_<WhileStatementAST>(), phoenix::at_c<0>(*_val) = _1] >> "ｶｲｼ"
                             >> *statements[push_back(phoenix::at_c<1>(*_val), _1)] >> "ﾙｰﾌﾟｵﾜﾘ";

    // 文の集合
    statements = eps[_val = new_<StatementsAST>()] >> statement[push_back(phoenix::at_c<0>(*_val), _1)]
                                                   >> *(':' >> statement[push_back(phoenix::at_c<0>(*_val), _1)]);

    // 式
    l0 = int_[_val = new_<NumberAST>(_1)] | id[_val = new_<IdentifierAST>(_1)] | '(' >> expr[_val = _1] >> ')';
    l1 = l0[_val = _1] >> *( ('*' >> l0[_val = new_<BinaryExprAST>("*", _val, _1)])
                             | ('/' >> l0[_val = new_<BinaryExprAST>("/", _val, _1)])
                             | ('%' >> l0[_val = new_<BinaryExprAST>("%", _val, _1)]));
    l2 = l1[_val = _1] >> *( ('+' >> l1[_val = new_<BinaryExprAST>("+", _val, _1)])
                             | ('-' >> l1[_val = new_<BinaryExprAST>("-", _val, _1)]));
    l3 = l2[_val = _1] >> *( ('=' >> l2[_val = new_<BinaryExprAST>("=", _val, _1)])
                             | ('<' >> l2[_val = new_<BinaryExprAST>("<", _val, _1)])
                             | ('>' >> l2[_val = new_<BinaryExprAST>(">", _val, _1)])
                             | ("<=" >> l2[_val = new_<BinaryExprAST>("<=", _val, _1)])
                             | (">=" >> l2[_val = new_<BinaryExprAST>(">=", _val, _1)]));
    l4 = l3[_val = _1] | ("ﾁｶﾞｳﾔﾂ" >> l3[_val = new_<MonoExprAST>("!", _1)]);
    expr = l4[_val = _1] >> *( ("ｶﾂ" >> l4[_val = new_<BinaryExprAST>("&&", _val, _1)])
                             | ("ﾏﾀﾊ" >> l4[_val = new_<BinaryExprAST>("||", _val, _1)]));
  }
};

}

}

#endif
