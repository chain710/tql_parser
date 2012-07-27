#include "tql_parser.h"
#include "SqlParser.h"
#include "SqlLex.h"
#include <stdio.h>
#include <stdlib.h>

using namespace tql;
using namespace std;

// dst_type:目标类型(不支持字符串), val:目标取值
template<typename T>
int conv_var_by_type(tql::variant_t& var, int dst_type, const T& val)
{
    switch (dst_type)
    {
    case variant_t::evt_double: var.numeral_.double_val_ = (double)val; break;
    case variant_t::evt_float: var.numeral_.float_val_ = (float)val; break;
    case variant_t::evt_int32: var.numeral_.int32_val_ = (int)val; break;
    case variant_t::evt_uint32: var.numeral_.uint32_val_ = (unsigned int)val; break;
    case variant_t::evt_int64: var.numeral_.int64_val_ = (int64_t)val; break;
    case variant_t::evt_uint64: var.numeral_.uint64_val_ = (uint64_t)val; break;
    default: return -1;
    }

    var.type_ = dst_type;
    return 0;
}

template<>
int conv_var_by_type<string>(tql::variant_t& var, int type, const string& val)
{
    switch (type)
    {
    case variant_t::evt_double: var.numeral_.double_val_ = strtod(val.c_str(), NULL); break;
    case variant_t::evt_float: var.numeral_.float_val_ = strtof(val.c_str(), NULL); break;
    case variant_t::evt_int32: var.numeral_.int32_val_ = strtol(val.c_str(), NULL, 10); break;
    case variant_t::evt_uint32: var.numeral_.uint32_val_ = strtoul(val.c_str(), NULL, 10); break;
    case variant_t::evt_int64: var.numeral_.int64_val_ = strtoll(val.c_str(), NULL, 10); break;
    case variant_t::evt_uint64: var.numeral_.uint64_val_ = strtoull(val.c_str(), NULL, 10); break;
    default: return -1;
    }

    var.type_ = type;
    return 0;
}

const std::string& tql::variant_t::to_string()
{
    char tmp[128];
    switch (type_)
    {
    case evt_double: snprintf(tmp, sizeof(tmp), "%f", numeral_.double_val_); str_ = tmp; break;
    case evt_float: snprintf(tmp, sizeof(tmp), "%f", numeral_.float_val_); str_ = tmp; break;
    case evt_int64: snprintf(tmp, sizeof(tmp), "%"PRId64, numeral_.int64_val_); str_ = tmp; break;
    case evt_uint64: snprintf(tmp, sizeof(tmp), "%"PRIu64, numeral_.uint64_val_); str_ = tmp; break;
    case evt_int32: snprintf(tmp, sizeof(tmp), "%d", numeral_.int32_val_); str_ = tmp; break;
    case evt_uint32: snprintf(tmp, sizeof(tmp), "%u", numeral_.uint32_val_); str_ = tmp; break;
    default: break;
    }

    return str_;
}

int tql::variant_t::cast_type( int type )
{
    if (type == type_)
    {
        return 0;
    }

    if (type == variant_t::evt_string)
    {
        str_ = to_string();
        type_ = variant_t::evt_string;
        return 0;
    }

    switch (type_)
    {
    case variant_t::evt_double: return conv_var_by_type(*this, type, numeral_.double_val_);
    case variant_t::evt_float: return conv_var_by_type(*this, type, numeral_.float_val_);
    case variant_t::evt_int32: return conv_var_by_type(*this, type, numeral_.int32_val_);
    case variant_t::evt_uint32: return conv_var_by_type(*this, type, numeral_.uint32_val_);
    case variant_t::evt_int64: return conv_var_by_type(*this, type, numeral_.int64_val_);
    case variant_t::evt_uint64: return conv_var_by_type(*this, type, numeral_.uint64_val_);
    case variant_t::evt_string: return conv_var_by_type(*this, type, str_);
    default: /*不支持的类型转换*/ return -1;
    }

    return 0;
}

tql::parser_context_t::parser_context_t()
{
    errno_ = epe_no_error;
    stmt_type_ = -1;
}

void tql::parser_context_t::append_field( const std::string& fld )
{
    fields_.push_back(fld);
}

int tql::parser_context_t::append_condition( const expr2_t& condition )
{
    int ret = conditions_.size();
    conditions_.push_back(condition);
    return ret;
}

int tql::parser_context_t::append_assignment( const assign_t& assign )
{
    int ret = assigns_.size();
    assigns_.push_back(assign);
    return ret;
}

void tql::parser_context_t::clear()
{
    errno_ = epe_no_error;
    stmt_type_ = -1;
    key_.clear();
    table_.clear();
    conditions_.clear();
    fields_.clear();
    assigns_.clear();
    var_pool_.clear();
    math_.clear();
    error_near_.clear();
}

int tql::parser_context_t::append_variant( const variant_t& var )
{
    int ret = var_pool_.size();
    var_pool_.push_back(var);
    return ret;
}

const variant_t* tql::parser_context_t::get_variant( int idx ) const
{
    if (idx < 0 || idx >= (int)var_pool_.size())
    {
        return NULL;
    }

    return &var_pool_.at(idx);
}

const std::string* tql::parser_context_t::get_field( int idx ) const
{
    if (idx < 0 || idx >= (int)fields_.size())
    {
        return NULL;
    }

    return &fields_.at(idx);
}

const expr2_t* tql::parser_context_t::get_condition( int idx ) const
{
    if (idx < 0 || idx >= (int)conditions_.size())
    {
        return NULL;
    }

    return &conditions_.at(idx);
}

const assign_t* tql::parser_context_t::get_assignment( int idx ) const
{
    if (idx < 0 || idx >= (int)assigns_.size())
    {
        return NULL;
    }

    return &assigns_.at(idx);
}

int tql::parser_context_t::append_math( const expr2_t& math )
{
    int ret = math_.size();
    math_.push_back(math);
    return ret;
}

const expr2_t* tql::parser_context_t::get_math( int idx ) const
{
    if (idx < 0 || idx >= (int)math_.size())
    {
        return NULL;
    }

    return &math_.at(idx);
}

const expr2_t* tql::parser_context_t::get_last_condition() const
{
    if (conditions_.empty())
    {
        return NULL;
    }

    return &conditions_.at(conditions_.size() - 1);
}

int tql::parser_context_t::from_string( const std::string stmt )
{
    clear();
    int ret;
    int parser_ret = 0;
    yyscan_t lex_scanner;
    ret = yylex_init(&lex_scanner);
    if (ret)
    {
        set_errno(epe_yylex_init_error);
        return -1;
    }

    yy_scan_string(stmt.c_str(), lex_scanner);
    vector<token_t> tokens;
    tokens.reserve(128);
    token_t tmp_token;
    while (true)
    {
        ret = yylex(lex_scanner);
        tmp_token.tid_ = ret;
        tmp_token.str_ = yyget_text(lex_scanner);
        tokens.push_back(tmp_token);
        if (ret < 0)
        {
            set_errno(epe_yylex_error);
            parser_ret = -1;
            break;
        }
        else if (0 == ret)
        {
            break;
        }
    }

    yylex_destroy(lex_scanner);
    if (parser_ret) return parser_ret;

    // 分析语法
    void* grammar_parser = ParseAlloc(malloc);
    if (NULL == grammar_parser)
    {
        set_errno(epe_init_grammar_error);
        return -1;
    }

    do 
    {
        int token_size = tokens.size();
        int i = 0;
        for (; i < token_size; ++i)
        {
            Parse(grammar_parser, tokens[i].tid_, &tokens[i], this);
            if (get_errno())
            {
                error_near_ = tokens[i].str_;
                parser_ret = -1;
                break;
            }
        }

        if (get_errno())
        {
            // error near
            error_near_ = tokens[i].str_;
            parser_ret = -1;
            break;
        }
    } while (false);

    ParseFree(grammar_parser, free);
    return parser_ret;
}

int tql::var_to_expr2( parser_context_t &ctx, int var )
{
    const variant_t* v = ctx.get_variant(var);
    if (NULL == v)
    {
        ctx.set_errno(tql::parser_context_t::epe_invalid_variant);
        return -1;
    }

    expr2_t add;
    add.type_ = tql::expr2_t::eet_math;
    add.op_ = -1;
    add.var_ = *v;
    return ctx.append_math(add);
}
