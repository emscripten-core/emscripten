/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <uuid/uuid.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

int isUUID(char* p, int upper) {
    char* p1 = p;
    do {
        if (!(isxdigit(*p1) || (*p1 == '-')) || (upper && islower(*p1)) || (!upper && isupper(*p1))) {
            return 0;
        } else {
        }
    } while (*++p1 != 0);

    if ((p[8] == '-') && (p[13] == '-') && (p[18] == '-') && (p[23] == '-')) {
        return 1;
    } else {
        return 0;
    }
}

int main() {
    uuid_t uuid;
    uuid_t uuid1;
    uuid_t uuid2;
    uuid_t empty_uuid = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uuid_generate(uuid);

    assert(uuid_is_null(uuid) == 0);
    assert(uuid_type(uuid) == UUID_TYPE_DCE_RANDOM);
    assert(uuid_variant(uuid) == UUID_VARIANT_DCE);
    assert((uuid[8] & 0xC0) == 0x80); // RFC-4122 variant marker

    char generated[37];
    uuid_unparse(uuid, generated);
    assert(isUUID(generated, 0) == 1); // Check it's a valid lower case UUID string.
    printf("\nuuid = %s\n", generated);

    assert(uuid_parse(generated, uuid1) == 0); // Check the generated UUID parses correctly into a compact UUID.
    assert(uuid_compare(uuid1, uuid) == 0);    // Compare the parsed UUID with the original.

    uuid_unparse_lower(uuid, generated);
    assert(isUUID(generated, 0) == 1); // Check it's a valid lower case UUID string.
    printf("uuid = %s\n", generated);

    uuid_unparse_upper(uuid, generated);
    assert(isUUID(generated, 1) == 1); // Check it's a valid upper case UUID string.
    printf("uuid = %s\n", generated);

    uuid_copy(uuid2, uuid);
    assert(uuid_compare(uuid2, uuid) == 0);

    uuid_clear(uuid);
    assert(uuid_compare(empty_uuid, uuid) == 0);

    assert(uuid_is_null(uuid) == 1);
    return 0;
}

