#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include "TextQuery.h"
using namespace std;
TextQuery::TextQuery(ifstream &in)
{
    string line; // 保存一阵行的数据
    file.reset(new vector<string>());
    while (getline(in, line))
    {
        file->push_back(line);
        auto n = file->size();
        string word; // 保存一个单词的数据
        stringstream line_to_word(line);
        while (line_to_word >> word)
        {
            auto &lines = sn[word]; // lines为shared_ptr
            if (!lines)
            { // 如果lines为空，先初始化lines
                lines.reset(new set<line_no>());
            }
            lines->insert(file->size());
        }
    }
}
QueryResult TextQuery::query(const string &word) const
{
    auto it = sn.find(word); // 用find防止word不存在sn时被插入sn
    if (it != sn.end())
    {
        return QueryResult(word, it->second, this->file);
    }
    else
    {
        return QueryResult(word, nullptr, nullptr);
    }
}
void print(ostream &os, const QueryResult &qr)
{
    if (qr.line_nos)
    {
        os << qr.sought << " occurs " << (qr.line_nos)->size() << " times." << endl;
        for (auto iter = qr.line_nos->begin(); iter != (qr.line_nos->end()); iter++)
        {
            os << "(" << *iter << " line) " << (*qr.file)[*iter - 1] << endl;
        }
    }
}
QueryResult NotQuery::eval(const TextQuery &tq) const
{
    auto result = query.eval(tq);
    auto lines = make_shared<set<line_no>>();

    auto &file = *result.file;
    auto &line_no = *result.line_nos;

    auto newLineNo = shared_ptr<set<TextQuery::line_no>>(new set<TextQuery::line_no>);
    for (size_t n = 0; n != file.size(); ++n)
    {
        if (line_no.find(n) == line_no.end())
        {
            newLineNo->insert(n);
        }
    }
    return QueryResult(result.sought, newLineNo, result.file);
}
QueryResult AddQuery::eval(const TextQuery &tq) const
{
    // &左右分别的查询结果
    auto lres = lhs.eval(tq);
    auto rres = rhs.eval(tq);

    auto lline_no = *lres.line_nos;
    auto rline_no = *rres.line_nos;

    auto newLineNo = shared_ptr<set<TextQuery::line_no>>(new set<TextQuery::line_no>);
    // 找到左右单词同时存在的行
    for (auto lit = lline_no.begin(); lit != lline_no.end(); lit++)
    {
        for (auto rit = rline_no.begin(); rit != rline_no.end(); rit++)
        {
            if (*lit == *rit)
            {
                newLineNo->insert(*lit);
            }
        }
    }
    return QueryResult(rep(), newLineNo, lres.file);
}
QueryResult OrQuery::eval(const TextQuery &tq) const
{
    // |左右分别的查询结果
    auto lres = lhs.eval(tq);
    auto rres = rhs.eval(tq);

    auto lline_no = *lres.line_nos;
    auto rline_no = *rres.line_nos;

    auto newLineNo = shared_ptr<set<TextQuery::line_no>>(new set<TextQuery::line_no>(lline_no));
    for (auto rit = rline_no.begin(); rit != rline_no.end(); ++rit)
    {
        newLineNo->insert(*rit);
    }
    return QueryResult(rep(), newLineNo, lres.file);
}