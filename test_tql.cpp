#include <stdio.h>
#include <stdlib.h>
#include "tql_inc.h"

using namespace tql;
using namespace std;

int main(int argc, char** argv)
{
    parser_context_t pctx;
    if (argc < 2)
    {
        printf("usage: %s sql\n", argv[0]);
        return 0;
    }

    const char* sql = argv[1];
    int ret = 0;
    yyscan_t scanner;
    ret = yylex_init(&scanner);
    if (ret)
    {
        printf("yylex_init failed ret:%d, errno:%d\n", ret, errno);
        return 0;
    }

    yy_scan_string(sql, scanner);
    void* parser = ParseAlloc(malloc);
    token_t token;
    vector<token_t> tokens;
    // 先扫描所有的token
    while (true)
    {
        ret = yylex(scanner);
        printf("yylex ret:%d, token:%s\n", ret, yyget_text(scanner));
        token.tid_ = ret;
        token.str_ = yyget_text(scanner);
        tokens.push_back(token);

        if (ret < 0)
        {
            printf("yylex error %d!\n", ret);
            return -1;
        }
        else if (ret == 0)
        {
            printf("yylex reach eof\n");
            break;
        }
    }

    for (int i = 0; i < (int)tokens.size(); ++i)
    {
        Parse(parser, tokens.at(i).tid_, &tokens.at(i), &pctx);
        if (pctx.get_errno())
        {
            printf("parse error:%d\n", pctx.get_errno());
            return -1;
        }
    }

    if (pctx.get_errno())
    {
        printf("parse error:%d, near %s\n", pctx.get_errno(), pctx.get_error_nearby().c_str());
        return -1;
    }

    // 完事，释放资源
    ParseFree(parser, free);
    yylex_destroy(scanner);

    // 打印语法树
    printf("stmt type:%d\n", pctx.get_stmt_type());
    printf("table:%s\n", pctx.get_table().c_str());

    for (int i = 0; NULL != pctx.get_field(i); ++i)
    {
        const std::string* fld = pctx.get_field(i);
        printf("fld[%d]=%s\n", i, fld->c_str());
    }

    for (int i = 0; NULL != pctx.get_condition(i); ++i)
    {
        const expr2_t* cond = pctx.get_condition(i);
        printf("cond[%d], type=%d, op=%d, left=%d, right=%d\n", i, cond->type_, cond->op_, cond->left_, cond->right_);
    }

    for (int i = 0; NULL != pctx.get_assignment(i); ++i)
    {
        const assign_t* ass = pctx.get_assignment(i);
        printf("ass[%d], op=%d, left=%s, right=%d\n", i, ass->op_, ass->left_.c_str(), ass->right_);
    }

    for (int i = 0; NULL != pctx.get_math(i); ++i)
    {
        expr2_t* math = (expr2_t*)pctx.get_math(i);
        printf("math[%d], type=%d, op=%d, left=%d, right=%d, val=%s, len=%d\n", i, math->type_, math->op_, math->left_, math->right_, math->var_.to_string().c_str(), math->var_.to_string().length());
    }

    for (int i = 0; NULL != pctx.get_key(i); ++i)
    {
        const string* key = pctx.get_key(i);
        printf("key[%d]=%s, len=%d\n", i, key->c_str(), key->length());
    }

    return 0;
}
