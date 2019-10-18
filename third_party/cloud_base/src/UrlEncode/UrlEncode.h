
// There are two standards for implementing urlencode.
//
// The first one in namespace rfc3986 reference to the standard of :
// T.Berners-Lee,RFC 3986:Uniform Resource Identifier (URI): Generic Syntax,
// W3C/MIT, January 2005, https://tools.ietf.org/html/rfc3986
// and wikipedia:https://en.wikipedia.org/wiki/Percent-encoding
//
// The rfc3986 method's rule is:
// Characters that are allowed in a URI but do not have a reserved
// purpose are called unreserved.These include uppercase and lowercase
// letters, decimal digits, hyphen, period, underscore, and tilde.
// unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
//
// The second one in nemespace http_post_form reference to the standard of MDN
// application/x-www-form-urlencoded web docs. which reference to:
// https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Methods/POST
// https://developer.mozilla.org/en-US/docs/Glossary/percent-encoding
//
// The http_post_form method's rule is:
// the keys and values are encoded in key-value tuples separated by '&', with a
// '=' between the key and the value. Non-alphanumeric characters in both keys
// and values are percent encoded: this is the reason why this type is not
// suitable to use with binary data (use multipart/form-data instead).
// unreserved = ALPHA / DIGIT
// The character ' ' is translated to a '+'.
//
// ALL urlencode results are normalized to use uppercase letters for the digits
// A-F.
//
#pragma once
#ifndef URLENCODE_H_
#define URLENCODE_H_

#include <string>

namespace cloud_base {
namespace url_encode {

namespace rfc3986 {
// url encode,ensure that input parameter is utf8 format
std::string url_encode(const std::string& str);
// url decode,ensure that input parameter is utf8 format
// This method is a general method
std::string url_decode(const std::string& str);

}  // namespace rfc3986

namespace http_post_form {
// url encode,ensure that input parameter is utf8 format
std::string url_encode(const std::string& str);
// url decode,ensure that input parameter is utf8 format
// This method is a general method
std::string url_decode(const std::string& str);

}  // namespace http_post_form

}  // namespace url_encode
}  // namespace cloud_base
#endif  // URLENCODE_H_
