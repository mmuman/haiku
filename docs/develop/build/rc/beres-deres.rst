Differences with beres/deres
############################

Some of your old rdef files may fail to compile with rc. Besides different command line arguments,
rc also has slightly different syntax and semantics. These changes were made to allow for the future
expansion of the rdef grammar and to make resource scripts easier to write and understand. The
compiler should be backwards compatible with the majority of old scripts, but in some cases it was
necessary to break stuff.

Differences with beres
======================

We allow the \0 escape in string literals.

Our handling of the minus sign is more restrictive; we don't treat it like a unary minus operator,
which means you may only put one minus in front of a number to negate it (i.e. "- -10" is an error).

beres allows #define statements, but apparently ignores them. The compiler reports no error, but
any symbols you define are not recognized later on. We don't allow #defines (yet).

beres allows you to put enum symbols in resource data, and replaces the symbol with its value. For
some reason, it lets you assign any kind of data to enum symbols, such as strings and floats. Since
that is not valid C/C++ syntax, librdef does not allow this. As a result, it does not make much
sense for us to support enum symbols in resource data.

We only allow a few type casts, but enough to be backwards compatible. beres allows you to cast
from bool to all other types, between strings and numeric types, etc. The usefulness of this is
limited, so to keep things simple librdef doesn't allow that.

There is no need to put parentheses around a type code, even for simple data; beres sometimes
requires the parens and sometimes it doesn't. With rc, they are always optional (preferred notation
is without the parens).

beres allows multiple type codes and/or type casts per resource statement/data field, but we allow
only one of each. Because we don't associate type codes with data (but only with the resource
itself or with individual message fields), we don't allow the shortcut notation that lets you put a
type code and cast inside one set of parentheses.

We do not allow arrays to have named fields, nor do we allow messages (and archives) to have
unnamed fields. beres apparently treats all compound data the same, but we don't, because not all
compound data is the same.

When specifying array data, the "array" keyword is required. beres allows you to omit the array
keyword and just put the data between braces, but that conflicts with our handling of user-defined
types.

Field names in message resources may only be string literals. beres allows identifiers but it
converts them to string literals anyway. Just like normal data, you may cast the field's data to a
different type, but not to a different type code. Specifying a non-standard type code is fine, but
it goes in front of the type name. Just like beres, rc considers "array", "message", and "archive"
to be valid data type names, but "message" and "archive" cannot be followed by a "what" code
(beres ignores that anyway). When you cast an archive to a message, we don't strip the "class"
field (in fact, we don't perform any conversion).

We allow users to define their own types. The built-in types from beres (point, rect, and rgb_color)
are re-implemented as such user-defined types. The syntax for these three built-ins should still be
backwards compatible with old scripts.

beres automatically adds names for resources that are specified as "resource(R_Symbol)", unless the
"explicit names only" option is set. We do that the other way around: we don't automatically add
names unless the user sets the "auto names" option.

Differences with deres
======================

We do not write the "generated at" time into the files.

Type codes that cannot be represented as #'xxxx' are written as #num where num is a decimal number, not hexadecimal.

Floats and doubles have a bunch of extra zeroes.

Hex numbers are always uppercase.

We may render string literals somewhat differently.

We only write a header file if the "auto names" option is set.

