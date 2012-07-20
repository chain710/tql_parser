#include "tql_parser.h"

using namespace tql;
using namespace std;

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
