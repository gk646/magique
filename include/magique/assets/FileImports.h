// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CSVREADER_H
#define MAGIQUE_CSVREADER_H

#include "magique/assets/types/TextLines.h"


#include <magique/assets/types/Asset.h>
#include <magique/assets/container/CSVImport.h>
#include <glaze/json/write.hpp>
#include <magique/util/Logging.h>
#include <magique/gamedev/VirtualClock.h>

//===============================================
// File Imports
//===============================================
// ................................................................................
// This module allows to import various files types to a representation that can easily be accessed.
//
// Refer to https://stephenberry.github.io/glaze/json/ for more info on how to use the JSON library
// IMPORTANT: If you define custom parse/serialization rules etc. they need to be included BEFORE you call Import/Export

// Notes: This also contains the serialization specializations for magique structs
// ................................................................................

namespace magique
{
    // Imports the given asset as CSV file - if specified the first row will be loaded as column names
    // Supported filetypes: ".csv"
    // Failure: Returns an empty import
    // IMPORTANT: Only supports \n or \r as line feeds (not the windows \r\n)
    CSVImport ImportCSV(Asset asset, char delimiter = ';', bool firstRowNames = true);

    // Deserializes the given json into the given c++ type
    // Refer to https://stephenberry.github.io/glaze/json/
    template <typename T>
    bool ImportJSON(Asset asset, T& data);
    template <typename T>
    bool ImportJSON(const char* json, T& data);

    // Serialized the given data into the buffer (will be cleared and sized appropriately)
    template <typename T, bool prettify = false>
    bool ExportJSON(const T& data, std::string& buffer);

    // Imports the given asset into a simple container
    // The file is separated and stored line-wise
    // Failure: Returns empty container
    TextLines ImportText(Asset asset, char delimiter = '\n');

} // namespace magique


template <>
struct glz::meta<magique::Point>
{
    using T = magique::Point;
    static constexpr auto value = object(&T::x, &T::y);
};

template <>
struct glz::meta<magique::Keybind>
{
    using T = magique::Keybind;
    static constexpr auto value = object(&T::key, &T::layered, &T::shift, &T::ctrl, &T::alt);
};

template <>
struct glz::meta<magique::VirtualClock>
{
    using T = magique::VirtualClock;
    static constexpr auto value = object(&T::realSecondSeconds, &T::ticks, &T::timeScale, &T::isPaused);
};


// IMPLEMENTATION

namespace magique
{
    inline const char* GetJSONErrStr(const glz::error_ctx& ec)
    {
        switch (ec.ec)
        {
        case glz::error_code::none:
            return "none";
        case glz::error_code::version_mismatch:
            return "version_mismatch";
        case glz::error_code::invalid_header:
            return "invalid_header";
        case glz::error_code::invalid_query:
            return "invalid_query";
        case glz::error_code::invalid_body:
            return "invalid_body";
        case glz::error_code::parse_error:
            return "parse_error";
        case glz::error_code::method_not_found:
            return "method_not_found";
        case glz::error_code::timeout:
            return "timeout";
        case glz::error_code::send_error:
            return "send_error";
        case glz::error_code::connection_failure:
            return "connection_failure";
        case glz::error_code::end_reached:
            return "end_reached";
        case glz::error_code::partial_read_complete:
            return "partial_read_complete";
        case glz::error_code::no_read_input:
            return "no_read_input";
        case glz::error_code::data_must_be_null_terminated:
            return "data_must_be_null_terminated";
        case glz::error_code::parse_number_failure:
            return "parse_number_failure";
        case glz::error_code::expected_brace:
            return "expected_brace";
        case glz::error_code::expected_bracket:
            return "expected_bracket";
        case glz::error_code::expected_quote:
            return "expected_quote";
        case glz::error_code::expected_comma:
            return "expected_comma";
        case glz::error_code::expected_colon:
            return "expected_colon";
        case glz::error_code::exceeded_static_array_size:
            return "exceeded_static_array_size";
        case glz::error_code::exceeded_max_recursive_depth:
            return "exceeded_max_recursive_depth";
        case glz::error_code::unexpected_end:
            return "unexpected_end";
        case glz::error_code::expected_end_comment:
            return "expected_end_comment";
        case glz::error_code::syntax_error:
            return "syntax_error";
        case glz::error_code::unexpected_enum:
            return "unexpected_enum";
        case glz::error_code::attempt_const_read:
            return "attempt_const_read";
        case glz::error_code::attempt_member_func_read:
            return "attempt_member_func_read";
        case glz::error_code::attempt_read_hidden:
            return "attempt_read_hidden";
        case glz::error_code::invalid_nullable_read:
            return "invalid_nullable_read";
        case glz::error_code::invalid_variant_object:
            return "invalid_variant_object";
        case glz::error_code::invalid_variant_array:
            return "invalid_variant_array";
        case glz::error_code::invalid_variant_string:
            return "invalid_variant_string";
        case glz::error_code::no_matching_variant_type:
            return "no_matching_variant_type";
        case glz::error_code::expected_true_or_false:
            return "expected_true_or_false";
        case glz::error_code::constraint_violated:
            return "constraint_violated";
        case glz::error_code::key_not_found:
            return "key_not_found";
        case glz::error_code::unknown_key:
            return "unknown_key";
        case glz::error_code::missing_key:
            return "missing_key";
        case glz::error_code::invalid_flag_input:
            return "invalid_flag_input";
        case glz::error_code::invalid_escape:
            return "invalid_escape";
        case glz::error_code::u_requires_hex_digits:
            return "u_requires_hex_digits";
        case glz::error_code::unicode_escape_conversion_failure:
            return "unicode_escape_conversion_failure";
        case glz::error_code::dump_int_error:
            return "dump_int_error";
        case glz::error_code::file_open_failure:
            return "file_open_failure";
        case glz::error_code::file_close_failure:
            return "file_close_failure";
        case glz::error_code::file_include_error:
            return "file_include_error";
        case glz::error_code::file_extension_not_supported:
            return "file_extension_not_supported";
        case glz::error_code::could_not_determine_extension:
            return "could_not_determine_extension";
        case glz::error_code::get_nonexistent_json_ptr:
            return "get_nonexistent_json_ptr";
        case glz::error_code::get_wrong_type:
            return "get_wrong_type";
        case glz::error_code::seek_failure:
            return "seek_failure";
        case glz::error_code::cannot_be_referenced:
            return "cannot_be_referenced";
        case glz::error_code::invalid_get:
            return "invalid_get";
        case glz::error_code::invalid_get_fn:
            return "invalid_get_fn";
        case glz::error_code::invalid_call:
            return "invalid_call";
        case glz::error_code::invalid_partial_key:
            return "invalid_partial_key";
        case glz::error_code::name_mismatch:
            return "name_mismatch";
        case glz::error_code::array_element_not_found:
            return "array_element_not_found";
        case glz::error_code::elements_not_convertible_to_design:
            return "elements_not_convertible_to_design";
        case glz::error_code::unknown_distribution:
            return "unknown_distribution";
        case glz::error_code::invalid_distribution_elements:
            return "invalid_distribution_elements";
        case glz::error_code::hostname_failure:
            return "hostname_failure";
        case glz::error_code::includer_error:
            return "includer_error";
        case glz::error_code::feature_not_supported:
            return "feature_not_supported";
        default:
            return "unknown_glz::error_code";
        }
    }

    template <typename T>
    bool ImportJSON(Asset asset, T& data)
    {
        std::string_view buff{asset.getData(), static_cast<size_t>(asset.getSize())};
        auto ec = glz::read_json(data, buff);
        if (ec)
        {
            LOG_ERROR("Failed to import JSON %s: %s at position %d\n%.30s", asset.getFileName(), GetJSONErrStr(ec),
                      (int)ec.location, asset.getData() + ec.location);
            return false;
        }
        return true;
    }

    template <typename T>
    bool ImportJSON(const char* json, T& data)
    {
        std::string_view buff{json, strlen(json)};
        auto ec = glz::read_json(data, buff);
        if (ec)
        {
            LOG_ERROR("Failed to import JSON %.15s: %s at position %d\n%.30s", json, GetJSONErrStr(ec), (int)ec.location,
                      json + ec.location);
            return false;
        }
        return true;
    }

    template <typename T, bool prettify>
    bool ExportJSON(const T& data, std::string& buffer)
    {
        buffer.clear();
        auto ec = glz::write<glz::opts{.prettify = prettify}>(data, buffer);
        if (ec)
        {
            LOG_ERROR("Failed to export JSON: %s ", GetJSONErrStr(ec));
            return false;
        }
        return true;
    }


} // namespace magique


#endif //MAGIQUE_CSVREADER_H
