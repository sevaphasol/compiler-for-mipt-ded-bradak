#include "buffer.h"
#include "custom_assert.h"

#include <stdlib.h>
#include <string.h>

//——————————————————————————————————————————————————————————————————————————————

lang_status_t buf_ctor(buffer_t* buf, size_t init_capacity)
{
    ASSERT(buf);

    buf->data = (uint8_t*) calloc(init_capacity, sizeof(uint8_t));
    VERIFY(!buf->data, return LANG_ERROR);

    buf->capacity = init_capacity;
    buf->size = 0;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t buf_dtor(buffer_t* buf)
{
    ASSERT(buf);

    VERIFY(!buf->data, return LANG_ERROR);

    free(buf->data);

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t buf_write(buffer_t* buf, const void* data, size_t data_size)
{
    ASSERT(buf);
    ASSERT(data);

    size_t needed = buf->size + data_size;
    if (needed > buf->capacity) {
        size_t new_cap = buf->capacity * 2;
        if (new_cap < needed) new_cap = needed;
        uint8_t* new_data = (uint8_t*) realloc(buf->data, new_cap);
        if (!new_data) return LANG_ERROR;
        buf->data = new_data;
        buf->capacity = new_cap;
    }

    memcpy(buf->data + buf->size, data, data_size);
    buf->size += data_size;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————

lang_status_t buf_rewrite(buffer_t*   buf,
                          const void* data,
                          size_t      data_size,
                          size_t      offset)
{
    ASSERT(buf);
    ASSERT(data);

    size_t old_buf_size = buf->size;
    buf->size = offset;
    buf_write(buf, data, data_size);
    buf->size = old_buf_size;

    return LANG_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————
