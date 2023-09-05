#ifndef IOS
#define IOS
#include <iostream>
#include <fstream>
#include <sstream>
#endif
#ifndef MEM
#define MEM
#include <memory>
#endif
#ifndef VEC
#define VEC
#include <vector>
#include <set>
#include <map>
#endif

class QueryResult;
class TextQuery
{
public:
    using line_no = std::vector<std::string>::size_type;
    TextQuery(std::ifstream &in);
    TextQuery() = default;
    QueryResult query(const std::string &word) const;

private:
    std::shared_ptr<std::vector<std::string>> file;               // 传入文件
    std::map<std::string, std::shared_ptr<std::set<line_no>>> sn; // 单词和行号之间的map
};
class QueryResult
{
    using line_no = TextQuery::line_no;
    friend class TextQuery;
    friend class NotQuery;
    friend class AddQuery;
    friend class OrQuery;
    friend void print(std::ostream &os, const QueryResult &qr);

public:
    QueryResult() = default;
    QueryResult(const std::string &word, std::shared_ptr<std::set<line_no>> lines, std::shared_ptr<std::vector<std::string>> f)
        : sought(word), line_nos(lines), file(f) {}

private:
    std::string sought;                             // 要查询的单词
    std::shared_ptr<std::set<line_no>> line_nos;    // 单词出现的行号
    std::shared_ptr<std::vector<std::string>> file; // 单词所在文件
};

class Query_base
{
    friend class Query;

protected:
    using line_no = TextQuery::line_no;
    virtual ~Query_base() = default; // 默认析构函数

private:
    /// @brief 传入TextQuery输出查询结果
    /// @return QueryResult
    virtual QueryResult eval(const TextQuery &) const = 0;
    virtual std::string rep() const = 0; // 输出查询结果(string)
};

class Query
{
    friend class Query_base;
    friend Query operator&(const Query &, const Query &);
    friend Query operator|(const Query &, const Query &);
    friend Query operator~(const Query &);

public:
    Query(const std::string &s);
    QueryResult eval(const TextQuery &tq) const { return q->eval(tq); }
    std::string rep() const
    {
        return q->rep();
    }

private:
    Query(std::shared_ptr<Query_base> query) : q(query) {}
    std::shared_ptr<Query_base> q;
};

class WordQuery : public Query_base
{
    friend class Query;
    WordQuery(const std::string &s) : word(s) {}
    QueryResult eval(const TextQuery &tq) const { return tq.query(word); }
    std::string rep() const { return word; };
    std::string word; // 要查询的单词
};

inline Query::Query(const std::string &s) : q(new WordQuery(s)) {}

class NotQuery : public Query_base
{
    using line_no = TextQuery::line_no;
    friend class Query;
    friend Query operator~(const Query &);
    NotQuery(const Query &q) : query(q) {}
    QueryResult eval(const TextQuery &tq) const;
    std::string rep() const { return "~(" + query.rep() + ")"; }
    Query query;
};

inline Query operator~(const Query &query)
{
    return std::shared_ptr<Query_base>(new NotQuery(query));
}

class BinaryQuery : public Query_base
{
protected:
    std::string rep() const override { return "(" + lhs.rep() + opSym + rhs.rep() + ")"; }
    BinaryQuery(const Query &left, const Query &right, std::string sig) : lhs(left), rhs(right), opSym(sig) {}
    Query lhs, rhs;    // yuansuan
    std::string opSym; // 运算符
};

class AddQuery : public BinaryQuery
{
    friend Query operator&(const Query &, const Query &);
    AddQuery(const Query &left, const Query &right) : BinaryQuery(left, right, "&") {}
    QueryResult eval(const TextQuery &) const override;
};

inline Query operator&(const Query &lhs, const Query &rhs)
{
    return std::shared_ptr<Query_base>(new AddQuery(lhs, rhs));
}

class OrQuery : public BinaryQuery
{
    friend Query operator|(const Query &, const Query &);
    OrQuery(const Query &left, const Query &right) : BinaryQuery(left, right, "|") {}
    QueryResult eval(const TextQuery &) const override;
};

inline Query operator|(const Query &left, const Query &right)
{
    return std::shared_ptr<Query_base>(new OrQuery(left, right));
}
