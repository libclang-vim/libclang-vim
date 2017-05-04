#if !defined LIBCLANG_VIM_TOKENIZER_HPP_INCLUDED
#define LIBCLANG_VIM_TOKENIZER_HPP_INCLUDED

#include <string>
#include <vector>

#include <clang-c/Index.h>

#include "helpers.hpp"

namespace libclang_vim {

class tokenizer {
    CXSourceRange
    get_range_whole_file(const location_tuple& tuple,
                         const cxtranslation_unit_ptr& translation_unit) const;
    const char* get_kind_spelling(CXTokenKind kind) const;
    std::string
    make_vimson_from_tokens(const cxtranslation_unit_ptr& translation_unit,
                            std::vector<CXToken> tokens) const;

  public:
    std::string tokenize_as_vimson(const location_tuple& tuple);
};

} // namespace libclang_vim

#endif // LIBCLANG_VIM_TOKENIZER_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
